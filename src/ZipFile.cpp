#include"ZipFile.h"

#include"ZipUtil.h"
#include"ZipDeflate.h"

#include<stdexcept>
#include<iostream>
#include<fstream>
#include<utility>
#include<iterator>

int const SIZE_LIMIT = 128 * 1024 * 1024; // MB
extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

uint32_t ZIP_DIGITAL_SIGNATURE_HEADER = 0x05054b50;
std::vector<std::uint8_t> ExtractData_stored(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span);
std::vector<std::uint8_t> ExtractData_deflate(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span);

ZipFile::ZipFile(std::string filepath) {
    try {
        std::ifstream fn(filepath, std::ios::binary);
        fn.unsetf(std::ios::skipws);
        std::streampos fsize;
        fn.seekg(0, std::ios::end);
        fsize = fn.tellg();
        if (fsize > SIZE_LIMIT) {
            snprintf(_zip_errmsg, _zip_errmsg_sz, "ZipFile::Constructor: File size too big, maximum supported size is %d MB", SIZE_LIMIT / 1024 / 1024);
            throw std::length_error(_zip_errmsg);
        }
        fn.seekg(0, std::ios::beg);

        rawData = std::vector<uint8_t>();
        rawData.reserve(fsize);
        rawData.insert(rawData.begin(), 
            std::istream_iterator<uint8_t>(fn), 
            std::istream_iterator<uint8_t>());
    } catch (std::ifstream::failure &e) {
        throw std::invalid_argument("ZipFile::Constructor: Cannot read zip file");
    }

    localFiles = std::vector<ZipLocalFile>();
    centralDirectories = std::vector<ZipCentralDirectory>();
    digital_signature_data_start_offset = digital_signature_data_end_offset = -1;

    for (int curpos = 0; curpos < (int)rawData.size();) {
        uint32_t header = GetUint32(rawData, curpos);
        curpos -= 4; //unget
        if (header == ZIP_LOCAL_FILE_HEADER_SIGNATURE) {
            try {
                localFiles.emplace_back(ZipLocalFile(rawData, curpos));
            } catch (std::exception const &e) {
                throw;
            }
        }
        // else if (header == ZIP_ARCHIVE_EXTRA_DATA_SIGNATURE) {} // Zip 6.2, skip
        else if (header == ZIP_CENTRAL_FILE_HEADER_SIGNATURE) {
            try {
                centralDirectories.emplace_back(ZipCentralDirectory(rawData, curpos));
            } catch (std::exception const &e) {
                throw;
            }
        }
        else if (header == ZIP_DIGITAL_SIGNATURE_HEADER) {
            curpos += 4;
            if (!CheckSize(rawData, curpos, 2)) {
                throw std::length_error("ZipFile::DigitalSignature: Unexpected EOF");
            }
            uint16_t digital_signature_size = GetUint16(rawData, curpos);
            if (!CheckSize(rawData, curpos, digital_signature_size)) {
                throw std::length_error("ZipFile::DigitalSignature: Unexpected EOF");
            }
            digital_signature_data_start_offset = curpos;
            curpos += digital_signature_size;
            digital_signature_data_end_offset = curpos;
        }
        // else if (header == ZIP_64_END_OF_CENTRAL_DIRECTORY_RECORD_SIGNATURE) {} // Zip 6.2, skip
        // else if (header == ZIP_64_END_OF_CENTRAL_DIRECTORY_LOCATOR_SIGNATURE) {} // Zip 6.2, skip
        else if (header == ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_SIGNATURE) {
            try {
                endOfCentralDirectoryRecord = ZipEndOfCentralDirectoryRecord(rawData, curpos);
            } catch (std::exception const& e) {
                throw;
            }
        }
        else {
            throw std::invalid_argument("ZipFile::Constructor: Unrecognized header, might belong to a proprietary zip format");
        }
    }
}

std::vector<std::string> ZipFile::GetFileList() const {
    std::vector<std::string> fileList = std::vector<std::string>();
    for (ZipLocalFile const& zf: localFiles) {
        std::pair<int,int> start_and_end = zf.GetFileName();
        int length = start_and_end.second - start_and_end.first;
        std::string filename(length, 'a');
        for (int i = 0; i < length; ++i) {
            filename[i] = rawData[i+start_and_end.first];
        }
        fileList.emplace_back(filename);
    }
    return fileList;
}
std::vector<std::uint8_t> ZipFile::ExtractData(int file_index) const {
    if (file_index >= (int)localFiles.size()) return std::vector<std::uint8_t>(0);
    else if (file_index < 0) return std::vector<std::uint8_t>(0);
    else return ExtractData(localFiles[file_index]);
}
std::vector<std::uint8_t> ZipFile::ExtractData(ZipLocalFile const& zf) const{
    uint16_t compression_method = zf.GetCompressionMethod();
    try {
        if (compression_method == 0) {
            return ExtractData_stored(rawData, zf.GetData());
        }
        else if (compression_method == 8) {
            return ExtractData_deflate(rawData, zf.GetData());
        }
        else {
            fprintf(stderr, "ZipFile::ExtractData::Unsupported compression method, will return empty data\n");
            return std::vector<std::uint8_t>(0);
        }
    } catch (std::exception const& e) {
        throw;
    }
}

std::vector<std::uint8_t> ExtractData_stored(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span) {
    return std::vector<std::uint8_t>(rawData.begin() + data_span.first, rawData.begin() + data_span.second);
}
std::vector<std::uint8_t> ExtractData_deflate(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span) {
    ZipDeflate zdf(rawData, data_span.first, data_span.second);
    return zdf.Decode();
}
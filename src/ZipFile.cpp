#include"ZipFile.h"

#include"ZipUtil.h"
#include"ZipDeflate.h"

#include<stdexcept>
#include<iostream>
#include<fstream>
#include<utility>
#include<iterator>
#include<thread>
#include<mutex>
#include<atomic>
#include<tuple>
#include<algorithm>

// Đặt giới hạn là 128MB để tránh trường hợp thiếu RAM, cũng như không phải xử lý các trường hợp số lớn hay số gần số lớn, tràn số, ...
int const SIZE_LIMIT = 128 * 1024 * 1024; // MB
extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

uint32_t ZIP_DIGITAL_SIGNATURE_HEADER = 0x05054b50;
std::vector<std::uint8_t> ExtractData_stored(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span);
std::vector<std::uint8_t> ExtractData_deflate(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span);
std::vector<uint8_t> ExtractDataWithPassword_stored(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span, ZipPassword pwdKey);
std::vector<uint8_t> ExtractDataWithPassword_deflate(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span, ZipPassword pwdKey);

/*
Khởi tạo file Zip, đọc dữ liệu
Sắp xếp lại các file trong file Zip theo tên
*/
ZipFile::ZipFile(std::string filepath) {
    try {
        std::ifstream fn(filepath, std::ios::binary);
        if (fn.fail()) {
            snprintf(_zip_errmsg, _zip_errmsg_sz, "cannot open file %s", filepath.c_str());
            throw std::invalid_argument(_zip_errmsg);
        }
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

    std::vector<ZipLocalFile> _localFiles;
    for (int curpos = 0; curpos < (int)rawData.size();) {
        uint32_t header = GetUint32(rawData, curpos);
        curpos -= 4; //unget
        if (header == ZIP_LOCAL_FILE_HEADER_SIGNATURE) {
            try {
                _localFiles.emplace_back(ZipLocalFile(rawData, curpos));
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

    // Lọc các file có độ lớn là 0 (các folder)
    std::copy_if(_localFiles.begin(), _localFiles.end(), std::back_inserter(localFiles), 
        [&](const ZipLocalFile& zf) {
            return zf.GetUncompressedSize() > 0;
        });

    // Sắp xếp lại các file theo tên (chỉ sắp xếp theo giá trị ascii, tức tên unicode có thể sẽ không theo thứ tự alphabet, nhưng luôn đảm bảo tính stable)
    std::stable_sort(localFiles.begin(), localFiles.end(), [&](const ZipLocalFile& a, const ZipLocalFile& b) {
        int nas, nae, nbs, nbe;
        std::tie(nas, nae) = a.GetFileName();
        std::tie(nbs, nbe) = a.GetFileName();
        for (int i = 0; i < std::min(nae-nas, nbe-nbs); ++i) {
            if (rawData[nas+i] != rawData[nbs+i]) {
                return rawData[nas+i] < rawData[nbs+i];
            }
        }
        return nae-nas < nbe-nbs;
    });
}

/* Trả về danh sách các file có dữ liệu trong file Zip */
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

// Lấy dữ liệu của file
std::vector<std::uint8_t> ZipFile::ExtractData(int file_index) const {
    if (file_index >= (int)localFiles.size())  {
        throw std::invalid_argument("ZipFile::ExtractData: Invalid file index");
    }
    else if (file_index < 0)  {
        throw std::invalid_argument("ZipFile::ExtractData: Invalid file index");
    }
    else return ExtractData(localFiles[file_index]);
}
std::vector<std::uint8_t> ZipFile::ExtractData(ZipLocalFile const& zf) const{
    if (zf.IsEncrypted()) {
        throw std::invalid_argument("ZipFile::ExtractData: File is encrypted; consider using ExtractDataWithPassword");
    }

    uint16_t compression_method = zf.GetCompressionMethod();
    try {
        if (compression_method == 0) {
            return ExtractData_stored(rawData, zf.GetData());
        }
        else if (compression_method == 8) {
            return ExtractData_deflate(rawData, zf.GetData());
        }
        else {
            throw std::invalid_argument("ZipFile::ExtractData: Unsupported compression method");
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
    try {
        return zdf.Decode();
    } catch (std::exception& e) {
        throw;
    }
}

// Lấy dữ liệu của file có password, std::string chỉ dùng để chứa password, hoàn toàn có thể là một xâu nhị phân chứ không nhất thiết là một xâu đọc được
std::vector<uint8_t> ZipFile::ExtractDataWithPassword(int file_index, std::string const& pwd) const {
    if (file_index >= (int)localFiles.size()) {
        throw std::invalid_argument("ZipFile::ExtractDataWithPassword: Invalid file index");
    }
    else if (file_index < 0) {
        throw std::invalid_argument("ZipFile::ExtractDataWithPassword: Invalid file index");
    }
    else return ExtractDataWithPassword(localFiles[file_index], pwd);
}

std::vector<uint8_t> ZipFile::ExtractDataWithPassword(ZipLocalFile const& zf, std::string const& pwd) const {
    if (!zf.IsEncrypted()) {
        throw std::invalid_argument("ZipFile::ExtractDataWithPassword: File isn't encrypted; consider using ExtractData");
    }
    ZipPassword zpwd;
    for (char c: pwd) zpwd.UpdateKey(c);
    return ExtractDataWithPassword(zf, zpwd);
}

std::vector<uint8_t> ZipFile::ExtractDataWithPassword(ZipLocalFile const& zf, ZipPassword zpwd) const {
    std::pair<int,int> data_span = zf.GetData();
    if (data_span.second - data_span.first < 12) {
        throw std::invalid_argument("ZipFile::ExtractDataWithPassword: Invalid encrypted header");
    }
    uint8_t buf[12];
    for (int i = 0; i < 12; ++i) {
        uint8_t db = zpwd.DecryptByte();
        buf[i] = rawData[i + data_span.first] ^ db;
        zpwd.UpdateKey(buf[i]); 
    }
    
    uint16_t ver = zf.GetVersionNeeded();
    int major = ver / 10;
    uint32_t crc_got, crc_check = zf.GetCrc32(); 
    if (major < 2) { /* ZIP 1.x, use 2 byte */
        crc_got = (buf[11] << 8) | buf[10];
        crc_check >>= 16;
    }
    else { /* ZIP 2.0, use 1 byte */
        crc_got = buf[11];
        crc_check >>= 24;
    }

    if (crc_got != crc_check) {
        throw std::invalid_argument("ZipFile::ExtractDataWithPassword: Mismatch crc32 check, maybe wrong password");
    }
    data_span.first += 12;


    uint16_t compression_method = zf.GetCompressionMethod();
    try {
        uint32_t crc32 = zf.GetCrc32();
        std::vector<uint8_t> decode_data;
        if (compression_method == 0) {
            decode_data = ExtractDataWithPassword_stored(rawData, data_span, zpwd);
        }
        else if (compression_method == 8) {
            decode_data = ExtractDataWithPassword_deflate(rawData, data_span, zpwd);
        }
        else {
            throw std::invalid_argument("ZipFile::ExtractDataWithPassword: Unsupported compression method");
        }

        if (crc32_compute(decode_data) != crc32) {
            throw std::invalid_argument("ZipFile::ExtractDataWithPassword: Mismatch crc32 check, maybe wrong password");
        }
        return decode_data;
    } catch (std::exception const& e) {
        throw;
    }
}

std::vector<uint8_t> ExtractDataWithPassword_stored(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span, ZipPassword pwdKey) {
    std::vector<uint8_t> decode_data;
    for (int i = data_span.first; i < data_span.second; ++i) {
        uint8_t nb = pwdKey.DecryptByte() ^ rawData[i];
        pwdKey.UpdateKey(nb);
        decode_data.emplace_back(nb);
    }
    return decode_data;
}

std::vector<uint8_t> ExtractDataWithPassword_deflate(std::vector<uint8_t> const& rawData, std::pair<int,int> data_span, ZipPassword pwdKey) {
    ZipDeflate zdf(rawData, data_span.first, data_span.second, pwdKey);
    try {
        return zdf.Decode();
    } catch (std::exception& e) {
        throw;
    }
}

// Tấn công từ điển để dò ra mật khẩu
void ZipFile::BruteForceFile(int file_index, std::vector<std::vector<std::string>> const& Dict, int jobs) const {
    if (jobs < 1) {
        throw std::invalid_argument("Invalid number of threads");
    }
    if (jobs > (int)std::thread::hardware_concurrency()) {
        throw std::invalid_argument("Too many threads");
    }
    if (file_index < 0 || file_index >= (int)localFiles.size()) {
        throw std::invalid_argument("Invalid file index");
    }
    BruteForceFile(localFiles[file_index], Dict, jobs);
}

void ZipFile::BruteForceFile(ZipLocalFile const& zf, std::vector<std::vector<std::string>> const& Dict, int jobs) const {
    if (!zf.IsEncrypted()) {
        fprintf(stderr, "File is not encrypted, no need to brute-forcing...\n");
        return;
    }
    
    unsigned long long const LIMIT = 4e12; /* 64-bit on C++11 */
    unsigned long long num_jobs = 1;
    for (auto const& x: Dict) {
        if (num_jobs > LIMIT / x.size()) {
            throw std::invalid_argument("Too many password to bruteforce, consider limiting Dictionary entries");
        }
        num_jobs = num_jobs * x.size();
    }

    unsigned long long const total_job = num_jobs;
    int const Dict_sz = Dict.size();
    std::vector<unsigned long long> Base(Dict_sz, 1);
    for (int i = Dict_sz - 2; ~i; --i) Base[i] = Dict[i+1].size() * Base[i+1];

    std::atomic<unsigned long long> job_cnt(0);
    std::mutex cout_mutex;
    auto thread_job = [&]() {
        unsigned long long job_idx, job_idx2;
        std::vector<int> w_idx(Dict.size());
        ZipPassword zpwd;
        for (;;) {
            job_idx = job_idx2 = job_cnt++;
            if (job_idx >= total_job) break;
            zpwd = ZipPassword();
            for (int i = 0; i < Dict_sz; ++i) {
                w_idx[i] = job_idx / Base[i];
                job_idx -= Base[i] * w_idx[i];
                for (auto& x: Dict[i][w_idx[i]]) zpwd.UpdateKey(x);
            }

            try {
                ExtractDataWithPassword(zf, zpwd);
            }
            catch (std::exception& e) {
                continue;
            }
            std::lock_guard<std::mutex> guard(cout_mutex);
            std::cout << "Possible password: ";
            for (int i = 0; i < Dict_sz; ++i) std::cout << Dict[i][w_idx[i]];
            std::cout << std::endl;
        }
    };

    std::vector<std::thread> vThread(jobs);
    for (int i = 0; i < jobs; ++i) {
        vThread[i] = std::thread(thread_job);
    }
    for (int i = 0; i < jobs; ++i) {
        vThread[i].join();
    }
}
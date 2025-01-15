#include"ZipLocalFile.h"

#include"ZipUtil.h"
#include<stdexcept>

extern int const ZIP_VERSION_MAX_MAJOR;
extern int const ZIP_VERSION_MAX_MINOR;
extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

uint32_t const ZIP_LOCAL_FILE_HEADER_SIGNATURE = 0x04034b50;
uint32_t const ZIP_LOCAL_FILE_HEADER_DATA_DESCRIPTOR_SIGNATURE = 0x08074b50;

ZipLocalFile::ZipLocalFile(std::vector<uint8_t> const& data, size_t& _start_offset) {
    size_t u = _start_offset;
    if (!CheckSize(data, u, 30)) {
        throw std::length_error("ZipFile::ZipLocalFile::Constructor::Header: Unexpected EOF");
    }

    uint32_t headerSignature = GetUint32(data, u);
    if (headerSignature != ZIP_LOCAL_FILE_HEADER_SIGNATURE) {
        throw std::invalid_argument("ZipFile::ZipLocalFile::Constructor::Header: Invalid Header Signature");
    }

    version_needed = GetUint16(data, u);
    if (!version_supported(version_needed)) {
        int major = version_needed / 10, minor = version_needed % 10;
        snprintf(_zip_errmsg, _zip_errmsg_sz, "ZipFile::ZipLocalFile::Constructor::Header: Unsupported version, maximum supported version is %d.%d, got version %d.%d", 
            ZIP_VERSION_MAX_MAJOR, ZIP_VERSION_MAX_MINOR, major, minor);
        throw std::invalid_argument(_zip_errmsg);
    }

    general_purpose_bitflag = GetUint16(data, u);
    compression_method = GetUint16(data, u);
    last_mod_file_time = GetUint16(data, u);
    last_mod_file_date = GetUint16(data, u);
    crc32 = GetUint32(data, u);
    compressed_size = GetUint32(data, u);
    uncompressed_size = GetUint32(data, u);
    file_name_length = GetUint16(data, u);
    extra_field_length = GetUint16(data, u);

    if (!CheckSize(data, u, file_name_length)) {
        throw std::length_error("ZipFile::ZipLocalFile::Constructor::Header::FileName: Unexpected EOF");
    }
    file_name_start_offset = u;
    u += file_name_length;
    file_name_end_offset = u;

    if (!CheckSize(data, u, extra_field_length)) {
        throw std::length_error("ZipFile::ZipLocalFile::Constructor::Header::ExtraField: Unexpected EOF");
    }
    extra_field_start_offset = u;
    u += extra_field_length;
    extra_field_end_offset = u;

    if (!CheckSize(data, u, compressed_size)) {
        throw std::length_error("ZipFile::ZipLocalFile::Constructor::FileData: Unexpected EOF");
    }
    data_start_offset = u;
    u += compressed_size;
    data_end_offset = u;

    if (general_purpose_bitflag & 0b1000) {
        uint32_t maybe_header = GetUint32(data, u); // refer to 4.3.9.3
        if (maybe_header != ZIP_LOCAL_FILE_HEADER_DATA_DESCRIPTOR_SIGNATURE) {
            u -= 4; // what about crc32 collision? ignore for now
        }
        uint32_t crc32_2 = GetUint32(data, u);
        uint32_t compressed_size_2 = GetUint32(data, u);
        uint32_t uncompressed_size_2 = GetUint32(data, u);

        if (crc32_2 != crc32) {
            fprintf(stderr, "ZipFile::ZipLocalFile::Constructor::DataDescriptor::Warning: crc32 not match header, got: %08X, header: %08X\n", crc32_2, crc32);
        }
        if (compressed_size_2 != compressed_size) {
            fprintf(stderr, "ZipFile::ZipLocalFile::Constructor::DataDescriptor::Warning: compressed size not match header, got: %u, header: %u\n", compressed_size_2, compressed_size);
        }
        if (uncompressed_size_2 != uncompressed_size) {
            fprintf(stderr, "ZipFile::ZipLocalFile::Constructor::DataDescriptor::Warning: uncompressed size not match header, got: %u, header: %u\n", uncompressed_size_2, uncompressed_size);
        }
    }

    _start_offset = u;
}

std::pair<size_t,size_t> ZipLocalFile::GetFileName() const {
    return std::make_pair(file_name_start_offset, file_name_end_offset);
}

std::pair<size_t,size_t> ZipLocalFile::GetExtraField() const {
    return std::make_pair(extra_field_start_offset, extra_field_end_offset);
}

std::pair<size_t,size_t> ZipLocalFile::GetData() const {
    return std::make_pair(data_start_offset, data_end_offset);
}

int ZipLocalFile::IsEncrypted() const {
    return general_purpose_bitflag & 0b1;
}

uint16_t ZipLocalFile::GetCompressionMethod() const {
    return compression_method;
}

uint32_t ZipLocalFile::GetCrc32() const {
    return crc32;
}
uint16_t ZipLocalFile::GetVersionNeeded() const {
    return version_needed;
}

uint16_t ZipLocalFile::GetUncompressedSize() const {
    return uncompressed_size;
}

uint16_t ZipLocalFile::GetModTime() const {
    return last_mod_file_time;
}
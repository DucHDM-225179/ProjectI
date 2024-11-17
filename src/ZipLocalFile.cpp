#include"ZipLocalFile.h"

#include"ZipUtil.h"
#include<stdexcept>

int const VERSION_MAX_MAJOR = 2;
int const VERSION_MAX_MINOR = 0;
int version_supported(uint16_t version_needed) {
    int major = version_needed / 10;
    int minor = version_needed % 10;
    if (major > VERSION_MAX_MAJOR) return 0;
    else if (major == 2 && minor > VERSION_MAX_MINOR) return 0;

    return 1;
}
extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

ZipLocalFile::ZipLocalFile(std::vector<uint8_t> const& data, int& _start_offset) {
    int u = _start_offset;
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
        snprintf(_zip_errmsg, _zip_errmsg_sz, "ZipFile::ZipLocalFile::Constrcutor::Header: Unsupported version, maximum supportede version is %d.%d, got version %d.%d", 
            VERSION_MAX_MAJOR, VERSION_MAX_MINOR, major, minor);
        throw std::invalid_argument(_zip_errmsg);
    }

    general_purpose_bitflag = GetUint16(data, u);
    compression_method = GetUint16(data, u);
    last_mod_file_time = GetUint16(data, u);
    last_mod_file_date = GetUint16(data, u);
    crc32 = GetUint32(data, u);
    compressed_size = GetUint32(data, u);
    uncompressed_size = GetUint32(data, u);
    filename_length = GetUint16(data, u);
    extra_field_length = GetUint16(data, u);

    if (!CheckSize(data, u, filename_length)) {
        throw std::length_error("ZipFile::ZipLocalFile::Constructor::Header::FileName: Unexpected EOF");
    }
    filename_start_offset = u;
    u += filename_length;
    filename_end_offset = u;

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

std::pair<int,int> ZipLocalFile::GetFileName() {
    return std::make_pair(filename_start_offset, filename_end_offset);
}
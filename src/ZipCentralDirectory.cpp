#include"ZipCentralDirectory.h"

#include"ZipUtil.h"
#include<stdexcept>

uint32_t const ZIP_CENTRAL_FILE_HEADER_SIGNATURE = 0x02014b50;

ZipCentralDirectory::ZipCentralDirectory(std::vector<uint8_t> const& data, int& _start_offset) {
    int u = _start_offset;
    if (!CheckSize(data, u, 46)) {
        throw std::length_error("ZipFile::ZipCentralDirectory::Constructor: Unexpected EOF");
    }

    uint32_t headerSignature = GetUint32(data, u);
    if (headerSignature != ZIP_CENTRAL_FILE_HEADER_SIGNATURE) {
        throw std::invalid_argument("ZipFile::ZipCentralDirectory::Constructor: Invalid Header Signature");
    }

    version_made_by = GetUint16(data, u);
    version_needed_to_extract = GetUint16(data, u);
    if (!version_supported(version_needed_to_extract)) {
        int major = version_needed_to_extract / 10;
        int minor = version_needed_to_extract % 10;
        snprintf(_zip_errmsg, _zip_errmsg_sz, "ZipFile::ZipCentralDirectory::Constructor: Unsupported version, maximum supported version is %d.%d, got version %d.%d", 
            ZIP_VERSION_MAX_MAJOR, ZIP_VERSION_MAX_MINOR, major, minor);
        throw std::invalid_argument(_zip_errmsg);
    }

    general_purpose_bit_flag = GetUint16(data, u);
    compression_method = GetUint16(data, u);
    last_mod_file_time = GetUint16(data, u);
    last_mod_file_date = GetUint16(data, u);
    crc32 = GetUint32(data, u);
    compressed_size = GetUint32(data, u);
    uncompressed_size = GetUint32(data, u);
    file_name_length = GetUint16(data, u);
    extra_field_length = GetUint16(data, u);
    file_comment_length = GetUint16(data, u);
    disk_number_start = GetUint16(data, u);
    internal_file_attribute = GetUint16(data, u);
    external_file_attribute = GetUint32(data, u);
    relative_offset_of_local_header = GetUint32(data, u);

    if (!CheckSize(data, u, file_name_length)) {
        throw std::length_error("ZipFile::ZipCentralDirectory::Constructor: Unexpected EOF");
    }
    file_name_start_offset = u;
    u += file_name_length;
    file_name_end_offset = u;

    if (!CheckSize(data, u, extra_field_length)) {
        throw std::length_error("ZipFile::ZipCentralDirectory::Constructor: Unexpected EOF");
    }
    extra_field_start_offset = u;
    u += extra_field_length;
    extra_field_end_offset = u;

    if (!CheckSize(data, u, file_comment_length)) {
        throw std::length_error("ZipFile::ZipCentralDirectory::Constructor: Unexpected EOF");
    }
    file_comment_start_offset = u;
    u += file_comment_length;
    file_comment_end_offset = u;

    _start_offset = u;
}

std::pair<int,int> ZipCentralDirectory::GetFileName() const{
    return std::make_pair(file_name_start_offset, file_name_end_offset);
}
std::pair<int,int> ZipCentralDirectory::GetExtraField() const{
    return std::make_pair(extra_field_start_offset, extra_field_end_offset);
}
std::pair<int,int> ZipCentralDirectory::GetFileComment() const{
    return std::make_pair(file_comment_start_offset, file_comment_end_offset);
}
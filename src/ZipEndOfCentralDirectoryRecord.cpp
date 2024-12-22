#include"ZipEndOfCentralDirectoryRecord.h"

#include "ZipUtil.h"
#include<stdexcept>

uint32_t ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_SIGNATURE = 0x06054b50;

ZipEndOfCentralDirectoryRecord::ZipEndOfCentralDirectoryRecord(std::vector<uint8_t> const& data, size_t& _start_offset) {
    size_t u = _start_offset;
    
    if (!CheckSize(data, u, 22)) {
        throw std::length_error("ZipFile::ZipEndOfCentralDirectoryRecord::Constructor: Unexpected EOF");
    }

    uint32_t headerSignature = GetUint32(data, u);
    if (headerSignature != ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_SIGNATURE) {
        throw std::invalid_argument("ZipFile::ZipEndOfCentralDirectoryRecord::Constructor: Invalid Header Signature");
    }

    number_of_this_disk = GetUint16(data, u);
    number_of_the_disk_with_the_start_of_the_central_directory = GetUint16(data, u);
    total_number_of_entries_in_the_central_directory_on_this_disk = GetUint16(data, u);
    total_number_of_entries_in_the_central_directory = GetUint16(data, u);
    size_of_the_central_directory = GetUint32(data, u);
    offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number = GetUint32(data, u);
    dot_zip_file_comment_length = GetUint16(data, u);

    if (!CheckSize(data, u, dot_zip_file_comment_length)) {
        throw std::length_error("ZipFile::ZipEndOfCentralDirectoryRecord::Constructor: Unexpected EOF");
    }
    dot_zip_file_comment_start_offset = u;
    u += dot_zip_file_comment_length;
    dot_zip_file_comment_end_offset = u;

    _start_offset = u;
}

std::pair<size_t,size_t> ZipEndOfCentralDirectoryRecord::GetDotZipFileComment() const{
    return std::make_pair(dot_zip_file_comment_start_offset, dot_zip_file_comment_end_offset);
}

ZipEndOfCentralDirectoryRecord::ZipEndOfCentralDirectoryRecord() {
    dot_zip_file_comment_start_offset = dot_zip_file_comment_end_offset = -1;
}
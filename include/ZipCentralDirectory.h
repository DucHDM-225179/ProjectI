#ifndef ZIP_CENTRAL_DIRECTORY_H_
#define ZIP_CENTRAL_DIRECTORY_H_

#include<cstdint>
#include<vector>
#include<utility>
#include<cstddef>

extern uint32_t const ZIP_CENTRAL_FILE_HEADER_SIGNATURE;

// Class chứa thông tin Central Directory
class ZipCentralDirectory {
public:
    ZipCentralDirectory(std::vector<uint8_t> const& data, size_t& _start_offset);
    std::pair<size_t,size_t> GetFileName() const;
    std::pair<size_t,size_t> GetExtraField() const;
    std::pair<size_t,size_t> GetFileComment() const;
protected:
    uint16_t version_made_by;
    uint16_t version_needed_to_extract;
    uint16_t general_purpose_bit_flag;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint16_t file_comment_length;
    uint16_t disk_number_start;
    uint16_t internal_file_attribute;
    uint32_t external_file_attribute;
    uint32_t relative_offset_of_local_header;

    size_t file_name_start_offset, file_name_end_offset;
    size_t extra_field_start_offset, extra_field_end_offset;
    size_t file_comment_start_offset, file_comment_end_offset;
};

#endif // ZIP_CENTRAL_DIRECTORY_H_
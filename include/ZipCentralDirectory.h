#ifndef ZIP_CENTRAL_DIRECTORY_H_
#define ZIP_CENTRAL_DIRECTORY_H_

#include<cstdint>
#include<vector>
#include<utility>

extern uint32_t const ZIP_CENTRAL_FILE_HEADER_SIGNATURE;

class ZipCentralDirectory {
public:
    ZipCentralDirectory(std::vector<uint8_t> const& data, int& _start_offset);
    std::pair<int,int> GetFileName() const;
    std::pair<int,int> GetExtraField() const;
    std::pair<int,int> GetFileComment() const;
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

    int file_name_start_offset, file_name_end_offset;
    int extra_field_start_offset, extra_field_end_offset;
    int file_comment_start_offset, file_comment_end_offset;
};

#endif // ZIP_CENTRAL_DIRECTORY_H_
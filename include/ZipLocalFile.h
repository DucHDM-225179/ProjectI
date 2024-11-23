#include<cstdint>
#include<vector>
#include<string>
#include<utility>

#ifndef ZIP_LOCAL_FILE_H_
#define ZIP_LOCAL_FILE_H_

extern uint32_t const ZIP_LOCAL_FILE_HEADER_SIGNATURE;
extern uint32_t const ZIP_LOCAL_FILE_HEADER_DATA_DESCRIPTOR_SIGNATURE;

class ZipLocalFile {
public:
    ZipLocalFile(std::vector<uint8_t> const& data, int& _start_offset);
    std::pair<int,int> GetFileName() const;
    std::pair<int,int> GetExtraField() const;
    std::pair<int,int> GetData() const;
    uint16_t GetCompressionMethod() const;

private:
    uint16_t version_needed;
    uint16_t general_purpose_bitflag;
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_name;
    uint16_t last_mod_file_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;

    int file_name_start_offset, file_name_end_offset;
    int extra_field_start_offset, extra_field_end_offset;
    int data_start_offset, data_end_offset;
    std::string file_name;
};

#endif // ZIP_LOCAL_FILE_H_
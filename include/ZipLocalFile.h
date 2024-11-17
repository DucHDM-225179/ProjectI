#include<cstdint>
#include<vector>
#include<string>
#include<utility>

#ifndef ZIP_LOCAL_FILE_H_
#define ZIP_LOCAL_FILE_H_

uint32_t const ZIP_LOCAL_FILE_HEADER_SIGNATURE = 0x04034b50;
uint32_t const ZIP_LOCAL_FILE_HEADER_DATA_DESCRIPTOR_SIGNATURE = 0x08074b50;

class ZipLocalFile {
public:
    ZipLocalFile(std::vector<uint8_t> const& data, int& start_offset);
    std::pair<int,int> GetFileName();

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
    uint16_t filename_length;
    uint16_t extra_field_length;

    int filename_start_offset, filename_end_offset;
    int extra_field_start_offset, extra_field_end_offset;
    int data_start_offset, data_end_offset;
    std::string filename;
};

#endif // ZIP_LOCAL_FILE_H_
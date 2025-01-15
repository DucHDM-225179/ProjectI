#include<cstdint>
#include<vector>
#include<string>
#include<utility>
#include<cstddef>

#ifndef ZIP_LOCAL_FILE_H_
#define ZIP_LOCAL_FILE_H_

extern uint32_t const ZIP_LOCAL_FILE_HEADER_SIGNATURE;
extern uint32_t const ZIP_LOCAL_FILE_HEADER_DATA_DESCRIPTOR_SIGNATURE;

// Class chứa thông tin local file trong file zip 
class ZipLocalFile {
public:
    ZipLocalFile(std::vector<uint8_t> const& data, size_t& _start_offset);
    std::pair<size_t,size_t> GetFileName() const;
    std::pair<size_t,size_t> GetExtraField() const;
    std::pair<size_t,size_t> GetData() const;
    int IsEncrypted() const;
    uint16_t GetCompressionMethod() const;
    uint32_t GetCrc32() const;
    uint16_t GetVersionNeeded() const;
    uint16_t GetUncompressedSize() const;
    uint16_t GetModTime() const;

protected:
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

    size_t file_name_start_offset, file_name_end_offset;
    size_t extra_field_start_offset, extra_field_end_offset;
    size_t data_start_offset, data_end_offset;
    std::string file_name;
};

#endif // ZIP_LOCAL_FILE_H_
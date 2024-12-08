#ifndef ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_H_
#define ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_H_

#include<cstdint>
#include<utility>
#include<vector>

extern uint32_t ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_SIGNATURE;

// Class chứ thông tin End Of Central Directory Record
class ZipEndOfCentralDirectoryRecord {
public:
    ZipEndOfCentralDirectoryRecord();
    ZipEndOfCentralDirectoryRecord(std::vector<uint8_t> const& data, int& _start_offset);
    std::pair<int,int> GetDotZipFileComment() const;

protected:
    uint16_t number_of_this_disk;
    uint16_t number_of_the_disk_with_the_start_of_the_central_directory;
    uint16_t total_number_of_entries_in_the_central_directory_on_this_disk;
    uint16_t total_number_of_entries_in_the_central_directory;
    uint32_t size_of_the_central_directory;
    uint32_t offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number;
    uint16_t dot_zip_file_comment_length;

    int dot_zip_file_comment_start_offset, dot_zip_file_comment_end_offset;
};

#endif // ZIP_END_OF_CENTRAL_DIRECTORY_RECORD_H_
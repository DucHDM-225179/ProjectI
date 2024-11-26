#include<string>
#include<vector>
#include<cstdint>
#include"ZipLocalFile.h"
#include"ZipCentralDirectory.h"
#include"ZipEndOfCentralDirectoryRecord.h"

#ifndef ZIP_FILE_H_
#define ZIP_FILE_H_

extern uint32_t ZIP_DIGITAL_SIGNATURE_HEADER;


class ZipFile {
public:
    ZipFile(std::string filepath);
    std::vector<std::string> GetFileList() const;
    std::vector<std::uint8_t> ExtractData(int file_index) const;
    std::vector<std::uint8_t> ExtractDataWithPassword(int file_index, std::string const& pwd) const;

protected:
    std::vector<uint8_t> rawData;
    std::vector<ZipLocalFile> localFiles;
    std::vector<ZipCentralDirectory> centralDirectories;
    ZipEndOfCentralDirectoryRecord endOfCentralDirectoryRecord;
    std::vector<uint8_t> ExtractData(ZipLocalFile const& zf) const;
    std::vector<uint8_t> ExtractDataWithPassword(ZipLocalFile const& zf, std::string const& pwd) const;


    int digital_signature_data_start_offset, digital_signature_data_end_offset;
};

#endif // ZIP_FILE_H_
#include<string>
#include<vector>
#include<cstdint>
#include"ZipLocalFile.h"

#ifndef ZIP_FILE_H_
#define ZIP_FILE_H_

class ZipFile {
public:
    ZipFile(std::string filepath);

private:
    std::vector<uint8_t> rawData;
    std::vector<ZipLocalFile> localFiles;
};

#endif // ZIP_FILE_H_
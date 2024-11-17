#include"ZipFile.h"

#include"ZipUtil.h"
#include<stdexcept>
#include<iostream>
#include<fstream>
#include<utility>
#include<iterator>

int const SIZE_LIMIT = 256 * 1024 * 1024; // 256 MB
extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

ZipFile::ZipFile(std::string filepath) {
    try {
        std::ifstream fn(filepath, std::ios::binary);
        fn.unsetf(std::ios::skipws);
        std::streampos fsize;
        fn.seekg(0, std::ios::end);
        fsize = fn.tellg();
        if (fsize > SIZE_LIMIT) {
            snprintf(_zip_errmsg, _zip_errmsg_sz, "ZipFile::Constructor: File size too big, maximum supported size is %d MB", SIZE_LIMIT / 1024 / 1024);
            throw std::length_error(_zip_errmsg);
        }
        fn.seekg(0, std::ios::beg);

        rawData = std::vector<uint8_t>();
        rawData.reserve(fsize);
        rawData.insert(rawData.begin(), 
            std::istream_iterator<uint8_t>(fn), 
            std::istream_iterator<uint8_t>());
    } catch (std::ifstream::failure &e) {
        throw std::invalid_argument("ZipFile::Constructor: Cannot read zip file");
    }
    localFiles = std::vector<ZipLocalFile>();

    for (int curpos = 0; curpos < (int)rawData.size();) {
        uint32_t header = GetUint32(rawData, curpos);
        curpos -= 4; //unget
        if (header == ZIP_LOCAL_FILE_HEADER_SIGNATURE) {
            try {
                localFiles.emplace_back(ZipLocalFile(rawData, curpos));
            } catch (std::exception &e) {
                throw e;
            }
        }
        else {
            break;
        }
    }

    for (ZipLocalFile& lf: localFiles) {
        std::pair<int,int> pii = lf.GetFileName();
        for (int i = pii.first; i < pii.second;) {
            fprintf(stdout, "%c", rawData[i]);
            i += 1;
        }
        fprintf(stdout, "\n");
    }
}
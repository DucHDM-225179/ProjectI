#ifndef ZIP_DEFLATE_H_
#define ZIP_DEFLATE_H_

#include"ZipBitStream.h"

#include<cstdint>
#include<vector>

class ZipDeflate {
public:
    ZipDeflate(std::vector<uint8_t> const& data, int _start_offset, int _end_offset) : bitStream(data, _start_offset, _end_offset) {}
    std::vector<uint8_t> Decode();
private:
    ZipBitStream bitStream;
};

#endif // ZIP_DEFLATE_H_
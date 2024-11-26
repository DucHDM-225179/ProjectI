#ifndef ZIP_DEFLATE_H_
#define ZIP_DEFLATE_H_

#include"ZipBitStream.h"

#include<cstdint>
#include<vector>

class ZipDeflate {
public:
    ZipDeflate(std::vector<uint8_t> const& data) {
        bitStream = new ZipBitStream(data);
    }
    ZipDeflate(std::vector<uint8_t> const& data, int _start_offset, int _end_offset) {
        bitStream = new ZipBitStream(data, _start_offset, _end_offset);
    }
    ZipDeflate(std::vector<uint8_t> const& data, int _start_offset, int _end_offset, ZipPassword initKey) {
        bitStream = new ZipBitStreamEncrypted(data, _start_offset, _end_offset, initKey);
    }
    ~ZipDeflate() {
        delete bitStream;
    }
    std::vector<uint8_t> Decode();
protected:
    ZipBitStreamInterface *bitStream;
};

#endif // ZIP_DEFLATE_H_
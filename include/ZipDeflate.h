#ifndef ZIP_DEFLATE_H_
#define ZIP_DEFLATE_H_

#include"ZipBitStream.h"

#include<cstdint>
#include<vector>
#include<memory>

// Class xử lý giải nén dòng bit Deflate
class ZipDeflate {
public:
    ZipDeflate(std::vector<uint8_t> const& data) {
        bitStream = std::unique_ptr<ZipBitStreamInterface>(new ZipBitStream(data));
    }
    ZipDeflate(std::vector<uint8_t> const& data, int _start_offset, int _end_offset) {
        bitStream = std::unique_ptr<ZipBitStreamInterface>(new ZipBitStream(data, _start_offset, _end_offset));
    }
    ZipDeflate(std::vector<uint8_t> const& data, int _start_offset, int _end_offset, ZipPassword initKey) {
        bitStream = std::unique_ptr<ZipBitStreamInterface>(new ZipBitStreamEncrypted(data, _start_offset, _end_offset, initKey));
    }
    std::vector<uint8_t> Decode();
protected:
    std::unique_ptr<ZipBitStreamInterface> bitStream;
};

#endif // ZIP_DEFLATE_H_
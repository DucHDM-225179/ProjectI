#ifndef ZIP_BITSTREAM_H_
#define ZIP_BITSTREAM_H_

#include<vector>
#include<cstdint>

class ZipBitStream {
public:
    ZipBitStream(std::vector<uint8_t> const& d) : data(d), cursor(0), bitsize(d.size()*8) {}
    uint32_t GetBit() const;
    int SkipBit(int bitlength);

private:
    std::vector<uint8_t> const& data;
    int const bitsize;
    int cursor;
};

#endif // ZIP_BITSTREAM_H_
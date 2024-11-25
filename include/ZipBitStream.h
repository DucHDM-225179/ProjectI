#ifndef ZIP_BITSTREAM_H_
#define ZIP_BITSTREAM_H_

#include<vector>
#include<cstdint>

class ZipBitStream {
public:
    ZipBitStream() : data(std::vector<uint8_t>(0)), bitsize(0), initial_cursor(0), cursor(0) {}
    ZipBitStream(std::vector<uint8_t> const& d) : data(d), bitsize(d.size()*8), initial_cursor(0), cursor(0) {}
    ZipBitStream(std::vector<uint8_t> const& d, int start_byte_offset, int end_byte_offset) 
        : data(d), bitsize((end_byte_offset - start_byte_offset)*8), 
        initial_cursor(8*start_byte_offset), cursor(8*start_byte_offset) {}
    uint32_t GetBit() const;
    int SkipToByte();
    int SkipBit(int bitlength);
    void Reset();

private:
    std::vector<uint8_t> const& data;
    int const bitsize;
    int const initial_cursor;
    int cursor;
};

#endif // ZIP_BITSTREAM_H_
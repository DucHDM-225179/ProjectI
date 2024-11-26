#ifndef ZIP_BITSTREAM_H_
#define ZIP_BITSTREAM_H_

#include<vector>
#include<cstdint>
#include"ZipUtil.h"

class ZipBitStreamInterface {
public:
    ZipBitStreamInterface() {}
    ZipBitStreamInterface(std::vector<uint8_t> const& d) {}
    ZipBitStreamInterface(std::vector<uint8_t> const& d, int start_byte_offset, int end_byte_offset) {}
    ZipBitStreamInterface(std::vector<uint8_t> const& d, int start_byte_offset, int end_byte_offset, ZipPassword key) {}
    virtual uint32_t GetBit() const = 0;
    virtual int SkipToByte() = 0;
    virtual int SkipBit(int bitlength) = 0;
    virtual void Reset() = 0;
};

class ZipBitStream : public ZipBitStreamInterface {
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

protected:
    std::vector<uint8_t> const& data;
    int const bitsize;
    int const initial_cursor;
    int cursor;
};

class ZipBitStreamEncrypted : public ZipBitStream {
public:
    ZipBitStreamEncrypted() : ZipBitStream(), initKey(ZipPassword()) {
        setup_initial_key();
    }
    ZipBitStreamEncrypted(std::vector<uint8_t> const& d) : ZipBitStream(d), initKey(ZipPassword()) {
        setup_initial_key();
    }
    ZipBitStreamEncrypted(std::vector<uint8_t> const& d, int start_byte_offset, int end_byte_offset) : ZipBitStream(d, start_byte_offset, end_byte_offset), initKey(ZipPassword()) {
        setup_initial_key();
    }
    ZipBitStreamEncrypted(std::vector<uint8_t> const& d, int start_byte_offset, int end_byte_offset, ZipPassword initKey) : ZipBitStream(d, start_byte_offset, end_byte_offset), initKey(initKey) {
        setup_initial_key();
    }
    uint32_t GetBit() const;
    int SkipToByte();
    int SkipBit(int bitlength);
    void Reset();

protected:
    ZipPassword const initKey;
    ZipPassword pwdKey;
    uint8_t zpwd[8]; // must be power of 2 for bit tricks
    int const zpwd_sz = sizeof(zpwd) / sizeof(zpwd[0]);
    int const zpwd_modulo = zpwd_sz - 1;
    void setup_initial_key();
};

#endif // ZIP_BITSTREAM_H_
#ifndef ZIP_BITSTREAM_H_
#define ZIP_BITSTREAM_H_

#include<vector>
#include<cstdint>
#include<cstddef>
#include"ZipUtil.h"

// Class xử lý truyền dòng Bit
// Sử dụng một dạng Interface vì ta muốn xử lý online với dòng bit được mã hoá
class ZipBitStreamInterface {
public:
    ZipBitStreamInterface() {}
    virtual ~ZipBitStreamInterface() {}
    ZipBitStreamInterface(std::vector<uint8_t> const& d) {}
    ZipBitStreamInterface(std::vector<uint8_t> const& d, size_t start_byte_offset, size_t end_byte_offset) {}
    ZipBitStreamInterface(std::vector<uint8_t> const& d, size_t start_byte_offset, size_t end_byte_offset, ZipPassword key) {}
    virtual uint32_t GetBit() const = 0;
    virtual int SkipToByte() = 0;
    virtual int SkipBit(int bitlength) = 0;
    virtual void Reset() = 0;
};

// Xử lý truyền dòng Bit với file không mã hoá
class ZipBitStream : public ZipBitStreamInterface {
public:
    ZipBitStream() : data(local_nullref), bytesize(0), initial_cursor(0), cursor(0), cursor_bit_read(0) {}
    ZipBitStream(std::vector<uint8_t> const& d) : data(d), bytesize(d.size()), initial_cursor(0), cursor(0), cursor_bit_read(0) {}
    ZipBitStream(std::vector<uint8_t> const& d, size_t start_byte_offset, size_t end_byte_offset) 
        : data(d), bytesize((end_byte_offset - start_byte_offset)), 
        initial_cursor(start_byte_offset), cursor(start_byte_offset), cursor_bit_read(0) {}
    uint32_t GetBit() const;
    int SkipToByte();
    int SkipBit(int bitlength);
    void Reset();

protected:
    std::vector<uint8_t> const& data;
    size_t const bytesize;
    size_t const initial_cursor;
    size_t cursor;
    int cursor_bit_read; // 0 <= bit_read <= 7

private:
    static std::vector<uint8_t> local_nullref;
};

// Xử lý truyền dòng bit với file mã hoá
class ZipBitStreamEncrypted : public ZipBitStream {
public:
    ZipBitStreamEncrypted() : ZipBitStream(), initKey(ZipPassword()) {
        setup_initial_key();
    }
    ZipBitStreamEncrypted(std::vector<uint8_t> const& d) : ZipBitStream(d), initKey(ZipPassword()) {
        setup_initial_key();
    }
    ZipBitStreamEncrypted(std::vector<uint8_t> const& d, size_t start_byte_offset, size_t end_byte_offset) : ZipBitStream(d, start_byte_offset, end_byte_offset), initKey(ZipPassword()) {
        setup_initial_key();
    }
    ZipBitStreamEncrypted(std::vector<uint8_t> const& d, size_t start_byte_offset, size_t end_byte_offset, ZipPassword initKey) : ZipBitStream(d, start_byte_offset, end_byte_offset), initKey(initKey) {
        setup_initial_key();
    }
    uint32_t GetBit() const;
    int SkipToByte();
    int SkipBit(int bitlength);
    void Reset();

protected:
    ZipPassword const initKey;
    ZipPassword pwdKey;
    uint8_t zpwd[8]; // phải là số dạng mũ 2 để có thể tính mod xoay vòng bằng phép toán AND
    int const zpwd_sz = sizeof(zpwd) / sizeof(zpwd[0]);
    int const zpwd_modulo = zpwd_sz - 1;
    void setup_initial_key();
};

#endif // ZIP_BITSTREAM_H_
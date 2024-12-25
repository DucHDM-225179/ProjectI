#include"ZipBitStream.h"

#include"ZipUtil.h"

uint32_t ZipBitStream::GetBit() const {
    uint32_t w = 0;
    size_t cursor_byte = cursor; 
    // Sẽ hợp lệ trong một file Zip, bởi vì luôn có các phần khác trong file Zip có độ lớn lớn hơn 0
    // Vì vậy phần dữ liệu nén luôn phải là một số hợp lệ trong 32 bit
    // --> Không xảy ra overflow
    size_t const cursor_byte_end = (initial_cursor+bytesize);
    if (cursor_byte_end - cursor_byte >= 4) {
        w = (data[cursor_byte  ]      ) |
            (data[cursor_byte+1] <<  8) |
            (data[cursor_byte+2] << 16) |
            (data[cursor_byte+3] << 24);
    }
    else {
        for (size_t i = 0, j = 0; i < (cursor_byte_end-cursor_byte); ++i, j += 8) w |= data[cursor_byte+i] << j;
    }
    return w >> (cursor_bit_read);
}

int ZipBitStream::SkipBit(int bitlength) {
    size_t const cursor_byte_end = (initial_cursor+bytesize);
    if (cursor_bit_read) {
        int bit_left_to_advance = 8 - cursor_bit_read;
        if (bitlength < bit_left_to_advance) {
            cursor_bit_read += bitlength;
            return 1;
        }
        // đọc xong byte hiện tại, tiến thêm 1 byte
        if (1U > cursor_byte_end || cursor > cursor_byte_end - 1) {
            return 0;
        }
        cursor_bit_read = 0;
        cursor += 1;
        bitlength -= bit_left_to_advance;
        if (bitlength == 0) return 1;
    }

    int cursor_new_bit = bitlength;
    size_t cursor_new_byte = cursor_new_bit / 8; // tối đa 4 byte, do buffer chỉ chứa 32 bit -> skip tối đa 32 bit
    if (cursor_new_byte > cursor_byte_end || cursor > cursor_byte_end - cursor_new_byte) {
        // overflow
        return 0;
    }
    cursor += cursor_new_byte;
    cursor_bit_read = cursor_new_bit % 8;
    return 1;
}

int ZipBitStream::SkipToByte() {
    if (cursor_bit_read) return SkipBit(8 - cursor_bit_read);
    else return 1;
}
void ZipBitStream::Reset() {
    cursor = initial_cursor;
}

void ZipBitStreamEncrypted::setup_initial_key() {
    size_t cursor_byte = cursor;
    pwdKey = initKey;
    for (size_t i = 0; i < bytesize && i < zpwd_sz; ++i) {
        zpwd[(cursor_byte+i)&zpwd_modulo] = pwdKey.DecryptByte() ^ data[cursor_byte+i];
        pwdKey.UpdateKey(zpwd[(cursor_byte+i)&zpwd_modulo]);
    }
}
uint32_t ZipBitStreamEncrypted::GetBit() const {
    uint32_t w = 0;
    size_t cursor_byte = cursor;
    size_t const cursor_byte_end = (initial_cursor+bytesize);
    if (cursor_byte_end - cursor_byte >= 4) {
        w = (zpwd[ cursor_byte   &zpwd_modulo]       )|
            (zpwd[(cursor_byte+1)&zpwd_modulo] <<  8 )|
            (zpwd[(cursor_byte+2)&zpwd_modulo] << 16 )|
            (zpwd[(cursor_byte+3)&zpwd_modulo] << 24 );
    }
    else {
        for (size_t i = 0, j = 0; i < (cursor_byte_end-cursor_byte); ++i, j += 8) w |= (zpwd[(cursor_byte+i)&zpwd_modulo]) << j;
    }
    return w >> (cursor_bit_read);
}

int ZipBitStreamEncrypted::SkipBit(int bitlength) {
    size_t const cursor_byte_end = (initial_cursor+bytesize);
    if (cursor_bit_read) {
        int bit_left_to_advance = 8 - cursor_bit_read;
        if (bitlength < bit_left_to_advance) {
            cursor_bit_read += bitlength;
            return 1;
        }
        // đọc xong byte hiện tại, tiến thêm 1 byte
        if (1U > cursor_byte_end || cursor > cursor_byte_end - 1) {
            return 0;
        }
        cursor_bit_read = 0;

        size_t new_entry_pass_idx = cursor + zpwd_sz;
        zpwd[new_entry_pass_idx&zpwd_modulo] = pwdKey.DecryptByte() ^ data[new_entry_pass_idx];
        pwdKey.UpdateKey(zpwd[new_entry_pass_idx&zpwd_modulo]);
        
        cursor += 1;
        bitlength -= bit_left_to_advance;
        if (bitlength == 0) return 1;
    }

    int cursor_new_bit = bitlength;
    size_t cursor_new_byte = cursor_new_bit / 8;
    if (cursor_new_byte > cursor_byte_end || cursor > cursor_byte_end - cursor_new_byte) {
        return 0;
    }
    
    size_t new_cursor = cursor + cursor_new_byte;
    if (cursor != new_cursor) {
        for (size_t i = cursor + zpwd_sz; i < new_cursor + zpwd_sz && i < cursor_byte_end; ++i) {
            zpwd[i&zpwd_modulo] = pwdKey.DecryptByte() ^ data[i];
            pwdKey.UpdateKey(zpwd[i&zpwd_modulo]);
        }
    } 
    cursor = new_cursor;
    cursor_bit_read = cursor_new_bit % 8;
    return 1;
}

int ZipBitStreamEncrypted::SkipToByte() {
    if (cursor_bit_read) return SkipBit(8 - cursor_bit_read);
    else return 0;
}

void ZipBitStreamEncrypted::Reset() {
    cursor = initial_cursor;
    setup_initial_key();
}
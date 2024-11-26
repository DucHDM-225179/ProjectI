#include"ZipBitStream.h"

#include"ZipUtil.h"

uint32_t ZipBitStream::GetBit() const {
    uint32_t w = 0;
    int cursor_byte = cursor / 8; // cursor >> 3
    int const cursor_byte_end = (initial_cursor+bitsize)/8;
    if (cursor_byte_end - cursor_byte >= 4) {
        w = (data[cursor_byte  ]      ) |
            (data[cursor_byte+1] <<  8) |
            (data[cursor_byte+2] << 16) |
            (data[cursor_byte+3] << 24);
    }
    else {
        for (int i = 0, j = 0; i < (int)(cursor_byte_end-cursor_byte); ++i, j += 8) w |= data[cursor_byte+i] << j;
    }
    return w >> (cursor%8); // cursor & 7
}
int ZipBitStream::SkipBit(int bitlength) {
    if (bitsize + initial_cursor - cursor < bitlength) return 0;
    cursor += bitlength; 
    return 1;
}
int ZipBitStream::SkipToByte() {
    if (cursor % 8) return SkipBit(8 - cursor%8);
    else return 0;
}
void ZipBitStream::Reset() {
    cursor = initial_cursor;
}

void ZipBitStreamEncrypted::setup_initial_key() {
    int cursor_byte = cursor / 8;
    pwdKey = initKey;
    for (int i = 0; i < bitsize/8 && i < zpwd_sz; ++i) {
        zpwd[(cursor_byte+i)&zpwd_modulo] = pwdKey.DecryptByte() ^ data[cursor_byte+i];
        pwdKey.UpdateKey(zpwd[(cursor_byte+i)&zpwd_modulo]);
    }
}
uint32_t ZipBitStreamEncrypted::GetBit() const {
    uint32_t w = 0;
    int cursor_byte = cursor / 8;
    int const cursor_byte_end = (initial_cursor+bitsize)/8;
    if (cursor_byte_end - cursor_byte >= 4) {
        w = (zpwd[ cursor_byte   &zpwd_modulo]       )|
            (zpwd[(cursor_byte+1)&zpwd_modulo] <<  8 )|
            (zpwd[(cursor_byte+2)&zpwd_modulo] << 16 )|
            (zpwd[(cursor_byte+3)&zpwd_modulo] << 24 );
    }
    else {
        for (int i = 0, j = 0; i < (int)(cursor_byte_end-cursor_byte); ++i, j += 8) w |= (zpwd[(cursor_byte+i)&zpwd_modulo]) << j;
    }
    return w >> (cursor%8);
}

int ZipBitStreamEncrypted::SkipBit(int bitlength) {
    if (bitsize + initial_cursor - cursor < bitlength) return 0;
    int end_byte = (bitsize+initial_cursor)/8;
    int new_cursor = cursor + bitlength;
    if (cursor / 8 != new_cursor / 8) {
        for (int i = cursor / 8 + zpwd_sz; i < new_cursor / 8 + zpwd_sz && i < end_byte; ++i) {
            zpwd[i&zpwd_modulo] = pwdKey.DecryptByte() ^ data[i];
            pwdKey.UpdateKey(zpwd[i&zpwd_modulo]);
        }
    } 
    cursor = new_cursor;
    return 1;
}

int ZipBitStreamEncrypted::SkipToByte() {
    if (cursor % 8) return SkipBit(8 - cursor%8);
    else return 0;
}

void ZipBitStreamEncrypted::Reset() {
    cursor = initial_cursor;
    setup_initial_key();
}
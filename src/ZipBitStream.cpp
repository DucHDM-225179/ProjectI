#include"ZipBitStream.h"

#include"ZipUtil.h"

uint32_t ZipBitStream::GetBit() const {
    uint32_t w = 0;
    int cursor_byte = cursor / 8; // cursor >> 3
    if (data.size() - cursor_byte >= 4) {
        w = (data[cursor_byte]        ) |
            (data[cursor_byte+1] <<  8) |
            (data[cursor_byte+2] << 16) |
            (data[cursor_byte+3] << 24);
    }
    else {
        for (int i = 0, j = 0; i < (int)(data.size()-cursor_byte); ++i, j += 8) w |= data[cursor_byte+i] << j;
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
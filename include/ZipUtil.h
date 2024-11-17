#ifndef ZIP_UTIL_H_
#define ZIP_UTIL_H_

#include<vector>
#include<cstdint>

extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

int CheckSize(std::vector<uint8_t> const& data, int start_offset, uint32_t sz);
uint32_t GetUint32(std::vector<uint8_t> const& data, int& start_offset);
uint16_t GetUint16(std::vector<uint8_t> const& data, int& start_offset);
uint8_t GetUint8(std::vector<uint8_t> const& data, int& start_offset);

#endif // ZIP_UTIL_H_
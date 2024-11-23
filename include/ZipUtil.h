#ifndef ZIP_UTIL_H_
#define ZIP_UTIL_H_

#include<vector>
#include<cstdint>

extern int const ZIP_VERSION_MAX_MAJOR;
extern int const ZIP_VERSION_MAX_MINOR;
int version_supported(uint16_t version_needed);

extern char _zip_errmsg[];
extern int const _zip_errmsg_sz;

extern uint32_t const crc32_tbl[];

int CheckSize(std::vector<uint8_t> const& data, int start_offset, uint32_t sz);
uint32_t GetUint32(std::vector<uint8_t> const& data, int& start_offset);
uint16_t GetUint16(std::vector<uint8_t> const& data, int& start_offset);
uint8_t GetUint8(std::vector<uint8_t> const& data, int& start_offset);
uint32_t crc32_compute(std::vector<uint8_t> const& data);

#endif // ZIP_UTIL_H_
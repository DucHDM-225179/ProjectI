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

extern uint8_t const bit_reverse8_tbl[];
uint8_t bit_reverse8(uint8_t b8);
uint16_t bit_reverse16(uint16_t b16);
uint32_t bit_reverse32(uint32_t b32);

struct ZipPassword {
public:
    ZipPassword() : k{0x12345678, 0x23456789, 0x34567890} {}
    ZipPassword(uint32_t k0, uint32_t k1, uint32_t k2) : k{k0, k1, k2} {}
    uint8_t DecryptByte() const;
    void UpdateKey(uint8_t c);
protected:
    uint32_t k[3];
};

int CheckSize(std::vector<uint8_t> const& data, int start_offset, uint32_t sz);
uint32_t GetUint32(std::vector<uint8_t> const& data, int& start_offset);
uint16_t GetUint16(std::vector<uint8_t> const& data, int& start_offset);
uint8_t GetUint8(std::vector<uint8_t> const& data, int& start_offset);
uint32_t crc32_compute(std::vector<uint8_t> const& data);

#endif // ZIP_UTIL_H_
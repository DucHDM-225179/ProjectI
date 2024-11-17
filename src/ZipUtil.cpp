#include"ZipUtil.h"

char _zip_errmsg[1024];
int const _zip_errmsg_sz = sizeof(_zip_errmsg) / sizeof(_zip_errmsg[0]);

int CheckSize(std::vector<uint8_t> const& data, int start_offset, uint32_t sz) {
    return start_offset + sz <= data.size();
}
uint32_t GetUint32(std::vector<uint8_t> const& data, int& start_offset) {
    int u = start_offset;
    uint32_t z = 0;
    z |= data[u];
    z |= data[u+1] << 8;
    z |= data[u+2] << 16;
    z |= data[u+3] << 24;
    start_offset = u + 4;
    return z;
}
uint16_t GetUint16(std::vector<uint8_t> const& data, int& start_offset) {
    int u = start_offset;
    uint16_t z = 0;
    z |= data[u];
    z |= data[u+1] << 8;
    start_offset = u + 2;
    return z;
}
uint8_t GetUint8(std::vector<uint8_t> const& data, int& start_offset) {
    int u = start_offset;
    uint16_t z = 0;
    z |= data[u];
    start_offset = u+1;
    return z;
}
#include"ZipDeflate.h"
#include"ZipUtil.h"

#include<stdexcept>
#include<utility>

struct HuffmanTree {
    struct {
        uint16_t symbol;
        uint16_t bits;
    } fast_tbl[1 << 8];

    uint32_t limit_bit[15 + 1];
    uint16_t first_symbol[15 + 1];
    uint16_t symbols[288 + 1];

    HuffmanTree(std::vector<int>::const_iterator start, std::vector<int>::const_iterator end) {
        int freq_cnt[15 + 1] = {0};
        uint16_t first_code[15 + 1];
        uint16_t sym[15 + 1];
        for (auto it = start; it != end; ++it) {
            int x = *it;
            if (x == 0) continue; /* ignore code having len 0 */
            if (x < 0 || x > 15) {
                throw std::invalid_argument("ZipFile::ZipDeflate::Decode::HuffmanTree::Constructor: Invalid zip code word length");
            }
            ++freq_cnt[x];
        }
        for (int i = 0; i < (int)(sizeof(fast_tbl) / sizeof(fast_tbl[0])); ++i) {
            fast_tbl[i].bits = 0;
        }
        first_code[0] = 0;
        sym[0] = 0;
        for (int len = 1; len <= 15; ++len) {
            first_code[len] = (uint16_t)((first_code[len-1] + freq_cnt[len-1]) << 1);
            if (freq_cnt[len] && (first_code[len] + freq_cnt[len]) > (1 << len)) {
                throw std::invalid_argument("ZipFile::ZipDeflate::Decode::HuffmanTree::Constructor: Not enough code word for the given length frequency");
            }
            uint32_t lim = ((uint32_t)(first_code[len] + freq_cnt[len])) << (16 - len);
            limit_bit[len] = lim;
            sym[len] = sym[len-1] + freq_cnt[len-1];
            first_symbol[len] = sym[len] - first_code[len];
        }

        int i = 0;
        for (auto it = start; it != end; ++i, ++it) {
            int len = *it;
            if (len == 0) continue;
            symbols[sym[len]++] = (uint16_t)i;
            if (len <= 8) {
                uint16_t cw = first_code[len]++;
                cw = bit_reverse16(cw) >> (16-len);
                int padlen = 8 - len;
                for (uint32_t pad = 0; pad < (1U << padlen); ++pad) {
                    uint32_t idx = (pad << len) | cw;
                    fast_tbl[idx].symbol = i;
                    fast_tbl[idx].bits = len;
                }
            }
        } 
    }

    HuffmanTree(std::vector<int> const& freq) : HuffmanTree(freq.begin(), freq.end()) {}

    std::pair<uint16_t, int> Decode(uint32_t b) const {
        uint32_t bb = b & 0xFF;
        if (fast_tbl[bb].bits) {
            return std::make_pair(fast_tbl[bb].symbol, fast_tbl[bb].bits);
        }
        uint16_t b2 = b & 0xFFFF;
        b2 = bit_reverse16(b2);
        for (int len = 1+8; len <= 15; ++len) {
            if (b2 < limit_bit[len]) {
                b2 >>= 16-len;
                int idx = (uint16_t)(first_symbol[len] + b2);
                if (idx <= 288) return std::make_pair(symbols[idx], len);
                else break;
            }
        }

        throw std::invalid_argument("ZipFile::ZipDefalte::Decode::HuffmanTree::Decode: Invalid code word");
    }
};

std::vector<int> const fixed_lit_len = {
    /*   0 - 143 */8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    /* 144 - 255 */9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    /* 256 - 279 */7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    /* 280 - 287 */8, 8, 8, 8, 8, 8, 8, 8
};
std::vector<int> const fixed_dist_len = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

HuffmanTree const fixedLiteralTree(fixed_lit_len);
HuffmanTree const fixedDistanceTree(fixed_dist_len);
std::pair<int,int> const extra_bit_lit_tbl[] = {
    /* 257 - 264 */{3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0},
    /* 265 - 268 */{11, 1}, {13, 1}, {15, 1}, {17, 1},
    /* 269 - 272 */{19, 2}, {23, 2}, {27, 2}, {31, 2},
    /* 273 - 276 */{35, 3}, {43, 3}, {51, 3}, {59, 3},
    /* 277 - 280 */{67, 4}, {83, 4}, {99, 4}, {115, 4},
    /* 281 - 284 */{131, 5}, {163, 5}, {195, 5}, {227, 5},
    /*    285    */{258, 0}
};
std::pair<int,int> const extra_bit_dist_tbl[] = {
    /*   0 -   3 */{1, 0}, {2, 0}, {3, 0}, {4, 0},
    /*   4 -   7 */{5, 1}, {7, 1}, {9, 2}, {13, 2},
    /*   8 -  11 */{17, 3}, {25, 3}, {33, 4}, {49, 4},
    /*  12 -  15 */{65, 5}, {97, 5}, {129, 6}, {193, 6},
    /*  16 -  19 */{257, 7}, {385, 7}, {513, 8}, {769, 8},
    /*  20 -  23 */{1025, 9}, {1537, 9}, {2049, 10}, {3073, 10},
    /*  24 -  27 */{4097, 11}, {6145, 11}, {8193, 12}, {12289, 12},
    /*  28 -  29 */{16385, 13}, {24577, 13}
};
int bitlen_code_order_tbl[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

void Decode_NonCompressed(std::vector<uint8_t>& decode_data, ZipBitStreamInterface* bitStream) {
    bitStream->SkipToByte();
    uint32_t b = bitStream->GetBit();
    if (!bitStream->SkipBit(32)) {
        throw std::length_error("ZipFile::ZipDeflate::Decode::NonCompressedBlock: Unexpected EOF");
    }
    uint32_t len = b & 0xFFFFU;
    uint32_t nlen = b >> 16;
    if (len != (~nlen & 0xFFFFU)) {
        throw std::invalid_argument("ZipFile::ZipDeflate::Decode::NonCompressedBlock: Invalid block length");
    }
    int len2 = len;
    for (; len2 >= 4; len2 -= 4) {
        b = bitStream->GetBit();
        decode_data.emplace_back(b       & 0xFF);
        decode_data.emplace_back((b>>8 ) & 0xFF);
        decode_data.emplace_back((b>>16) & 0xFF);
        decode_data.emplace_back((b>>24) & 0xFF);
        if (!bitStream->SkipBit(32)) {
            throw std::length_error("ZipFile::ZipDeflate::Decode::NonCompressedBlock: Unexpected EOF");
        }
    }

    for (; len2 > 0; len2 -= 1) {
        b = bitStream->GetBit();
        decode_data.emplace_back(b & 0xFF);
        if (!bitStream->SkipBit(8)) {
            throw std::length_error("ZipFile::ZipDeflate::Decode::NonCompressedBlock: Unexpected EOF");
        }
    }
}
void Decode_Huffman(std::vector<uint8_t>& decode_data, ZipBitStreamInterface* bitStream, HuffmanTree const& litTree, HuffmanTree const& disTree) {
    int shouldStop = false;
    for (; !shouldStop;) {
        uint32_t b = bitStream->GetBit();
        uint16_t decoded; int bit_used;
        try {
            std::tie(decoded, bit_used) = litTree.Decode(b);
        } catch (std::exception& e) {
            throw;
        }


        if (decoded < 0 || decoded > 285) {
            throw std::invalid_argument("ZipFile::ZipDeflate::Decodee::Huffman: Unrecognized literal code");
        }
        else if (decoded == 256) { /* EOB */
            shouldStop = true;
            if (!bitStream->SkipBit(bit_used)) {
                throw std::length_error("ZipFile::ZipDeflate::Decode::Huffman: Unexpected EOF");
            }
        }
        else if (decoded < 256) {
            decode_data.emplace_back(decoded);
            if (!bitStream->SkipBit(bit_used)) {
                throw std::length_error("ZipFile::ZipDeflate::Decode::Huffman: Unexpected EOF");
            }
        }
        else {
            int baselen, extrabit;
            std::tie(baselen, extrabit) = extra_bit_lit_tbl[decoded - 257];
            b >>= bit_used;
            bit_used += extrabit;
            int len = baselen + (b & ((1U << extrabit) - 1));
            //b >>= extrabit;

            if (!bitStream->SkipBit(bit_used)) {
                throw std::length_error("ZipFile::ZipDeflate::Decode::Huffman: Unexpected EOF");
            }

            b = bitStream->GetBit();
            try {
                std::tie(decoded, bit_used) = disTree.Decode(b);
            } catch (std::exception& e) {
                throw;
            }

            if (decoded < 0 || decoded > 29) {
                throw std::invalid_argument("ZipFile::ZipDeflate::Decodee::Huffman: Unrecognized distance code");
            }
            else {
                int basedist, extrabit;
                std::tie(basedist, extrabit) = extra_bit_dist_tbl[decoded];
                b >>= bit_used;
                bit_used += extrabit;
                int dist = basedist + (b & ((1U << extrabit) - 1));
                //b >>= extrabit;

                if (!bitStream->SkipBit(bit_used)) {
                    throw std::length_error("ZipFile::ZipDeflate::Decode::Huffman: Unexpected EOF");
                }
                if (dist > (int)decode_data.size()) {
                    throw std::invalid_argument("ZipFile::ZipDeflate::Decode::dHuffman: Invalid distance");
                }

                // vector reallocate --> lost cache --> maybe slow?
                for (int offset = (int)decode_data.size() - dist, j = 0; j < len; ++offset, ++j) {
                    decode_data.emplace_back(decode_data[offset]);
                }
            }
        }
    }
}

std::vector<uint8_t> ZipDeflate::Decode() {
    std::vector<uint8_t> decode_data;
    bitStream->Reset();

    int shouldStop = false;
    for (; !shouldStop;) {
        uint32_t b = bitStream->GetBit();
        if (!bitStream->SkipBit(3)) {
            throw std::length_error("ZipFile::ZipDeflate::Decode: Unexpected EOF");
        }
        uint32_t bfinal = b & 1;
        b >>= 1;
        uint32_t block_type = b&0b11U;
        if (block_type == 0) {
            try {
                Decode_NonCompressed(decode_data, bitStream);
            }
            catch (std::exception const& e) {
                throw;
            }
        } else if (block_type == 1) {
            try {
                Decode_Huffman(decode_data, bitStream, fixedLiteralTree, fixedDistanceTree);
            }
            catch (std::exception const& e) {
                throw;
            } 
        } else if (block_type == 2) {
                /* Construct huffman */
                b = bitStream->GetBit();
                int num_lit = 257 + (b & 0b11111);
                b >>= 5;
                int num_dist = 1 + (b & 0b11111);
                b >>= 5;
                int num_len = 4 + (b & 0b1111);
                b >>= 4;
                if (!bitStream->SkipBit(5+5+4)) {
                    throw std::length_error("ZipFile::ZipDeflate::Decode::DynamicHuffman: Unexpected EOF");
                }
                std::vector<int> bitlen_len(19, 0);
                for (int i = 0; i < num_len; ++i) {
                    b = bitStream->GetBit();
                    bitlen_len[bitlen_code_order_tbl[i]] = b & 0b111;
                    if (!bitStream->SkipBit(3)) {
                        throw std::length_error("ZipFile::ZipDeflate::Decode::DynamicHuffman: Unexpected EOF");
                    }
                }
                HuffmanTree bitcodeTree(bitlen_len);

                std::vector<int> codeFreq(num_lit + num_dist, 0);
                for (int i = 0; i < num_lit + num_dist;) {
                    b = bitStream->GetBit();
                    uint16_t cw; int bitused;
                    try {
                        std::tie(cw, bitused) = bitcodeTree.Decode(b);
                    } catch (std::exception const& e) {
                        throw;
                    }
                    b >>= bitused;
                    if (cw < 16) {
                        codeFreq[i++] = cw;
                    }
                    else if (cw == 16) {
                        for (int tcopy = 3 + (b&0b11), prv = codeFreq[i-1]; tcopy; --tcopy) codeFreq[i++] = prv;
                        bitused += 2;
                    }
                    else if (cw == 17) {
                        for (int trep = 3 + (b&0b111); trep; --trep) codeFreq[i++] = 0;
                        bitused += 3;
                    }
                    else if (cw == 18) {
                        for (int trep = 11 + (b&0b1111111); trep; --trep) codeFreq[i++] = 0;
                        bitused += 7;
                    }
                    else {
                        throw std::invalid_argument("ZipFile::ZipDeflate::Decodee::DynamicHuffman: Unrecognized codelen code");
                    }
                    if (!bitStream->SkipBit(bitused)) {
                        throw std::length_error("ZipFile::ZipDeflate::Decode::DynamicHuffman: Unexpected EOF");
                    }
                }

                HuffmanTree litTree(codeFreq.begin(), codeFreq.begin() + num_lit);
                HuffmanTree disTree(codeFreq.begin() + num_lit, codeFreq.end());

                try {
                    Decode_Huffman(decode_data, bitStream, litTree, disTree);
                } catch (std::exception const& e) {
                    throw;
                }
        } else {
            throw std::invalid_argument("ZipFile::ZipDeflate::Decode: Unrecognized deflate block type");
        }

        shouldStop = bfinal;
    }


    return decode_data;
}
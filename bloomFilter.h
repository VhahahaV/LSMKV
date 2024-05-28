//
// Created by CZQ on 2024/5/23.
//

#ifndef KVLSM_BLOOMFLITER_H
#define KVLSM_BLOOMFLITER_H
#include "bitset"
#include "MurmurHash3.h"
#include <cstdint>
#include <vector>
//MAX_SIZE bits

class BloomFilter {
protected:
//    std::bitset<BLOOM_FILTER_SIZE> mContent{};
    std::vector<std::bitset<64>> mContent{};
    uint32_t mSize;
public:
    BloomFilter(): mSize(0){};
    BloomFilter(const BloomFilter&) = default;
    explicit BloomFilter(uint32_t bloomFilterSize);
    [[maybe_unused]] explicit BloomFilter(const std::vector<uint64_t> &content,uint32_t bloomFilterSize);
    [[maybe_unused, nodiscard]] bool contain(uint64_t key) const;
    [[maybe_unused]] void add(uint64_t key);

    void flush(std::vector<uint64_t> &content) const;
    void load(const std::vector<uint64_t> &content);
//    void

};


#endif //KVLSM_BLOOMFLITER_H

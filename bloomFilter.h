//
// Created by CZQ on 2024/5/23.
//

#ifndef KVLSM_BLOOMFLITER_H
#define KVLSM_BLOOMFLITER_H
#include "bitset"
#include "MurmurHash3.h"
#include <cstdint>
//MAX_SIZE bits
static constexpr int BLOOM_FILTER_SIZE = 1024 * 10 * 8;

class BloomFilter {
protected:
    std::bitset<BLOOM_FILTER_SIZE> mContent{};

public:
    explicit BloomFilter() = default;

    [[maybe_unused]] bool contain(uint64_t key);

    [[maybe_unused]] void add(uint64_t key);
//    void

};


#endif //KVLSM_BLOOMFLITER_H

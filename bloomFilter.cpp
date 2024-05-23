//
// Created by CZQ on 2024/5/23.
//

#include "bloomFilter.h"

[[maybe_unused]] void BloomFilter::add(uint64_t key) {
    uint32_t hashKey[4] = {0};
    MurmurHash3_x64_128(&key,sizeof key,1,hashKey);
    for(auto it: hashKey)
        mContent[it%BLOOM_FILTER_SIZE] = true;
}

[[maybe_unused]] bool BloomFilter::contain(uint64_t key) {
    uint32_t hashKey[4] = {0};
    MurmurHash3_x64_128(&key,sizeof key,1,hashKey);
    bool exist = true;
    for(auto it: hashKey)
        exist &= mContent[it%BLOOM_FILTER_SIZE];
    return exist;
}
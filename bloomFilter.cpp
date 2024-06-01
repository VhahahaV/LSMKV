//
// Created by CZQ on 2024/5/23.
//

#include "bloomFilter.h"

BloomFilter::BloomFilter(uint32_t bloomFilterSize):mSize(bloomFilterSize){
    mContent.resize(mSize / 8);
}

[[maybe_unused]] BloomFilter::BloomFilter(const std::vector<uint64_t> &content,uint32_t bloomFilterSize): mSize(bloomFilterSize) {
    mContent.reserve(mSize / 8);
    for(int i = 0 ; i < mContent.size(); i++){
        auto tmp = content[i];
        mContent.emplace_back(tmp);
    }
}
[[maybe_unused]] void BloomFilter::add(uint64_t key) {
    uint32_t hashKey[4] = {0};
    MurmurHash3_x64_128(&key,sizeof key,1,hashKey);
    for(auto it: hashKey){
        it %= (mSize*8);
        auto idx = it / 64;
        auto offset = it % 64;
        mContent[idx].set(offset);
    }
}

[[maybe_unused]] bool BloomFilter::contain(uint64_t key) const {
    uint32_t hashKey[4] = {0};
    MurmurHash3_x64_128(&key,sizeof key,1,hashKey);
    bool exist = true;
    for(auto it: hashKey){
        it %= (mSize*8);
        auto idx = it / 64;
        auto offset = it % 64;
        exist &= mContent[idx][offset];
    }
    return exist;
}

void BloomFilter::flush(std::vector<uint64_t> &content) const{
    for(int i = 0 ; i < mContent.size(); i++){
        content[i] = mContent[i].to_ullong();
    }
}

void BloomFilter::load(const std::vector<uint64_t> &content){
    mContent.clear();
    mContent.reserve(mSize / 8);
    for(int i = 0 ; i < mSize / 8; i++){
        auto tmp = content[i];
        mContent.emplace_back(tmp);
    }
}




//
// Created by CZQ on 2024/5/23.
//

#include "bloomFilter.h"

BloomFilter::BloomFilter(const char *content) {
    int nums = BLOOM_FILTER_SIZE/8;

    for(int i = 0 ; i < nums; i++){
        char tmp = content[i];
        int mask=1;
        int offset = i * 8;
        for(int j = 0 ;j < 8 ; j++){
            mContent[offset+j] = (tmp&mask);
            mask = mask << 1;
        }

    }
}
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

void BloomFilter::flush(char* content){
    int nums = BLOOM_FILTER_SIZE/8;

    for(int i = 0 ; i < nums; i++){
        auto &tmp = content[i];
        int offset = i * 8;
        for(int j = 0 ;j < 8 ; j++){
            if(mContent.test(offset+j)){
                tmp |= (1<<j);
            }
            else{
                tmp &= ~(1<<j);
            }
        }
    }
}



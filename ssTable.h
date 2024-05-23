//
// Created by CZQ on 2024/5/23.
//

#ifndef KVLSM_SSTABLE_H
#define KVLSM_SSTABLE_H
#include <cstdint>
#include "bloomFilter.h"
#include <vector>
#include <string>
#include "memTable.h"
uint32_t gTimeStamp = 0;
class SSTable {
protected:
//    HEADER 部分
    uint32_t mTimeStamp;
    uint32_t mNUm = 0;
    uint32_t mMin = UINT32_MAX;
    uint32_t mMax = 0;
//    BF 布隆过滤器区
    BloomFilter mBloomFilter{};
//    Index 索引区
    using indexData = std::pair<uint64_t ,uint32_t>;
    std::vector<indexData> mIndex{};
//    data 数据区 (不包含对应的 key)
    std::string mDat{};
//    size
    uint32_t mSize = 0;
public:
    explicit SSTable(const MemTable &memTable);
    void flush();
    bool reachLimit(uint32_t newSize);

};


#endif //KVLSM_SSTABLE_H
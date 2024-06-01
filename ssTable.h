//
// Created by CZQ on 2024/5/23.
//

#ifndef KVLSM_SSTABLE_H
#define KVLSM_SSTABLE_H
#include <cstdint>
#include "bloomFilter.h"
#include <vector>
#include <string>
#include <filesystem>
#include "memTable.h"
//constexpr int BYTE_SIZE = 8;
using Key_Val = std::pair<uint64_t,std::string>;

class SSTable {
    inline static uint64_t gTimeStamp = 0;
    inline static const uint32_t MAX_SIZE = 1024 * 1024 * 2;
protected:
//    HEADER 部分
    uint64_t mTimeStamp;
    uint64_t mNum = 0;
    uint64_t mMin = UINT64_MAX;
    uint64_t mMax = 0;
//    BF 布隆过滤器区
    BloomFilter mBloomFilter;
    inline static constexpr uint32_t BLOOM_FILTER_SIZE = 1024 * 10;
//    Index 索引区
    using indexData = std::pair<uint64_t ,uint32_t>;
    std::vector<indexData> mIndex{};
//    data 数据区 (不包含对应的 key)
    std::string mData{};
//    mSize
    uint32_t mSize = 0;
    std::string mPath{};
public:
    explicit SSTable(const MemTable &memTable,const std::string &path);
    explicit SSTable(const std::string &dir);
    explicit SSTable(std::vector<std::pair<uint64_t,std::string>> &vec,uint64_t timeStamp);
    [[nodiscard]] uint64_t getMin() const;
    std::string getPath() const;
    void flush() const;
    [[nodiscard]] bool reachLimit(uint32_t newSize) const ;
    bool existKey[[nodiscard]](uint64_t key) const ;
    bool crossKey[[nodiscard]](uint64_t minKey,uint64_t maxKey) const;
    [[nodiscard]] std::string get(uint64_t key) const;
    void cleanData();
    void loadVector(std::vector<Key_Val> &vec, uint64_t &maxTimeStamp) const;
    void rename(const std::string &dir);

    [[maybe_unused]] void test();
};


#endif //KVLSM_SSTABLE_H

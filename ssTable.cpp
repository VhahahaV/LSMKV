//
// Created by CZQ on 2024/5/23.
//

#include "ssTable.h"
#include <fstream>
//   将memTable转为SSTable
//https://blog.csdn.net/Rasin_Wu/article/details/79048094 应该考虑二进制读写，已达到bit的读写精度
SSTable::SSTable(const MemTable &memTable){
    mTimeStamp = gTimeStamp++;
//    skip BloomFilter and Header
    mNum = memTable.mNum;
    uint32_t offset = 10240 + 32 + (sizeof(uint32_t) +sizeof(uint64_t)) * mNum;
    auto cur = memTable.mHead;
    while(cur->down)
        cur = cur->down;
    mMin = cur->key;

    while(cur){
        mIndex.emplace_back(cur->key,offset);
        mBloomFilter.add(cur->key);
        mData+=cur->val;
//        mData += std::string(cur->val.size()%BYTE_SIZE,' ');
//        有一部分padding的空间。保证对齐
//        offset += (cur->val.size() + BYTE_SIZE - 1) / BYTE_SIZE;
//        二进制读写，不padding，直接按字符数读取
        offset += cur->val.size();
        if(!cur->right){
            mMax = cur->key;
        }
        cur = cur->right;
    }

}

//使用fstream,参考：https://blog.csdn.net/Long_xu/article/details/137073414
//https://blog.csdn.net/u011028345/article/details/76563245
SSTable::SSTable(const std::string &dir){
    std::ifstream input;
    input.open(dir);
    if(!input.good()){
        throw std::runtime_error("can not open the file");
    }
//    header
    uint64_t header[4];
    input.read((char *)header, sizeof(header));
    mTimeStamp = header[0],mNum = header[1];
    mMin = header[2],mMax = header[3];
//    BloomFilter


}

//将ssTable落入磁盘
void SSTable::flush() {

}
bool SSTable::reachLimit(uint32_t newSize){

}
//
// Created by CZQ on 2024/5/23.
//

#include "ssTable.h"
#include <fstream>
#include <iostream>
//   将memTable转为SSTable
//https://blog.csdn.net/Rasin_Wu/article/details/79048094 应该考虑二进制读写，已达到byte读写精度


SSTable::SSTable(const MemTable &memTable,const std::string &path): mBloomFilter(BLOOM_FILTER_SIZE){
    mTimeStamp = SSTable::gTimeStamp++;
//    skip BloomFilter and Header
    mNum = memTable.mNum;
    mSize = memTable.mSize;
    mPath = path;
    uint32_t offset = BLOOM_FILTER_SIZE + sizeof(uint64_t) * 4 + (sizeof(indexData)) * mNum;

    auto cur = memTable.mHead;
    while(cur->down)
        cur = cur->down;
    cur = cur->right;
    mMin = cur->key;

    while(cur){
        mIndex.emplace_back(cur->key,offset);
        mBloomFilter.add(cur->key);
        mData+=cur->val;
        offset += cur->val.size();
        if(!cur->right){
            mMax = cur->key;
        }
        cur = cur->right;
    }
}

//使用fstream,参考：https://blog.csdn.net/Long_xu/article/details/137073414
//https://blog.csdn.net/u011028345/article/details/76563245
SSTable::SSTable(const std::string &dir): mBloomFilter(BLOOM_FILTER_SIZE){
    mPath = dir;
    std::ifstream input;
    input.open(dir,std::ios::binary);
    if(!input.good()){
        std::cout << "when init SSTable dir :" << dir;
        throw std::runtime_error("can not open the file");
    }
//    header
    std::vector<uint64_t> header(4);
    input.read(reinterpret_cast<char *>(header.data()), sizeof(uint64_t)*4);
    mTimeStamp = header[0],mNum = header[1];
    mMin = header[2],mMax = header[3];
//    BloomFilter
    std::vector<uint64_t> bfContent(BLOOM_FILTER_SIZE/8);
    input.read(reinterpret_cast<char *>(bfContent.data()), BLOOM_FILTER_SIZE);
    mBloomFilter.load(bfContent);
//    indexes
    uint32_t indexSize= mNum * sizeof(indexData);
    input.read(reinterpret_cast<char *>(mIndex.data()),indexSize);

// ignore data
    input.close();
}

//将ssTable落入磁盘
void SSTable::flush(const std::string &dir) const{
    std::ofstream output;
    output.open(dir,std::ios::binary);
    if(!output.good()){
        std::cout << "when flush dir :" << dir;
        throw std::runtime_error("can not open the file");
    }
    //    header
    std::vector<uint64_t> header(4);
    header[0] = mTimeStamp,header[1] = mNum;
    header[2] = mMin,header[3] = mMax;
    output.write(reinterpret_cast<char *>(header.data()), 4* sizeof(uint64_t));
//    BloomFilter
    std::vector<uint64_t> bfContent(BLOOM_FILTER_SIZE/8);
    mBloomFilter.flush(bfContent);
    output.write(reinterpret_cast<char *>(bfContent.data()),BLOOM_FILTER_SIZE);
//    indexes
    uint32_t indexSize= mNum * (sizeof(indexData));
    output.write(reinterpret_cast<const char *>(mIndex.data()), indexSize);
//  data
    int dataSize = (int)mData.size();
    output.write(mData.data(),dataSize);
    output.close();

}
bool SSTable::reachLimit(uint32_t newSize) const {

}

bool SSTable::existKey(uint64_t key) const {
    if(key< mMin || key > mMax)
        return false;
    return mBloomFilter.contain(key);
}

std::string SSTable::get(uint64_t key) const{
    auto index = std::lower_bound(mIndex.begin(),mIndex.end(),indexData(key,0));
    if(index == mIndex.end() || index->first != key)
        return {};
    auto offset = index->second;
    uint32_t len = (*index == mIndex.back()) ? mSize - offset :(index+1)->second - offset ;
    std::ifstream input(mPath,std::ios::binary);
    if(!input.good()){
        std::cout << "when get dir :" << mPath;
        throw std::runtime_error("can not open the file");
    }
    input.seekg(offset);
    std::string val(len,' ');
    input.read(val.data(),len);
    input.close();
    return val;
}

void SSTable::cleanData() {
    mData.clear();
}

void SSTable::test() {
    std::cout <<"SStable info : "<<  mTimeStamp << " " << mNum << " " << mMin << " " << mMax << std::endl;
    std::cout << "Index nums : " << mIndex.size() << " " << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << mIndex[i].first << "  " << mIndex[i].second << std::endl;
    }
}
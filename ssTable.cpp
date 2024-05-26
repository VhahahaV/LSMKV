//
// Created by CZQ on 2024/5/23.
//

#include "ssTable.h"
#include <fstream>
#include <iostream>
//   将memTable转为SSTable
//https://blog.csdn.net/Rasin_Wu/article/details/79048094 应该考虑二进制读写，已达到byte读写精度
SSTable::SSTable(const MemTable &memTable){
    mTimeStamp = SSTable::gTimeStamp++;
//    skip BloomFilter and Header
    mNum = memTable.mNum;
    uint32_t offset = 10240 + 32 + (sizeof(uint32_t) +sizeof(uint64_t)) * mNum;
    auto cur = memTable.mHead;
    while(cur->down)
        cur = cur->down;
    cur = cur->right;
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
        std::cout << "when init SSTable dir :" << dir;
        throw std::runtime_error("can not open the file");
    }
//    header
    uint64_t header[4];
    input.read((char *)header, sizeof(header));
    mTimeStamp = header[0],mNum = header[1];
    mMin = header[2],mMax = header[3];
//    BloomFilter
    char bfContent[10240];
    input.read(bfContent,sizeof(bfContent));
    mBloomFilter = BloomFilter(bfContent);
//    indexes
    uint32_t indexSize= mNum * sizeof(indexData);
    std::vector<char> indexBuffer(indexSize);
    input.read(indexBuffer.data(),indexSize);
    auto indexContent = reinterpret_cast<indexData *>(indexBuffer.data());
    for(int i = 0 ; i < mNum ; i++){
        mIndex.emplace_back(indexContent[i]);
    }
// ignore data
    input.close();
}

//将ssTable落入磁盘
void SSTable::flush(const std::string &dir) {
    std::ofstream output;
    output.open(dir);
    if(!output.good()){
        std::cout << "when flush dir :" << dir;
        throw std::runtime_error("can not open the file");
    }
    //    header
    uint64_t header[4];
    header[0] = mTimeStamp,header[1] = mNum;
    header[2] = mMin,header[3] = mMax;
    output.write((char *)header, sizeof(header));

//    BloomFilter
    char bfContent[10240];
    mBloomFilter.flush(bfContent);
    output.write(bfContent,sizeof(bfContent));
//    indexes
    uint32_t indexSize= mNum * sizeof(indexData);
    std::vector<char> indexBuffer(indexSize);
    auto indexPtr = indexBuffer.data();
    for (int i = 0; i < mNum; ++i) {
        memcpy(indexPtr, &mIndex[i], sizeof(indexData));
        indexPtr += sizeof(indexData);
    }
    // 将整个缓冲区写入文件
    output.write(indexBuffer.data(), indexSize);
//    auto *indexContent = new indexData [mNum];
//    for(int i = 0 ; i < mNum ; i++){
//        indexContent[i] = mIndex[i];
//    }
//    output.write((char *)indexContent,(int)(sizeof(uint32_t) +sizeof(uint64_t)) * mNum );


//  data
    const char *data = mData.c_str();
    output.write(data,sizeof(data));

    output.close();

}
bool SSTable::reachLimit(uint32_t newSize){

}

bool SSTable::existKey(uint64_t key) {
    if(key< mMin || key > mMax)
        return false;
    return mBloomFilter.contain(key);
}

std::string SSTable::get(const std::string &dir,uint64_t key){
    auto index = std::lower_bound(mIndex.begin(),mIndex.end(),indexData(key,0));
    if(index == mIndex.end() || index->first != key)
        return {};
    auto offset = index->second;
    auto len = (index+1)->second - offset;
    std::ifstream input(dir);
    if(!input.good()){
        std::cout << "when get dir :" << dir;
        throw std::runtime_error("can not open the file");
    }
    input.seekg(offset);
    char *buffer = new char [len];
    input.read(buffer,len);
    input.close();
    std::string val(buffer);
    return val;
}

void SSTable::test() {
    std::cout <<"SStable info : "<<  mTimeStamp << " " << mNum << " " << mMin << " " << mMax << std::endl;
    std::cout << "Index nums : " << mIndex.size() << " " << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << mIndex[i].first << "  " << mIndex[i].second << std::endl;
    }
}
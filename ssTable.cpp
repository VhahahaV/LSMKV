//
// Created by CZQ on 2024/5/23.
//

#include "ssTable.h"
#include <fstream>
#include <iostream>
//   将memTable转为SSTable
//https://blog.csdn.net/Rasin_Wu/article/details/79048094 应该考虑二进制读写，已达到byte读写精度



SSTable::SSTable(const MemTable &memTable,const std::string &path): mBloomFilter(BLOOM_FILTER_SIZE){
    mData.reserve(MAX_SIZE);
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
    mSize = offset;
    if(offset != mSize){
        std::cout << "offset = " << offset << "while mSize = " << mSize << std::endl;
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

    mIndex.reserve(mNum);
    std::vector<char> indexBuffer(indexSize);
    input.read(indexBuffer.data(),indexSize);
    auto indexes = reinterpret_cast<indexData*>(indexBuffer.data());
    // 从缓冲区复制数据到mIndex
    for (size_t i = 0; i < mNum; ++i) {
        mIndex.emplace_back(indexes[i]);
    }

//    这句话用不了
//    input.read(reinterpret_cast<char *>(mIndex.data()),indexSize);
    mSize = std::filesystem::file_size(mPath);

// ignore data
    input.close();
}
uint64_t SSTable::getMin() const{
    return mMin;
}
std::string SSTable::getPath() const{
    return mPath;
}


SSTable::SSTable(std::vector<std::pair<uint64_t,std::string>> &vec,uint64_t timeStamp): mBloomFilter(BLOOM_FILTER_SIZE){
//    mPath 需要调整
//    header
    mData.reserve(MAX_SIZE);
    mTimeStamp = timeStamp;
    mSize = 4 * sizeof(uint64_t) + BLOOM_FILTER_SIZE;
    uint32_t offset  = mSize;

    while (!vec.empty()){
//        if(vec.size()==1 && timeStamp==53 && vec.front().second[0] == 's'){
//            int a=1;
//            std::cout << vec.front().second << std::endl;
//        }

        uint32_t newSize = sizeof(indexData) + vec.front().second.size();
        if(reachLimit((newSize)))
            break;
        else
            mSize+=newSize;

        auto [key,val] = vec.front();
        vec.erase(vec.begin());
        mNum++;
        mBloomFilter.add(key);

        mData+=val;
        mMax = std::max<uint64_t>(mMax,key);
        mMin = std::min<uint64_t>(mMin,key);

        mIndex.emplace_back(key,offset);
        offset += val.size();

    }
//    更新mIndex 中的offset
    for(auto &[key,off] : mIndex)
        off += sizeof(indexData)*mNum;
}


//将ssTable落入磁盘
void SSTable::flush() const{
//    std::cout <<"flush path : " << mPath << std::endl;

    std::ofstream output;
    output.open(mPath,std::ios::binary);
    if(!output.good()){
        std::cout << "when flush dir :" << mPath;
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
    return mSize + newSize > MAX_SIZE;
}

bool SSTable::existKey(uint64_t key) const {
    if(key< mMin || key > mMax)
        return false;
    return mBloomFilter.contain(key);
}
bool SSTable::crossKey[[nodiscard]](uint64_t minKey,uint64_t maxKey) const{
    return  std::max(minKey, mMin) <= std::min(maxKey, mMax);
//    return (minKey <= mMax && maxKey > mMin) || (maxKey >= mMin && minKey < mMax);
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

void SSTable::loadVector(std::vector<Key_Val> &vec, uint64_t &maxTimeStamp) const{
    maxTimeStamp = std::max<uint64_t>(maxTimeStamp,mTimeStamp);
    std::ifstream input(mPath,std::ios::binary);
    if(!input.good()){
        std::cout << "when get dir :" << mPath;
        throw std::runtime_error("can not open the file");
    }
    for(auto index = mIndex.begin() ; index != mIndex.end() ; index++){
        auto offset = index->second;
        uint32_t len = (*index == mIndex.back()) ? mSize - offset :(index+1)->second - offset ;
        if(index == mIndex.begin())
            input.seekg(offset);
        std::string val(len,' ');
        input.read(val.data(),len);
        vec.emplace_back(index->first,val);
    }
}

void SSTable::rename(const std::string &dir){
    if(!mPath.empty()){
        std::filesystem::rename(mPath.c_str(),dir.c_str());
        if(!std::filesystem::exists(dir)){
            std::cout << "rename fails :" << dir << std::endl;
        }
//        else{
//            std::cout << "rename success from " << mPath << " to  "<<dir << std::endl;
//        }
    }

    mPath = dir;

}


//
// Created by VhahahaV on 2024/5/28.
//

#include "level.h"
#include <iostream>
#include <utility>

Level::Level(uint32_t rank, std::string dirPath) : mRank(rank), mDirPath(std::move(dirPath)){
    if(!utils::dirExists(mDirPath)){
        if(utils::mkdir(mDirPath.c_str()))
            throw std::runtime_error("can't mkdir");
    }
//    为可能多出来的ssTable也分配空间
    mSSTableCache.reserve((1<<(mRank+1))+(1<<mRank));
    if (!rank) mMode = MODE::Tiering;
    else mMode = MODE::Leveling;
}
void Level::addSSTable(const MemTable &memTable){
//    判断ssTable数量是否超过
    std::string filePath = mDirPath+"/"+std::to_string(mSSTableCache.size()+1)+".sst";
    SSTable ssTable(memTable,filePath);
    ssTable.flush();
    ssTable.cleanData();
    mSSTableCache.emplace_back(std::move(ssTable));
}
void Level::addSSTable(const SSTable &ssTable){
    mSSTableCache.emplace_back(ssTable);
}

std::string Level::get(uint64_t key){
    int totalNum = (int)mSSTableCache.size();
    if(!totalNum) return {};
    for(int i = 0,order = 1; i < totalNum ; i++,order++){
//        从最新的ssTable开始找
        auto &ssTable = mSSTableCache[totalNum-i-1];
        if(ssTable.existKey(key)){
            std::string val = ssTable.get(key);
            if(!val.empty()){
                return val;
            }
        }
    }
    return {};
}

bool Level::exceedLimit() const{
    return mSSTableCache.size() > 1<<(mRank+1);
}
void Level::compact(Level &nextLevel){
    std::vector<Key_Val> curVec,NextVec;
    uint64_t maxTimeStamp = 0;
//   本层选取
    if(mMode == MODE::Tiering){
//        for(auto &ssTable : mSSTableCache){
//            ssTable.loadVector(vec,maxTimeStamp);
//            utils::rmfile(ssTable.getPath().c_str());
//        }
        while(!mSSTableCache.empty()){
            auto &ssTable = mSSTableCache.back();
            ssTable.loadVector(curVec,maxTimeStamp);
            utils::rmfile(ssTable.getPath().c_str());
            mSSTableCache.pop_back();
        }
//        mSSTableCache.clear();
    }
    else{
        while (exceedLimit()){
            mSSTableCache.back().loadVector(curVec,maxTimeStamp);
            utils::rmfile(mSSTableCache.back().getPath().c_str());
            mSSTableCache.pop_back();
        }
    }
    std::stable_sort(curVec.begin(), curVec.end());
//    auto u = std::unique(vec.begin(),vec.end());
    for(auto &[k,v]:curVec){
        if(k == 2498)
            std::cout << "find 2498 : " << v <<std::endl;
    }
    uint64_t minKey = curVec.front().first, maxKey = curVec.back().first;
    if(minKey == 0 && maxKey == 12287)
        int a = 1;
//    下一层选取,nextVec必定是有序的
    nextLevel.nextSelect(minKey,maxKey,NextVec,maxTimeStamp);

//    归并排序,而且是stable过程，第一个范围中的元素（保留其原始顺序）先于第二个范围中的元素（保留其原始顺序）。
//    std::stable_sort(vec.begin(),vec.end());
    std::vector<Key_Val> resVec;
    {
        size_t i = 0 ,j = 0;
        size_t curNum = curVec.size();
        size_t nextNum = NextVec.size();
        while(i < curNum && j < nextNum){
            if(curVec[i].first < NextVec[j].first){
                resVec.emplace_back(curVec[i++]);
            }
            else if(curVec[i].first > NextVec[j].first){
                resVec.emplace_back(NextVec[j++]);
            }
            else{
                resVec.emplace_back(curVec[i++]);
                resVec.emplace_back(NextVec[j++]);
            }
        }
        while (i<curNum) resVec.emplace_back(curVec[i++]);
        while (j<nextNum) resVec.emplace_back(NextVec[j++]);
    }
    for(auto &[k,v]:resVec){
        if(k == 2498)
            std::cout << "find 2498 : " << v <<std::endl;
    }
//    去重，保留时间戳更大的key，默认保留第一个重复的key
//    auto it = std::unique(vec.begin(),vec.end());
    auto it = (resVec.begin()+1);
    uint64_t lastKey = resVec.begin()->first;
    while(it != resVec.end()){
        if(it->first == lastKey){
            if(lastKey == 2498)
                int a=1;
            it = resVec.erase(it);

        }
        else{
            lastKey = it->first;
            if(lastKey == 2498)
                int a=1;
            it++;
        }
    }

//    将vec中的内容merge到下一层
    nextLevel.nextMerge(resVec,maxTimeStamp);
}

void Level::nextSelect(uint64_t minKey, uint64_t maxKey, std::vector<Key_Val> &vec, uint64_t &maxTimeStamp){
    if(mMode == MODE::Tiering || mSSTableCache.empty())
        return ;

    auto it = mSSTableCache.begin();
    while(it != mSSTableCache.end()){
        if(it->crossKey(minKey,maxKey)){
            it->loadVector(vec,maxTimeStamp);
            it = mSSTableCache.erase(it);
        }
        else
            it++;
    }
}

void Level::nextMerge(std::vector<Key_Val> &vec, const uint64_t &maxTimeStamp){

    int newFileNum = 0;
    while (!vec.empty()){
        SSTable ssTable(vec, maxTimeStamp);
        mSSTableCache.emplace_back(std::move(ssTable));
        newFileNum++;
    }

    std::sort(mSSTableCache.begin(), mSSTableCache.end(),
              [](SSTable &a , SSTable &b)->bool {return a.getMin() < b.getMin();});

//    update filepath
    int unchangedFileNum = 0;
    while(!mSSTableCache[unchangedFileNum].getPath().empty())
        unchangedFileNum++;

//    需要从后往前
//    for(int i = unchangedFileNum + newFileNum; i < mSSTableCache.size() ; i++){
    for(int i = (int)mSSTableCache.size()-1; i >= unchangedFileNum + newFileNum ; i--){

        std::string filePath = mDirPath+"/"+std::to_string(i+1)+".sst";
        if(filePath== "./data/level-2/6.sst")
            int a = 1;
        mSSTableCache[i].rename(filePath);
    }
//    flush 内存中的data（有一部分ssTable的mData不为空，是在emplace_back时创建的
//    for(int i = unchangedFileNum;i < unchangedFileNum + newFileNum ; i++){
    for(int i = unchangedFileNum + newFileNum - 1;i >= unchangedFileNum  ; i--){

        auto &ssTable = mSSTableCache[i];
        std::string filePath = mDirPath+"/"+std::to_string(i+1)+".sst";
        ssTable.rename(filePath);
        ssTable.flush();
        ssTable.cleanData();
    }

}



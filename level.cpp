//
// Created by VhahahaV on 2024/5/28.
//

#include "level.h"

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
                if(val == "~DELETED~")
                    return {};
                else
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
    std::vector<std::pair<uint64_t,std::string>> vec;
    uint64_t maxTimeStamp = 0;
//   本层选取
    if(mMode == MODE::Tiering){
        for(auto &ssTable : mSSTableCache){
            ssTable.loadVector(vec,maxTimeStamp);
            utils::rmfile(ssTable.getPath().c_str());
        }
        mSSTableCache.clear();
        std::sort(vec.begin(), vec.end());
    }
    else{
        while (exceedLimit()){
            mSSTableCache.back().loadVector(vec,maxTimeStamp);
            utils::rmfile(mSSTableCache.back().getPath().c_str());
            mSSTableCache.pop_back();
        }
    }
    uint32_t curNum = vec.size();
    uint64_t minKey = vec.front().first, maxKey = vec.back().first;

//    下一层选取,nextVec必定是有序的
    nextLevel.nextSelect(minKey,maxKey,vec,maxTimeStamp);

//    归并排序
    std::inplace_merge(vec.begin(),vec.begin()+curNum,vec.end());

//    将vec中的内容merge到下一层
    nextLevel.nextMerge(vec,maxTimeStamp);
}

void Level::nextSelect(uint64_t minKey, uint64_t maxKey, std::vector<std::pair<uint64_t,std::string>> &vec, uint64_t &maxTimeStamp){
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

void Level::nextMerge(std::vector<std::pair<uint64_t,std::string>> &vec, const uint64_t &maxTimeStamp){
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
    for(int i = unchangedFileNum + newFileNum; i < mSSTableCache.size() ; i++){
        std::string filePath = mDirPath+"/"+std::to_string(i+1)+".sst";
        mSSTableCache[i].rename(filePath);
    }
//    flush 内存中的data（有一部分ssTable的mData不为空，是在emplace_back时创建的
    for(int i = unchangedFileNum;i < unchangedFileNum + newFileNum ; i++){
        auto &ssTable = mSSTableCache[i];
        std::string filePath = mDirPath+"/"+std::to_string(i+1)+".sst";
        ssTable.rename(filePath);
        ssTable.flush();
        ssTable.cleanData();
    }

}



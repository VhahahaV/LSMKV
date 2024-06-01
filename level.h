//
// Created by VhahahaV on 2024/5/28.
//

#ifndef KVLSM_LEVEL_H
#define KVLSM_LEVEL_H

#include "ssTable.h"
#include <vector>
#include <algorithm>
#include "utils.h"
enum class MODE{
    Tiering,
    Leveling
};
class Level {
protected:
    std::vector<SSTable> mSSTableCache{};
    const uint32_t mRank;
    const std::string mDirPath;
    MODE mMode;
public:
    explicit Level(uint32_t rank, std::string dirPath);
    void addSSTable(const MemTable &memTable);
    void addSSTable(const SSTable &ssTable);
    
    [[nodiscard]] std::string get(uint64_t key);
    [[nodiscard]] bool exceedLimit() const;
    void compact(Level &next);
//    合并时下一层选取的ssTable
    void nextSelect(uint64_t minKey, uint64_t maxKey,std::vector<Key_Val> &vec,uint64_t &maxTimeStamp);
//     将选取的kv合并到下一层
    void nextMerge(std::vector<Key_Val > &vec,const uint64_t &maxTimeStamp);
};


#endif //KVLSM_LEVEL_H

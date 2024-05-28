#include "kvstore.h"

KVStore::KVStore(const std::string &dir): KVStoreAPI(dir),mDir(dir)
{
//    持久话，persistence，需要从文件夹中load所有数据
    mLevel.reserve(100);
    std::vector<std::string > subdir;
    utils::scanDir(dir,subdir);
    for(auto &sub:subdir){
        auto subFiles = dir+"/"+sub;
        std::vector<std::string > subvec;
        utils::scanDir(subFiles,subvec);
        std::cout << "rm dir : " << subFiles << std::endl;
        for(auto &f:subvec)
            utils::rmfile((subFiles+"/"+f).c_str());
        utils::rmdir(subFiles.c_str());
    }
}

KVStore::~KVStore()
{
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */

void KVStore::put(uint64_t key, const std::string &s)
{
    if(!mMemTable.put(key, s)){
//        need load to ssTable and flush
        auto createLevel = [&](){
            std::string curDir = mDir+"/level-"+std::to_string(mLevelNum);
            mLevel.emplace_back(mLevelNum++,curDir);
        };
        if(mLevel.empty()){
            createLevel();
        }
        auto curLevel = mLevel.begin();
        curLevel->addSSTable(mMemTable);
        while(curLevel->exceedLimit()){
            if(curLevel+1 == mLevel.end()){
//                create next level
                createLevel();
            }
//            compaction with next level
            curLevel->compact(*(curLevel+1));
            curLevel++;
        }
        mMemTable.reset();
        mMemTable.put(key, s);
    }
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
    auto res = mMemTable.get(key);
	if(res.empty()){
        for(auto &level : mLevel){
            res = level.get(key);
            if(!res.empty())
                break;
        }
    }
    if(res == "~DELETED~")
        return {};
    return res;

}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
    if(get(key).empty())
        return false;
	if(!mMemTable.del(key)){
        put(key,"~DELETED~");
    }
    return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
    mMemTable.reset();
    mLevel.clear();
    mLevelNum = 0;
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list)
{
    mMemTable.scan(key1, key2, list);
}

void KVStore::test() {


}

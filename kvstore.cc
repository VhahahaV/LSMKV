#include "kvstore.h"

KVStore::KVStore(const std::string &dir): KVStoreAPI(dir),mDir(dir)
{

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
        SSTable ssTable(mMemTable);
        std::string curDir = mDir+"/level-"+std::to_string(mLevelNum);
        if(!utils::dirExists(curDir)){
            if(utils::mkdir(curDir.c_str()))
                throw std::runtime_error("can't mkdir");
        }
        std::vector<std::string> subFiles;
        int filesNum = utils::scanDir(curDir,subFiles);
//        if(filesNum < (1<<mLevelNum)){
//            std::cout << "prepare mk file : " << filesNum+1 << "when key = " << key << std::endl;
            ssTable.flush(curDir+"/"+std::to_string(filesNum+1)+".sst");
            mSSTableCache.emplace_back(ssTable);

//            test flush , index's flush fails
            SSTable test(curDir+"/"+std::to_string(filesNum+1)+".sst");
            test.test();
//        }
//        else{
//            need compaction
//        }

        mMemTable.reset();
    }
    mMemTable.put(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
    auto res = mMemTable.get(key);
	if(res.empty()){
        int curLevel = 0;
//        暂时这是为最大值，因为还没有分层
        int numBound = INT32_MAX;
        int totalNum = (int)mSSTableCache.size();
        if(!totalNum) return {};
        for(int i = 0,order = 1; i < totalNum ; i++,order++){
//            计算在cache中的ssTable的level和次序
            if(i > numBound){
                curLevel++;
                numBound += (1 << (curLevel + 1));
                order=1;
            }
            auto &ssTable = mSSTableCache[i];
            if(ssTable.existKey(key)){
                std::string filePath = mDir + "/level-" + std::to_string(curLevel)
                        + "/" + std::to_string(order) + ".sst";
                std::string val = ssTable.get(filePath,key);
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
    mSSTableCache.clear();
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

    for(auto &t : mSSTableCache){
        t.test();
    }
}

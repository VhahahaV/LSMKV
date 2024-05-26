#pragma once

#include "kvstore_api.h"
#include "memTable.h"
#include "ssTable.h"
#include "utils.h"
#include <optional>
#include <string>
#include <memory>
class KVStore : public KVStoreAPI {
	// You can add your implementation here
private:
    MemTable mMemTable{};
    std::vector<SSTable> mSSTableCache{};

    std::string mDir;
    int mLevelNum = 0;

public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list) override;

};

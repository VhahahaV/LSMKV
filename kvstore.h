#pragma once
#include "kvstore_api.h"
#include "memTable.h"
#include "ssTable.h"
#include "level.h"
#include "utils.h"
#include <optional>
#include <string>
#include <iostream>
#include <memory>
class KVStore : public KVStoreAPI {
	// You can add your implementation here
private:
    MemTable mMemTable{};

    std::vector<Level> mLevel{};

    std::string mDir{};
    uint32_t mLevelNum = 0;

public:
	explicit KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list) override;

    void flush();

};

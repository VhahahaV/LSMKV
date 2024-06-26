//
// Created by VhahahaV on 2024/5/23.
//

#ifndef LSMKV_MEMTABLE_H
#define LSMKV_MEMTABLE_H
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <queue>
#include <list>
#include <random>
//memtable 的 mSize 有问题
class   MemTable {
    friend class SSTable;
    struct Node{
        uint64_t key;
        std::string val;
        Node *down;
        Node *right;
        Node(uint64_t k,std::string v, Node *r = nullptr,Node *d = nullptr)
                :key(k),val(std::move(v)),right(r),down(d){}
    };
protected:
    inline static std::random_device gRandomDevice{};
    std::uniform_int_distribution<int> mRandomDistribution{false, true};
    Node *mHead = nullptr;
    uint32_t mNum = 0;
    uint64_t mSize = 32 + 10240;
    std::vector<Node *> mSkipList{};
    inline static constexpr int MAX_LEVEL = 32;
    inline static constexpr int MAX_SIZE = 1024*1024*2;

public:
    explicit MemTable(){
        mSkipList.reserve(MAX_LEVEL);
    }
    std::string get[[maybe_unused, nodiscard]](uint64_t k);
    bool put(uint64_t k ,const std::string &v);
    bool del(uint64_t k);
    bool empty () const;
    void reset();
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list);
    ~MemTable();// todo:


};

#endif //LSMKV_MEMTABLE_H

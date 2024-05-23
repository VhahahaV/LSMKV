//
// Created by VhahahaV on 2024/5/23.
//

#ifndef LSMKV_MEMTABLE_H
#define LSMKV_MEMTABLE_H
#include <stdint.h>
#include <string>
#include <vector>
#include <queue>
#include <list>
struct Node{
    uint64_t key;
    std::string val;
    Node *down;
    Node *right;
    Node(uint64_t k,std::string v,Node *r = nullptr,Node *d = nullptr)
    :key(k),val(v),right(r),down(d){}
};

class MemTable {
    Node *head = nullptr;
    uint64_t size = 0;
    std::vector<Node *> skipList{};
    const static int MAX_LEVEL = 32;
    //   1024 * 1024 * 2 byte
    const int MAX_SIZE = 1024*1024*2;
    explicit memTable(){
        skipList.reserve(MAX_LEVEL);
    }
public:
    std::string get(uint64_t k);
    void put(uint64_t k , std::string  v);
    bool del(uint64_t k);
    void reset();
    void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list);



};


#endif //LSMKV_MEMTABLE_H

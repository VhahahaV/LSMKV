//
// Created by VhahahaV on 2024/5/23.
//

#include "memTable.h"

std::string MemTable::get[[maybe_unused, nodiscard]](uint64_t k){
    Node *cur = mHead;
    while(cur){
        while(cur->right && cur->right->key < k){
            cur = cur->right;
        }
        if(cur->right && cur->right->key == k){
            return cur->right->val;
        }
        cur = cur->down;
    }
    return {};
}

bool MemTable::put(uint64_t k, const std::string &v){
    uint64_t newSize = sizeof(std::pair<uint64_t ,uint32_t >) + v.size();
    if (mSize + newSize >= MAX_SIZE)
        return false;
    mSize+=newSize;
    mSkipList.clear();
    Node *cur = mHead;
//    插入还是覆盖
    bool cover = false;
    while(cur){
        while(cur->right && cur->right->key < k)
            cur = cur->right;
        if(cur->right && cur->right->key == k){
            cover = true;
        }
        mSkipList.emplace_back(cur);
        cur = cur->down;
    }
//    如果出现相同的值，则选择覆盖
    if(cover){
        while(!mSkipList.empty()){
            auto curPos = mSkipList.back();
            if(curPos->right->key != k)
                break;
            mSkipList.pop_back();
            if(curPos->right)
                curPos->right->val = v;
        }
    }
    else{
        bool insert = true;
        Node *downNode = nullptr;
        while (insert && !mSkipList.empty()) {
            auto curPos = mSkipList.back();
            mSkipList.pop_back();
            curPos->right = new Node(k, v, curPos->right, downNode);
            downNode = curPos->right;
            insert = mRandomDistribution(gRandomDevice);
//        insert = false;
        }
//        新加一层
        if (insert) {
            mHead = new Node(0, "", new Node(k, v, nullptr, downNode), mHead);
        }
//        增加mNum的数量
        mNum++;
    }
    return true;
}

bool MemTable::del(uint64_t k){
    auto cur = mHead;
    bool success = false;
    uint64_t valSize;
    while(cur){
        while (cur->right && cur->right->key < k)
            cur = cur->right;
        if(cur->right && cur->right->key == k && cur->right->val != "~DELETED~"){
            success = true;
            auto rmNode = cur->right;
            cur->right = cur->right->right;
            cur = cur->down;
            valSize = rmNode->val.size();
            delete rmNode;
        }
        else{
            cur = cur->down;
        }
    }
    while (mHead && !mHead->right){
        auto rmHead = mHead;
        mHead = mHead->down;
        delete rmHead;
    }
    if(success){
        mNum--;
        uint64_t newSize = sizeof(std::pair<uint64_t ,uint32_t >) + valSize;
        mSize -=  newSize;
    }
    return success;
}
bool MemTable::empty() const{
    return mHead == nullptr;
}

void MemTable::reset(){
    mSkipList.clear();
    auto cur = mHead;
    while (cur){
        auto tmp = cur;
        cur = cur->down;
        while (tmp != nullptr){
            auto next = tmp->right;
            delete tmp;
            tmp = next;
        }
    }
    mHead = nullptr;
    mNum = 0;
    mSize = 32 + 10240;
}

MemTable::~MemTable(){
    this->reset();
}

void MemTable::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) {
    Node *start = mHead , *end = mHead;
    while(true){
//        start 小于 key1
        while(start->right && start->right->key < key1)
            start = start->right;
        if(start->down)
            start = start->down;
        else break;
    }
    while(true){
//        end小于等于key2
        while(end->right && end->right->key <= key2)
            end = end->right;
        if(end->down)
            end = end->down;
        else break;
    }
//    item大于等于key1
    auto item = start->right;
//    [key1 , key2]
    while (item != end->right){
        list.emplace_back(item->key,item->val);
        item = item->right;
    }

}
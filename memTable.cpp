//
// Created by VhahahaV on 2024/5/23.
//

#include "memTable.h"

std::string MemTable::get(uint64_t k){
    Node *cur = head;
    while(cur){
        while(cur->right && cur->right->key < k){
            cur = cur->right;
        }
        if(cur->right && cur->right->key == k){
            return cur->right->val;
        }
        cur = cur->down;
    }
    return nullptr;
}

void MemTable::put(uint64_t k , std::string  v){
    Node *cur = head;
    while(cur){
        while(cur->right && cur->right->key < k)
            cur = cur->right;
        skipList.emplace_back(cur);
        cur = cur->down;
    }

    bool insert = true;
    Node *downNode = nullptr;
    while(insert && !skipList.empty()){
        auto curPos = skipList.back();
        skipList.pop_back();
        curPos->right = new Node(k,v,curPos->right,downNode);
        downNode = curPos->right;
        insert = (rand() & 1);
    }
//        新加一层
    if(insert){
        head = new Node(0,"",new Node(k,v,nullptr,downNode),head);
    }
}

bool MemTable::del(uint64_t k){
    Node *cur = head;
    bool success = false;
    while(cur){
        while (cur->right && cur->right->key < k)
            cur = cur->right;
        if(cur->right && cur->right->key == k){
            success = true;
            auto rmNode = cur->right;
            cur->right = cur->right->right;
            delete rmNode;
            cur = cur->down;
        }
        else{
            cur = cur->down;
        }
    }
    return success;
}

void MemTable::reset(){
    skipList.clear();
    auto cur = head;
    while (cur){
        auto tmp = cur;
        cur = cur->down;
        while (tmp){
            auto next = tmp->right;
            delete tmp;
            tmp = next;
        }
    }
}

void MemTable::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) {
    Node *start = head , *end = head;
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
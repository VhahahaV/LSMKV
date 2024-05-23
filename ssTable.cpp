//
// Created by CZQ on 2024/5/23.
//

#include "ssTable.h"
SSTable::SSTable(const MemTable &memTable){
//   将memTable转为SSTable
    mTimeStamp = gTimeStamp++;
    uint32_t offset = 10240 + 32;


}
void SSTable::flush() {

}
bool SSTable::reachLimit(uint32_t newSize){

}
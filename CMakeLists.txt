cmake_minimum_required(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 17)
PROJECT(KVLSM)
ADD_LIBRARY(LibKVLSM
        kvstore.cc kvstore.h kvstore_api.h memTable.cpp memTable.h bloomFilter.cpp bloomFilter.h ssTable.cpp ssTable.h
)
ADD_EXECUTABLE(correctness correctness.cc)
ADD_EXECUTABLE(persistence persistence.cc)

target_link_libraries(correctness PUBLIC LibKVLSM)
target_link_libraries(persistence PUBLIC LibKVLSM)


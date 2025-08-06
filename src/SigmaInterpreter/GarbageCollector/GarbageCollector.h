#pragma once
#include "../RunTime.h"
#include <boost/asio/post.hpp>
#include <memory_resource>
#include <boost/asio/thread_pool.hpp>
#include <ostream>
#include <unordered_map>
#include <iostream>
#include <vector>

#define UNCHECKED_ALLOC_MAX 1000000

class GarbageCollector {
public:
    static boost::asio::thread_pool collector_pool;
    static std::vector<RunTimeVal*> alive_vals;
    static int unchecked_allocs_count;

    static bool massiveGCShouldRun(){
        return alive_vals.size() >= UNCHECKED_ALLOC_MAX;
    }
    static void alloc_val(RunTimeVal* val){
        alive_vals.push_back(val);
    }
    static void mark(std::vector<RunTimeVal*>& vals /*Global Values*/){
        if(vals.size() < 300){
            for(auto& val : vals) val->mark();
            return;
        }
        size_t chunk_size = vals.size() / 4;

        for(size_t i = 0; i < 4; i++){
            size_t start_index = i * chunk_size;
            size_t end_index = (i == (4 - 1)) ? vals.size() : start_index + chunk_size;

            boost::asio::post(collector_pool, [start_index, end_index, vals](){
                for(size_t k = start_index; k < end_index; k++)
                    vals[k]->mark();
            });

        }
        collector_pool.join();        
    }
    static void unMark(std::vector<RunTimeVal*>& vals){
        if(vals.size() < 300){
            for(auto& val : vals) val->mark();
            return;
        }
        size_t chunk_size = vals.size() / 4;

        for(size_t i = 0; i < 4; i++){
            size_t start_index = i * chunk_size;
            size_t end_index = (i == (4 - 1)) ? vals.size() : start_index + chunk_size;

            boost::asio::post(collector_pool, [start_index, end_index, vals](){
                for(size_t k = start_index; k < end_index; k++)
                    vals[k]->unMark();
            });

        }
        collector_pool.join();
    }

    static void sweep(std::pmr::synchronized_pool_resource& mem_pool){
        std::cout << "sweeping" << std::endl;
        for(auto itr = alive_vals.begin(); itr != alive_vals.end(); itr++){
            if(!(*itr)->marked && (*itr) != nullptr) {
                RunTimeVal::deallocateVal(mem_pool, &(*itr));
            }
        }
        alive_vals.erase(std::remove_if(alive_vals.begin(), alive_vals.end(),
            [&](RunTimeVal* val){ return !val; }), alive_vals.end());
        unMark(alive_vals);
        if(alive_vals.size() < 2000){
            for(auto& val : alive_vals) val->mark();
        }
        size_t chunk_size = alive_vals.size() / 4;

        for(size_t i = 0; i < 4; i++){
            size_t start_index = i * chunk_size;
            size_t end_index = (i == (4 - 1)) ? alive_vals.size() : start_index + chunk_size;

            boost::asio::post(collector_pool, [start_index, end_index, &mem_pool](){
                for(size_t k = start_index; k < end_index; k++){
                    if(!(alive_vals[k])->marked && (alive_vals[k]) != nullptr) {
                        RunTimeVal::deallocateVal(mem_pool, &alive_vals[k]);
                    }
                }
            });

        }
        collector_pool.join();
        alive_vals.erase(std::remove_if(alive_vals.begin(), alive_vals.end(),
            [&](RunTimeVal* val){ return !val; }), alive_vals.end());
        unMark(alive_vals);
    }
};
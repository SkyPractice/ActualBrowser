#pragma once
#include "../RunTime.h"
#include <memory_resource>
#include <unordered_map>
#include <vector>
class GarbageCollector {
public:
    static std::vector<RunTimeVal*> alive_vals;

    static void alloc_val(RunTimeVal* val){
        alive_vals.push_back(val);
    }
    static void mark(std::vector<RunTimeVal*>& vals /*Global Values*/){
        for(auto& val : vals) val->mark();
    }
    static void mark(std::unordered_map<std::string, RunTimeVal*>& vals /*Global Values*/){
        for(auto& val : vals){ val.second->mark(); std::cout << val.first << " marked" << std::endl;}
    }
    static void unMark(std::vector<RunTimeVal*>& vals){
        for(auto& val : vals) if(val) val->unMark();
    }

    static void sweep(std::pmr::unsynchronized_pool_resource& mem_pool){
        for(auto itr = alive_vals.begin(); itr != alive_vals.end(); itr++){
            if(!(*itr)->marked && (*itr) != nullptr) {
                std::cout << "value sweeped" << std::endl;
                RunTimeVal::deallocateVal(mem_pool, &(*itr));
            }
        }
        alive_vals.erase(std::remove_if(alive_vals.begin(), alive_vals.end(),
            [&](RunTimeVal* val){ return !val; }), alive_vals.end());
        unMark(alive_vals);
    }
};
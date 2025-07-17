#include "RunTime.h"
#include "SigmaInterpreter.h"
#include <algorithm>
#include <fstream>
#include <memory>
#include <numeric>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <chrono>

RunTimeValue SigmaInterpreter::println(std::vector<RunTimeValue> args){
    if(!std::all_of(args.begin(), args.end(), [](RunTimeValue& arg){
        return arg->type == StringType; })) {
        throw std::runtime_error("all the args provided to \'println\' must be of type \'String\'");
    };

    for(auto& arg : args){
        printf("%s\n", std::dynamic_pointer_cast<StringVal>(arg)->str.c_str());
    }

    return RunTimeFactory::makeNum(std::accumulate(args.begin(), args.end(), 0,
        [](size_t current_total_size, RunTimeValue second) {
            return current_total_size +
                std::dynamic_pointer_cast<StringVal>(second)->str.size();
        }) + 1);
};
RunTimeValue SigmaInterpreter::print(std::vector<RunTimeValue> args){
    if(!std::all_of(args.begin(), args.end(), [](RunTimeValue& arg){
        return arg->type == StringType; })) {
        throw std::runtime_error("all the args provided to \'println\' must be of type \'String\'");
    };

    for(auto& arg : args){ 
        std::cout << std::dynamic_pointer_cast<StringVal>(arg)->str << std::flush;
    }

    return RunTimeFactory::makeNum(std::accumulate(args.begin(), args.end(), 0,
        [](size_t current_total_size, RunTimeValue second) {
            return current_total_size +
                std::dynamic_pointer_cast<StringVal>(second)->str.size();
        }));
};
RunTimeValue SigmaInterpreter::toString(std::vector<RunTimeValue> args) {
    if(args.size() > 0){
        return RunTimeFactory::makeString(args[0]->getString());
    } else return RunTimeFactory::makeString("");
};

// args -> [0] : size, [1] : start
RunTimeValue SigmaInterpreter::numIota(std::vector<RunTimeValue> args) {
    return nullptr;
};

RunTimeValue SigmaInterpreter::readFileSync(std::vector<RunTimeValue> args) {
    if(args[0]->type != StringType) throw
        std::runtime_error("readFileSync Excepts arg 0 to be of type String");
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    if(!std::filesystem::exists(path)) 
        throw std::runtime_error("file " + path + " doesn't exist");
    std::ifstream strea(path, std::ios::ate);
    size_t s = strea.tellg();
    std::string strr;
    strr.resize(s);
    strea.seekg(0, std::ios::beg);
    strea.read(&strr[0], s);
    strea.close();
    return RunTimeFactory::makeString(std::move(strr));
};
RunTimeValue SigmaInterpreter::writeFileSync(std::vector<RunTimeValue> args) {
    if(args[0]->type != StringType || args[0]->type != StringType) throw
        std::runtime_error("writeFileSync Excepts arg 0 and 1 to be of type String");
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    std::string str = std::dynamic_pointer_cast<StringVal>(args[1])->str;
    std::ofstream strea(path);
    strea.write(&str[0], str.size());
    strea.close();
    return nullptr;
};

RunTimeValue SigmaInterpreter::clone(std::vector<RunTimeValue> args) {
    return args[0]->clone();
};

RunTimeValue SigmaInterpreter::input(std::vector<RunTimeValue> args) {
    std::string str;
    std::getline(std::cin, str);
    return RunTimeFactory::makeString(str);
};

RunTimeValue SigmaInterpreter::getCurrentTimeMillis(std::vector<RunTimeValue> args) {
    std::chrono::time_point tim = std::chrono::high_resolution_clock::now();
    long actual_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        tim).time_since_epoch().count();
    
    return RunTimeFactory::makeNum(actual_time);
};
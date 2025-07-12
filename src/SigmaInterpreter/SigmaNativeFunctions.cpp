#include "RunTime.h"
#include "SigmaInterpreter.h"
#include <algorithm>
#include <memory>
#include <numeric>
#include <stdexcept>

RunTimeValue SigmaInterpreter::println(std::vector<RunTimeValue> args){
    if(!std::all_of(args.begin(), args.end(), [](RunTimeValue& arg){
        return arg->type == StringType; })) {
        throw std::runtime_error("all the args provided to \'println\' must be of type \'String\'");
    };

    for(auto& arg : args){
        printf("%s\n", std::dynamic_pointer_cast<StringVal>(arg)->str.c_str());
    }

    return std::make_shared<NumVal>(std::accumulate(args.begin(), args.end(), 0,
        [](size_t current_total_size, RunTimeValue second) {
            return current_total_size +
                std::dynamic_pointer_cast<StringVal>(second)->str.size();
        }));
};

RunTimeValue SigmaInterpreter::toString(std::vector<RunTimeValue> args) {
    if(args.size() > 0){
        switch (args[0]->type) {
            case NumType:
                return std::make_shared<StringVal>(
                    std::to_string(std::dynamic_pointer_cast<NumVal>(args[0])->num));
        }
    } else return std::make_shared<StringVal>("");
};

// args -> [0] : size, [1] : start
RunTimeValue SigmaInterpreter::numIota(std::vector<RunTimeValue> args) {
    return nullptr;
};
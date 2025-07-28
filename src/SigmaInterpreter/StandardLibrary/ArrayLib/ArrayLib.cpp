#include "ArrayLib.h"
#include "../../SigmaInterpreter.h"

StructVal* ArrayLib::getStruct() {
    std::unordered_map<std::string, RunTimeVal*> vals = {
        {"resizeArray", RunTimeFactory::makeNativeFunction(&ArrayLib::resizeArray)},
        {"pushBackArray", RunTimeFactory::makeNativeFunction(&ArrayLib::pushBackArray)},
        {"popBackArray", RunTimeFactory::makeNativeFunction(&ArrayLib::popBackArray)},
        {"pushFirstArray", RunTimeFactory::makeNativeFunction(&ArrayLib::pushFirstArray)},
        {"popFirstArray", RunTimeFactory::makeNativeFunction(&ArrayLib::popFirstArray)},
        {"insertIntoArray", RunTimeFactory::makeNativeFunction(&ArrayLib::insertIntoArray)},
        {"getArraySize", RunTimeFactory::makeNativeFunction(&ArrayLib::arraySize)}
    };

    return RunTimeFactory::makeStruct(std::move(vals));
};

RunTimeValue ArrayLib::resizeArray(std::vector<RunTimeValue>& args, SigmaInterpreter*) {
    auto arr = dynamic_cast<ArrayVal*>(args[0]);
    auto new_size = static_cast<size_t>(dynamic_cast<NumVal*>(args[1])->num);

    arr->vals.resize(new_size);
    return args[1];
};

RunTimeValue ArrayLib::pushBackArray(std::vector<RunTimeValue>& args, SigmaInterpreter*){
    auto arr = dynamic_cast<ArrayVal*>(args[0]);
    arr->vals.push_back(SigmaInterpreter::copyIfRecommended(args[1]));
    return nullptr;
};
RunTimeValue ArrayLib::popBackArray(std::vector<RunTimeValue>& args, SigmaInterpreter*){
    auto arr = dynamic_cast<ArrayVal*>(args[0]);
    arr->vals.pop_back();
    return nullptr;
};
RunTimeValue ArrayLib::pushFirstArray(std::vector<RunTimeValue>& args, SigmaInterpreter*){
    auto arr = dynamic_cast<ArrayVal*>(args[0]);
    arr->vals.insert(arr->vals.begin(), SigmaInterpreter::copyIfRecommended(args[1]));
    return nullptr;
};
RunTimeValue ArrayLib::popFirstArray(std::vector<RunTimeValue>& args, SigmaInterpreter*){
    auto arr = dynamic_cast<ArrayVal*>(args[0]);
    arr->vals.erase(arr->vals.begin());
    return nullptr;
};
RunTimeValue ArrayLib::insertIntoArray(std::vector<RunTimeValue>& args, SigmaInterpreter*){
    auto arr = dynamic_cast<ArrayVal*>(args[0]);
    size_t index = static_cast<size_t>(dynamic_cast<NumVal*>(args[1])->num);
    arr->vals.insert(arr->vals.begin() + index, SigmaInterpreter::copyIfRecommended(args[2]));
    return nullptr;
};

RunTimeValue ArrayLib::arraySize(std::vector<RunTimeValue>& args, SigmaInterpreter*) {
    return RunTimeFactory::makeNum(dynamic_cast<ArrayVal*>(args[0])->vals.size());
};
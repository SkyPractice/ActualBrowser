#include "RunTime.h"
#include "GarbageCollector/GarbageCollector.h"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

std::vector<RunTimeVal*>* RunTimeFactory::target_alloc_vec = nullptr;
std::pmr::synchronized_pool_resource RunTimeMemory::pool;
std::mutex RunTimeMemory::pool_mut = std::mutex();

NumVal* RunTimeFactory::makeNum(double num) {
    return makeVal<NumVal>(num);
};
StringVal* RunTimeFactory::makeString(std::string str) {
    return makeVal<StringVal>(std::move(str));
};
CharVal* RunTimeFactory::makeChar(char ch) {
    return makeVal<CharVal>(ch);
};
ArrayVal* RunTimeFactory::makeArray(std::vector<RunTimeVal*> vec){
    return makeVal<ArrayVal>(std::move(vec));
};
ObjectVal* RunTimeFactory::makeStruct(std::unordered_map<std::string,
    RunTimeVal*> vals) {
    return makeVal<ObjectVal>(std::move(vals));
};
ReturnVal* RunTimeFactory::makeReturn(RunTimeVal* val) {
    return makeVal<ReturnVal>(std::move(val));
};
BreakVal* RunTimeFactory::makeBreak() {
    return makeVal<BreakVal>();
};
ContinueVal* RunTimeFactory::makeContinue() {
    return makeVal<ContinueVal>();
};

BoolVal* RunTimeFactory::makeBool(bool boolean) {
    return makeVal<BoolVal>(boolean);
};

LambdaVal* RunTimeFactory::makeLambda(std::vector<std::string> params, 
    std::vector<Statement*> stmts, std::unordered_map<std::string, 
        RunTimeVal*> captured) {
    return makeVal<LambdaVal>(std::move(params), std::move(stmts), std::move(captured));
};

NativeFunctionVal* RunTimeFactory::makeNativeFunction(
        NativeFunctionVal::FuncType func, std::vector<ArgInformation> arg_information
){
    return makeVal<NativeFunctionVal>(std::move(func), std::move(arg_information));
};

RefrenceVal* RunTimeFactory::makeRefrence(RunTimeVal** val){
    return makeVal<RefrenceVal>(val);
}

RunTimeVal* NullVal::clone() {
    return this;
};
RunTimeVal* NumVal::clone() { return RunTimeFactory::makeNum(num); };
RunTimeVal* StringVal::clone() { return RunTimeFactory::makeString(str); };
RunTimeVal* CharVal::clone() { return nullptr; };
RunTimeVal* BoolVal::clone() { return RunTimeFactory::makeBool(boolean); };
RunTimeVal* LambdaVal::clone() { return RunTimeFactory::makeLambda(params,
     stmts, captured); };
RunTimeVal* ArrayVal::clone() { 
    std::vector<RunTimeVal*> new_arr(vals.size());
    std::transform(vals.begin(), vals.end(), new_arr.begin(),
     [&](RunTimeVal* val){
        return val->clone();
    });
    return RunTimeFactory::makeArray(new_arr); };
RunTimeVal* ObjectVal::clone() { 
    std::unordered_map<std::string, RunTimeVal*> valss;
    for(auto& [val_name, value] : vals){
        valss.insert({val_name, value->clone() });
    }
    return RunTimeFactory::makeStruct(valss); };
RunTimeVal* ReturnVal::clone() { return RunTimeFactory::makeReturn(val->clone()); };
RunTimeVal* BreakVal::clone() { return RunTimeFactory::makeBreak(); };
RunTimeVal* ContinueVal::clone() { return RunTimeFactory::makeContinue(); };
RunTimeVal* RefrenceVal::clone() { return RunTimeFactory::makeRefrence(val); };
RunTimeVal* NativeFunctionVal::clone() { return this; };

BinaryVal* RunTimeFactory::makeBinary(
    std::vector<unsigned char> d
) {
    return makeVal<BinaryVal>(std::move(d));
};

RunTimeVal* BinaryVal::clone() {
    return RunTimeFactory::makeBinary(binary_data);
}

std::string BinaryVal::getString(){
    std::ostringstream sstrea;

    for(int i = 0; i < binary_data.size(); i++){
        sstrea << std::hex << std::setw(2) << std::setfill('0') << (int)binary_data[i];
    }
    return sstrea.str();
}

HtmlElementVal* RunTimeFactory::makeHtmlElement(HTMLTag* tag) {
    return makeVal<HtmlElementVal>(tag);
};

// Json Serialization
std::string ObjectVal::getString() {
    if(vals.contains("is_primitive")){
        return vals["primitive"]->getString();
    }
    std::ostringstream result_stream;
    result_stream << "{ ";
    for(auto& [name, val] : vals){
        result_stream << "\"" << name << "\"" << " : " << val->getString() << ", ";
    }
    std::string result_str = result_stream.str();
    if(result_str != "{")
        result_str.erase(result_str.size() - 2,1);
    result_str += "}";
    return result_str;
}
#include "RunTime.h"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <unordered_map>

std::vector<RunTimeVal*>* RunTimeFactory::target_alloc_vec = nullptr;
std::pmr::unsynchronized_pool_resource RunTimeMemory::pool;

NumVal* RunTimeFactory::makeNum(double num) {
    return makeVal<NumVal>(num);
};
StringVal* RunTimeFactory::makeString(std::string str) {
    return makeVal<StringVal>(std::move(str));
};
ArrayVal* RunTimeFactory::makeArray(std::vector<RunTimeVal*> vec){
    return makeVal<ArrayVal>(std::move(vec));
};
StructVal* RunTimeFactory::makeStruct(std::unordered_map<std::string,
    RunTimeVal*> vals) {
    return makeVal<StructVal>(std::move(vals));
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
        NativeFunctionVal::FuncType func
){
    return makeVal<NativeFunctionVal>(std::move(func));
};

RefrenceVal* RunTimeFactory::makeRefrence(RunTimeVal** val){
    return makeVal<RefrenceVal>(val);
}

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
RunTimeVal* StructVal::clone() { 
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
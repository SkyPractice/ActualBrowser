#include "RunTime.h"
#include <memory>

std::pmr::unsynchronized_pool_resource RunTimeMemory::pool;

std::shared_ptr<NumVal> RunTimeFactory::makeNum(double num) {
    return makeVal<NumVal>(num);
};
std::shared_ptr<StringVal> RunTimeFactory::makeString(std::string str) {
    return makeVal<StringVal>((str));
};
std::shared_ptr<ArrayVal> RunTimeFactory::makeArray(std::vector<std::shared_ptr<RunTimeVal>> vec){
    return makeVal<ArrayVal>((vec));
};
std::shared_ptr<StructVal> RunTimeFactory::makeStruct(std::unordered_map<std::string,
    std::shared_ptr<RunTimeVal>> vals) {
    return makeVal<StructVal>((vals));
};
std::shared_ptr<ReturnVal> RunTimeFactory::makeReturn(std::shared_ptr<RunTimeVal> val) {
    return makeVal<ReturnVal>((val));
};
std::shared_ptr<BreakVal> RunTimeFactory::makeBreak() {
    return makeVal<BreakVal>();
};
std::shared_ptr<ContinueVal> RunTimeFactory::makeContinue() {
    return makeVal<ContinueVal>();
};

std::shared_ptr<BoolVal> RunTimeFactory::makeBool(bool boolean) {
    return makeVal<BoolVal>(boolean);
};

std::shared_ptr<LambdaVal> RunTimeFactory::makeLambda(std::vector<std::string> params, 
    std::vector<std::shared_ptr<Statement>> stmts) {
    return makeVal<LambdaVal>((params), (stmts));
};

std::shared_ptr<NativeFunctionVal> RunTimeFactory::makeNativeFunction(
        NativeFunctionVal::FuncType func
){
    return makeVal<NativeFunctionVal>((func));
};
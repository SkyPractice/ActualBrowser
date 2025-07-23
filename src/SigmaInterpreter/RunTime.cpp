#include "RunTime.h"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <unordered_map>

std::pmr::unsynchronized_pool_resource RunTimeMemory::pool;

std::shared_ptr<NumVal> RunTimeFactory::makeNum(double num) {
    return makeVal<NumVal>(num);
};
std::shared_ptr<StringVal> RunTimeFactory::makeString(std::string str) {
    return makeVal<StringVal>(std::move(str));
};
std::shared_ptr<ArrayVal> RunTimeFactory::makeArray(std::vector<std::shared_ptr<RunTimeVal>> vec){
    return makeVal<ArrayVal>(std::move(vec));
};
std::shared_ptr<StructVal> RunTimeFactory::makeStruct(std::unordered_map<std::string,
    std::shared_ptr<RunTimeVal>> vals) {
    return makeVal<StructVal>(std::move(vals));
};
std::shared_ptr<ReturnVal> RunTimeFactory::makeReturn(std::shared_ptr<RunTimeVal> val) {
    return makeVal<ReturnVal>(std::move(val));
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
    std::vector<std::shared_ptr<Statement>> stmts, std::unordered_map<std::string, 
        std::shared_ptr<RunTimeVal>> captured) {
    return makeVal<LambdaVal>(std::move(params), std::move(stmts), std::move(captured));
};

std::shared_ptr<NativeFunctionVal> RunTimeFactory::makeNativeFunction(
        NativeFunctionVal::FuncType func
){
    return makeVal<NativeFunctionVal>(std::move(func));
};

std::shared_ptr<RefrenceVal> RunTimeFactory::makeRefrence(std::shared_ptr<RunTimeVal> *val){
    return makeVal<RefrenceVal>(val);
}

std::shared_ptr<RunTimeVal> NumVal::clone() { return RunTimeFactory::makeNum(num); };
std::shared_ptr<RunTimeVal> StringVal::clone() { return RunTimeFactory::makeString(str); };
std::shared_ptr<RunTimeVal> CharVal::clone() { return nullptr; };
std::shared_ptr<RunTimeVal> BoolVal::clone() { return RunTimeFactory::makeBool(boolean); };
std::shared_ptr<RunTimeVal> LambdaVal::clone() { return RunTimeFactory::makeLambda(params,
     stmts, captured); };
std::shared_ptr<RunTimeVal> ArrayVal::clone() { 
    std::vector<std::shared_ptr<RunTimeVal>> new_arr(vals.size());
    std::transform(vals.begin(), vals.end(), new_arr.begin(),
     [&](std::shared_ptr<RunTimeVal>& val){
        return val->clone();
    });
    return RunTimeFactory::makeArray(new_arr); };
std::shared_ptr<RunTimeVal> StructVal::clone() { 
    std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> valss;
    for(auto& [val_name, value] : vals){
        valss.insert({val_name, value->clone() });
    }
    return RunTimeFactory::makeStruct(valss); };
std::shared_ptr<RunTimeVal> ReturnVal::clone() { return RunTimeFactory::makeReturn(val->clone()); };
std::shared_ptr<RunTimeVal> BreakVal::clone() { return RunTimeFactory::makeBreak(); };
std::shared_ptr<RunTimeVal> ContinueVal::clone() { return RunTimeFactory::makeContinue(); };
std::shared_ptr<RunTimeVal> RefrenceVal::clone() { return RunTimeFactory::makeRefrence(val); };
std::shared_ptr<RunTimeVal> NativeFunctionVal::clone() { return shared_from_this(); };

std::shared_ptr<BinaryVal> RunTimeFactory::makeBinary(
    std::vector<unsigned char> d
) {
    return makeVal<BinaryVal>(std::move(d));
};

std::shared_ptr<RunTimeVal> BinaryVal::clone() {
    return RunTimeFactory::makeBinary(binary_data);
}

std::string BinaryVal::getString(){
    std::ostringstream sstrea;

    for(int i = 0; i < binary_data.size(); i++){
        sstrea << std::hex << std::setw(2) << std::setfill('0') << (int)binary_data[i];
    }
    return sstrea.str();
}

std::shared_ptr<HtmlElementVal> RunTimeFactory::makeHtmlElement(std::shared_ptr<HTMLTag> tag) {
    return makeVal<HtmlElementVal>(tag);
};
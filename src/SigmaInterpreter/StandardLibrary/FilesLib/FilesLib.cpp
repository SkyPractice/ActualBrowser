#include "FilesLib.h"
#include <boost/asio/post.hpp>
#include <fstream>
#include <memory>
#include <unordered_map>
#include "../../../Concurrency/ThreadPool.h"
#include "../../SigmaInterpreter.h"

std::shared_ptr<StructVal> FilesLib::getStruct() {
    std::unordered_map<std::string, RunTimeValue> vals = {
        {"readFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::readFileSync)},
        {"writeFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::writeFileSync)},
        {"readBinaryFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::readBinaryFileSync)},
        {"writeBinaryFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::writeBinaryFileSync)},
        {"readFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::readFileAsync)},
        {"writeFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::writeFileAsync)},
        {"readBinaryFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::readBinaryFileAsync)},
        {"writeBinaryFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::writeBinaryFileAsync)}
    };

    return RunTimeFactory::makeStruct(std::move(vals));
};

RunTimeValue FilesLib::readFileSync(std::vector<RunTimeValue>& args) {
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
RunTimeValue FilesLib::writeFileSync(std::vector<RunTimeValue>& args) {
    if(args[0]->type != StringType || args[0]->type != StringType) throw
        std::runtime_error("writeFileSync Excepts arg 0 and 1 to be of type String");
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    std::string str = std::dynamic_pointer_cast<StringVal>(args[1])->str;
    std::ofstream strea(path);
    strea.write(&str[0], str.size());
    strea.close();
    return RunTimeFactory::makeNum(0); // success
};

RunTimeValue FilesLib::writeBinaryFileSync(std::vector<RunTimeValue>& args){
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    std::vector<unsigned char> val = std::dynamic_pointer_cast<BinaryVal>(args[1])->binary_data;
    std::ofstream strea(path, std::ios::binary);
    size_t siz = val.size();
    strea.write(reinterpret_cast<const char*>(&siz), sizeof(size_t));
    strea.write(reinterpret_cast<const char*>(&val[0]), val.size());
    strea.close();
    return RunTimeFactory::makeNum(0); // success
};
RunTimeValue FilesLib::readBinaryFileSync(std::vector<RunTimeValue>& args){
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    std::vector<unsigned char> val;
    size_t siz = 0;
    std::ifstream strea(path, std::ios::binary);
    strea.read(reinterpret_cast<char*>(&siz), sizeof(size_t));
    val.resize(siz);
    strea.read(reinterpret_cast<char*>(&val[0]), siz);
    strea.close();
    return RunTimeFactory::makeBinary(val);
};


RunTimeValue FilesLib::readFileAsync(std::vector<RunTimeValue>& args) {
    std::vector<RunTimeValue> actual_args = args;
    boost::asio::post(Concurrency::pool, [args]() mutable {
        auto lambda_val = std::dynamic_pointer_cast<LambdaVal>(args[1]);
        SigmaInterpreter interp;
        interp.initialize();
        interp.evaluateAnonymousLambdaCall(lambda_val, {FilesLib::readFileSync(args)});
    });
    return nullptr;
};
RunTimeValue FilesLib::writeFileAsync(std::vector<RunTimeValue>& args) {
    std::vector<RunTimeValue> actual_args = args;
    boost::asio::post(Concurrency::pool, [args]() mutable {
        auto lambda_val = std::dynamic_pointer_cast<LambdaVal>(args[2]);
        SigmaInterpreter interp;
        interp.initialize();
        interp.evaluateAnonymousLambdaCall(lambda_val, {FilesLib::writeFileSync(args)});
    });
    return nullptr;
};
RunTimeValue FilesLib::writeBinaryFileAsync(std::vector<RunTimeValue>& args) {
    std::vector<RunTimeValue> actual_args = args;
    boost::asio::post(Concurrency::pool, [args]() mutable {
        auto lambda_val = std::dynamic_pointer_cast<LambdaVal>(args[2]);
        SigmaInterpreter interp;
        interp.initialize();
        interp.evaluateAnonymousLambdaCall(lambda_val, {FilesLib::writeBinaryFileSync(args)});
    });
    return nullptr;
};
RunTimeValue FilesLib::readBinaryFileAsync(std::vector<RunTimeValue>& args) {
    std::vector<RunTimeValue> actual_args = args;
    boost::asio::post(Concurrency::pool, [args]() mutable {
        auto lambda_val = std::dynamic_pointer_cast<LambdaVal>(args[1]);
        SigmaInterpreter interp;
        interp.initialize();
        interp.evaluateAnonymousLambdaCall(lambda_val, {FilesLib::readBinaryFileSync(args)});
    });
    return nullptr;
};
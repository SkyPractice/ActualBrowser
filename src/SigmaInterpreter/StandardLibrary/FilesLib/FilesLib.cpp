#include "FilesLib.h"
#include <boost/asio/post.hpp>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include "../../../Concurrency/ThreadPool.h"
#include "../../SigmaInterpreter.h"
#include "../TypeWrappers/StringWrapper.h"

ObjectVal* FilesLib::getStruct() {
    std::unordered_map<std::string, RunTimeValue> vals = {
        {"readFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::readFileSync, {{"file_path", StringType}})},
        {"writeFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::writeFileSync, {{"file_path", StringType}, {"string", StringType}})},
        {"readBinaryFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::readBinaryFileSync, {{"file_name", StringType}})},
        {"writeBinaryFileSync", RunTimeFactory::makeNativeFunction(&FilesLib::writeBinaryFileSync, {{"file_name", StringType}, {"binary", BinaryType}})},
        {"readFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::readFileAsync, {{"file_name", StringType}, {"handler", LambdaType}})},
        {"writeFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::writeFileAsync, {{"file_name", StringType}, {"string", StringType}, {"handler", LambdaType}})},
        {"readBinaryFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::readBinaryFileAsync, {{"file_name", StringType}, {"handler", LambdaType}})},
        {"writeBinaryFileAsync", RunTimeFactory::makeNativeFunction(&FilesLib::writeBinaryFileAsync, {{"file_name", StringType}, {"binary", BinaryType}, {"handler", LambdaType}})}
    };

    return RunTimeFactory::makeStruct(std::move(vals));
};

RunTimeValue FilesLib::readFileSync(COMPILED_FUNC_ARGS) {
    if(!interpreter->perms.isPermissionGranted(FileReading) && 
        !interpreter->perms.isPermissionGranted(FileFullAccess))
        throw std::runtime_error("Permission \'FileReading\' Not Granted!");
    if(args[0]->type != StringType) throw
        std::runtime_error("readFileSync Excepts arg 0 to be of type String");
    std::string path = static_cast<StringVal*>(args[0])->str;
    if(!std::filesystem::exists(path)) 
        throw std::runtime_error("file " + path + " doesn't exist");
    std::ifstream strea(path, std::ios::ate);
    size_t s = strea.tellg();
    std::string strr;
    strr.resize(s);
    strea.seekg(0, std::ios::beg);
    strea.read(&strr[0], s);
    strea.close();
    return  StringWrapper::genObject(RunTimeFactory::makeString(std::move(strr)));
};
RunTimeValue FilesLib::writeFileSync(COMPILED_FUNC_ARGS) {
    if(!interpreter->perms.isPermissionGranted(FileWriting) && 
        !interpreter->perms.isPermissionGranted(FileFullAccess))
        throw std::runtime_error("Permission \'FileWriting\' Not Granted!");
    if(args[0]->type != StringType || args[0]->type != StringType) throw
        std::runtime_error("writeFileSync Excepts arg 0 and 1 to be of type String");
    std::string path = static_cast<StringVal*>(args[0])->str;
    std::string str = static_cast<StringVal*>(args[1])->str;
    std::ofstream strea(path);
    strea.write(&str[0], str.size());
    strea.close();
    return RunTimeFactory::makeNum(0); // success
};

RunTimeValue FilesLib::writeBinaryFileSync(COMPILED_FUNC_ARGS){
    if(!interpreter->perms.isPermissionGranted(FileReading) && 
        !interpreter->perms.isPermissionGranted(FileFullAccess))
        throw std::runtime_error("Permission \'FileWriting\' Not Granted!");
    std::string path = static_cast<StringVal*>(args[0])->str;
    std::vector<unsigned char> val = static_cast<BinaryVal*>(args[1])->binary_data;
    std::ofstream strea(path, std::ios::binary);
    size_t siz = val.size();
    strea.write(reinterpret_cast<const char*>(&siz), sizeof(size_t));
    strea.write(reinterpret_cast<const char*>(&val[0]), val.size());
    strea.close();
    return RunTimeFactory::makeNum(0); // success
};
RunTimeValue FilesLib::readBinaryFileSync(COMPILED_FUNC_ARGS){
    if(!interpreter->perms.isPermissionGranted(FileReading) && 
        !interpreter->perms.isPermissionGranted(FileFullAccess))
        throw std::runtime_error("Permission \'FileReading\' Not Granted!");
    std::string path = static_cast<StringVal*>(args[0])->str;
    std::vector<unsigned char> val;
    size_t siz = 0;
    std::ifstream strea(path, std::ios::binary);
    strea.read(reinterpret_cast<char*>(&siz), sizeof(size_t));
    val.resize(siz);
    strea.read(reinterpret_cast<char*>(&val[0]), siz);
    strea.close();
    return RunTimeFactory::makeBinary(val);
};


RunTimeValue FilesLib::readFileAsync(COMPILED_FUNC_ARGS) {
    std::vector<RunTimeValue> actual_args = args;
    auto lambda_val = static_cast<LambdaVal*>(args[1]);

    interpreter->garbageCollectionRestricter.registerAsyncLambda(lambda_val);
    boost::asio::post(Concurrency::pool, [args, interpreter, lambda_val]() mutable {
        interpreter->evaluateAnonymousLambdaCall(lambda_val, {FilesLib::readFileSync(args, interpreter)});
        interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda_val->lambda_uuid);
    });
    return nullptr;
};
RunTimeValue FilesLib::writeFileAsync(COMPILED_FUNC_ARGS) {
    std::vector<RunTimeValue> actual_args = args;
    auto lambda_val = static_cast<LambdaVal*>(args[2]);

    interpreter->garbageCollectionRestricter.registerAsyncLambda(lambda_val);
    boost::asio::post(Concurrency::pool, [args, interpreter, lambda_val]() mutable {
        interpreter->evaluateAnonymousLambdaCall(lambda_val, {FilesLib::writeFileSync(args, interpreter)});
        interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda_val->lambda_uuid);
    });
    return nullptr;
};
RunTimeValue FilesLib::writeBinaryFileAsync(COMPILED_FUNC_ARGS) {
    auto lambda_val = static_cast<LambdaVal*>(args[2]);
    
    interpreter->garbageCollectionRestricter.registerAsyncLambda(lambda_val);
    boost::asio::post(Concurrency::pool, [args, interpreter, lambda_val]() mutable {
        interpreter->evaluateAnonymousLambdaCall(lambda_val, {FilesLib::writeBinaryFileSync(args, interpreter)});
        interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda_val->lambda_uuid);
    });
    return nullptr;
};
RunTimeValue FilesLib::readBinaryFileAsync(COMPILED_FUNC_ARGS) {
    auto lambda_val = static_cast<LambdaVal*>(args[1]);

    interpreter->garbageCollectionRestricter.registerAsyncLambda(lambda_val);
    boost::asio::post(Concurrency::pool, [args, interpreter, lambda_val]() mutable {
        interpreter->evaluateAnonymousLambdaCall(lambda_val, {FilesLib::readBinaryFileSync(args, interpreter)});
        interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda_val->lambda_uuid);
    });
    return nullptr;
};
#include "RunTime.h"
#include "SigmaInterpreter.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <memory>
#include <numeric>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <vector>
#include "../Interpreter/Interpreter.h"
#include "../Interpreter/Lexer.h"
#include "../Interpreter/Parser.h"
#include "Cryptography.h"


RunTimeValue SigmaInterpreter::println(std::vector<RunTimeValue>& args){
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
RunTimeValue SigmaInterpreter::print(std::vector<RunTimeValue>& args){
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
RunTimeValue SigmaInterpreter::toString(std::vector<RunTimeValue>& args) {
    if(args.size() > 0){
        return RunTimeFactory::makeString(args[0]->getString());
    } else return RunTimeFactory::makeString("");
};

// args -> [0] : size, [1] : start
RunTimeValue SigmaInterpreter::numIota(std::vector<RunTimeValue>& args) {
    return nullptr;
};

RunTimeValue SigmaInterpreter::readFileSync(std::vector<RunTimeValue>& args) {
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
RunTimeValue SigmaInterpreter::writeFileSync(std::vector<RunTimeValue>& args) {
    if(args[0]->type != StringType || args[0]->type != StringType) throw
        std::runtime_error("writeFileSync Excepts arg 0 and 1 to be of type String");
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    std::string str = std::dynamic_pointer_cast<StringVal>(args[1])->str;
    std::ofstream strea(path);
    strea.write(&str[0], str.size());
    strea.close();
    return nullptr;
};

RunTimeValue SigmaInterpreter::writeBinaryFileSync(std::vector<RunTimeValue>& args){
    std::string path = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    std::vector<unsigned char> val = std::dynamic_pointer_cast<BinaryVal>(args[1])->binary_data;
    std::ofstream strea(path, std::ios::binary);
    size_t siz = val.size();
    strea.write(reinterpret_cast<const char*>(&siz), sizeof(size_t));
    strea.write(reinterpret_cast<const char*>(&val[0]), val.size());
    strea.close();
    return nullptr;
};
RunTimeValue SigmaInterpreter::readBinaryFileSync(std::vector<RunTimeValue>& args){
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

RunTimeValue SigmaInterpreter::clone(std::vector<RunTimeValue>& args) {
    return args[0]->clone();
};

RunTimeValue SigmaInterpreter::input(std::vector<RunTimeValue>& args) {
    std::string str;
    std::getline(std::cin, str);
    return RunTimeFactory::makeString(str);
};

RunTimeValue SigmaInterpreter::getCurrentTimeMillis(std::vector<RunTimeValue>& args) {
    std::chrono::time_point tim = std::chrono::high_resolution_clock::now();
    long actual_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        tim).time_since_epoch().count();
    
    return RunTimeFactory::makeNum(actual_time);
};

RunTimeValue SigmaInterpreter::resizeArray(std::vector<RunTimeValue>& args) {
    auto arr = std::dynamic_pointer_cast<ArrayVal>(args[0]);
    auto new_size = static_cast<size_t>(std::dynamic_pointer_cast<NumVal>(args[1])->num);

    arr->vals.resize(new_size);
    return args[1];
};

RunTimeValue SigmaInterpreter::pushBackArray(std::vector<RunTimeValue>& args){
    auto arr = std::dynamic_pointer_cast<ArrayVal>(args[0]);
    arr->vals.push_back(copyIfRecommended(args[1]));
    return nullptr;
};
RunTimeValue SigmaInterpreter::popBackArray(std::vector<RunTimeValue>& args){
    auto arr = std::dynamic_pointer_cast<ArrayVal>(args[0]);
    arr->vals.pop_back();
    return nullptr;
};
RunTimeValue SigmaInterpreter::pushFirstArray(std::vector<RunTimeValue>& args){
    auto arr = std::dynamic_pointer_cast<ArrayVal>(args[0]);
    arr->vals.insert(arr->vals.begin(), copyIfRecommended(args[1]));
    return nullptr;
};
RunTimeValue SigmaInterpreter::popFirstArray(std::vector<RunTimeValue>& args){
    auto arr = std::dynamic_pointer_cast<ArrayVal>(args[0]);
    arr->vals.erase(arr->vals.begin());
    return nullptr;
};
RunTimeValue SigmaInterpreter::insertIntoArray(std::vector<RunTimeValue>& args){
    auto arr = std::dynamic_pointer_cast<ArrayVal>(args[0]);
    size_t index = static_cast<size_t>(std::dynamic_pointer_cast<NumVal>(args[1])->num);
    arr->vals.insert(arr->vals.begin() + index, copyIfRecommended(args[2]));
    return nullptr;
};
RunTimeValue SigmaInterpreter::Sha256Wrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeString(Crypto::Sha256(std::dynamic_pointer_cast<StringVal>(args[0])->str));
};

RunTimeValue SigmaInterpreter::Sha512Wrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeString(Crypto::Sha512(std::dynamic_pointer_cast<StringVal>(args[0])->str));
};
RunTimeValue SigmaInterpreter::Aes256Wrapper(std::vector<RunTimeValue>& args) {
    std::vector<unsigned char> res = Crypto::Aes256(std::dynamic_pointer_cast<StringVal>(args[0])->str,
        std::dynamic_pointer_cast<BinaryVal>(args[1])->binary_data);
    return RunTimeFactory::makeBinary(res);
};

RunTimeValue SigmaInterpreter::Aes256GenKeyWrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeBinary(Crypto::genAes256Key());
};

RunTimeValue SigmaInterpreter::Aes256DecryptWrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeString(Crypto::decryptAes256(
        std::dynamic_pointer_cast<BinaryVal>(args[0])->binary_data,
         std::dynamic_pointer_cast<BinaryVal>(args[1])->binary_data));
};

RunTimeValue SigmaInterpreter::getElementById(std::vector<RunTimeValue>& args) {
    std::string id = std::dynamic_pointer_cast<StringVal>(args[0])->str;
    return RunTimeFactory::makeHtmlElement(accessor->id_ptrs.at(id));
};
RunTimeValue SigmaInterpreter::setElementInnerHtml(std::vector<RunTimeValue>& args){
    auto html_elm = std::dynamic_pointer_cast<HtmlElementVal>(args[0]);
    Lexer lex;
    Parser pars;
    Interpreter interpret;
    std::string html_str = std::dynamic_pointer_cast<StringVal>(args[1])->str;
    
    auto tokens = lex.tokenize(html_str);
    auto ast_val = pars.produceAst(tokens);
    for(auto& child : html_elm->target_tag->children)
        child->unRender();
    html_elm->target_tag->setChildren(ast_val.html_tags);
    interpret.renderTags(dynamic_cast<Gtk::Box*>(html_elm->target_tag->current_widget), ast_val);
    accessor->current_interp->refreshIdsAndClasses();
    return nullptr;
};
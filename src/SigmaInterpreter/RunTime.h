#pragma once
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <sys/cdefs.h>
#include <vector>
#include "SigmaAst.h"
#include <unordered_map>
#include <memory_resource>
#include "../Interpreter/Ast.h"

class SigmaInterpreter;


enum RunTimeValType {
    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType, ReturnType,
    BreakType, ContinueType, NativeFunctionType, RefrenceType, BinaryType, HtmlType, AnyType,
    NullType
};


struct ArgInformation {
    std::string arg_name;
    RunTimeValType type;
    bool optional = false;
};

class RunTimeVal {
public:
    bool invincible = false;
    bool marked = false;
    bool is_l_val = false;

    RunTimeValType type;
    RunTimeVal(RunTimeValType t): type(t) {
    };

    virtual RunTimeVal* clone() = 0;
    virtual ~RunTimeVal() {};
    virtual void setValue(RunTimeVal* val) {};
    virtual void markChildren() {};
    virtual void unMarkChildren() {};
    virtual void mark() { if(!marked) { marked = true; markChildren(); } };
    virtual void unMark() { if(marked) { marked = false; unMarkChildren(); } };
    virtual void cleanUpChildren(std::pmr::synchronized_pool_resource& target_pool) {};
    virtual size_t getSize(){ return sizeof(RunTimeVal); };
    virtual size_t getAlignment() { return alignof(RunTimeVal); };
    virtual std::string getString() { return ""; };
    static void deallocateVal(std::pmr::synchronized_pool_resource& target_pool, RunTimeVal** ptr){
        auto size = (*ptr)->getSize();
        auto alignment = (*ptr)->getAlignment();
        (*ptr)->~RunTimeVal();
        target_pool.deallocate((*ptr), size, alignment);
        *ptr = nullptr;
    }
};

class NullVal : public RunTimeVal {
public:
    NullVal(): RunTimeVal(NullType) {};

    RunTimeVal* clone() override;

    size_t getSize() override { return sizeof(NullVal); };
    size_t getAlignment() override { return alignof(NullVal); }

    std::string getString() override {
        return "<Null>";
    }
};

class NumVal : public RunTimeVal {
public:
    double num;
    NumVal(double number): RunTimeVal(NumType), num(number) {};

    RunTimeVal* clone() override;
    void setValue(RunTimeVal* val) override {
        num = static_cast<NumVal*>(val)->num;
    }

    size_t getSize() override { return sizeof(NumVal); };
    size_t getAlignment() override { return alignof(NumVal); };

    std::string getString() override {
        return std::to_string(num);
    };
};

// deprecated
class StringVal : public RunTimeVal {
public:
    std::string str;
    StringVal(std::string stri): RunTimeVal(StringType), str(std::move(stri)) {};

    RunTimeVal* clone() override;
    void setValue(RunTimeVal* val) override {
        str = dynamic_cast<StringVal*>(val)->str;
    }

    size_t getSize() override { return sizeof(StringVal); };
    size_t getAlignment() override { return alignof(StringVal); };

    std::string getString() override {
        return "\"" + str + "\"";
    };
};

class CharVal : public RunTimeVal {
public:
    char ch;
    CharVal(char cha): RunTimeVal(CharType), ch(cha) {};

    RunTimeVal* clone() override;

    size_t getSize() override { return sizeof(CharVal); };
    size_t getAlignment() override { return alignof(CharVal); };

    std::string getString() override {
        std::string str;
        str.push_back(ch);
        return str;
    };
};

class BoolVal : public RunTimeVal {
public:
    bool boolean;
    BoolVal(bool boolea): RunTimeVal(BoolType), boolean(boolea) {};

    RunTimeVal* clone() override;
    void setValue(RunTimeVal* val) override {
        boolean = dynamic_cast<BoolVal*>(val)->boolean;
    }

    size_t getSize() override { return sizeof(BoolVal); };
    size_t getAlignment() override { return alignof(BoolVal); };
    
    std::string getString() override {
        if(boolean) return "true";
        else return "false";
    };
};

class LambdaVal : public RunTimeVal {
public:
    std::vector<std::string> params;
    std::vector<Statement*> stmts;
    std::unordered_map<std::string, RunTimeVal*> captured;
    std::string lambda_uuid;

    LambdaVal(std::vector<std::string> parameters,
        std::vector<Statement*> statements,
        std::unordered_map<std::string, RunTimeVal*> captured_vals):
        RunTimeVal(LambdaType), params(std::move(parameters)), stmts(std::move(statements)),
        captured(captured_vals) {
        boost::uuids::random_generator gen;
        boost::uuids::uuid uuid = gen();
        lambda_uuid = boost::uuids::to_string(uuid);
    }; 

    void cleanUpChildren(std::pmr::synchronized_pool_resource& target_pool) override {
        for(auto& [name, val] : captured){
            if(val) deallocateVal(target_pool, &val);
        }
        captured.clear();
    };

    void markChildren() override {
        for(auto& [name, val] : captured){
            if(val) val->mark();
        }
    };
    void unMarkChildren() override {
        for(auto& [name, val] : captured){
            if(val) val->unMark();
        }
    };

    size_t getSize() override { return sizeof(LambdaVal); };
    size_t getAlignment() override { return alignof(LambdaVal); };

    RunTimeVal* clone() override;
    void setValue(RunTimeVal* val) override {
        auto real_val = dynamic_cast<LambdaVal*>(val);
        params = real_val->params;
        stmts = real_val->stmts;
        captured = real_val->captured;
    }

    std::string getString() override {
        return "<Lambda>";
    };
};

class ArrayVal : public RunTimeVal {
public:
    std::vector<RunTimeVal*> vals;

    ArrayVal(std::vector<RunTimeVal*> values):
        RunTimeVal(ArrayType), vals(std::move(values)) {};
    std::string getString() override {
        std::string str = "[ ";
        for(auto val = vals.begin(); val != vals.end(); val++){
            std::string strr = (*val)->getString();
            str += strr;
            if(val != vals.end()){
                str += ", ";
            }
        }
        if(str != "[ "){
            str.erase(str.size() - 2, 1);
        }
        str += "]";
        return str;
    }


    size_t getSize() override { return sizeof(ArrayVal); };
    size_t getAlignment() override { return alignof(ArrayVal); };
     
    void markChildren() override {
        for(auto& val : vals){
            if(val) val->mark();
        }
    };
    void unMarkChildren() override {
        for(auto& val : vals){
            if(val) val->unMark();
        }
    };

    void setValue(RunTimeVal* val) override {
        vals = dynamic_cast<ArrayVal*>(val)->vals;
    }
    RunTimeVal* clone() override;
    
};

class ObjectVal : public RunTimeVal {
public:
    std::unordered_map<std::string, RunTimeVal*> vals;

    ObjectVal(std::unordered_map<std::string, RunTimeVal*> values):
        RunTimeVal(StructType), vals(std::move(values)) {};
    std::string getString() override;

    size_t getSize() override { return sizeof(ObjectVal); };
    size_t getAlignment() override { return alignof(ObjectVal); };

    void markChildren() override { 
        for(auto& [name, val] : vals){
            if(val) val->mark();
        }
    };
    void unMarkChildren() override {
        for(auto& [name, val] : vals){
            if(val) val->unMark();
        }
    };

    void setValue(RunTimeVal* val) override {
        vals = dynamic_cast<ObjectVal*>(val)->vals;
    }

    RunTimeVal* clone() override;
};

class ReturnVal : public RunTimeVal {
public:
    RunTimeVal* val;

    size_t getSize() override { return sizeof(ReturnVal); };
    size_t getAlignment() override { return alignof(ReturnVal); };

    void markChildren() override { if(val) val->mark(); };
    void unMarkChildren() override { if(val) val->unMark(); };

    ReturnVal(RunTimeVal* value): RunTimeVal(ReturnType), val(std::move(value)) {};

    RunTimeVal* clone() override;
};

class BreakVal : public RunTimeVal {
public:
    BreakVal(): RunTimeVal(BreakType) {};

    size_t getSize() override { return sizeof(BreakVal); };
    size_t getAlignment() override { return alignof(BreakVal); };

    RunTimeVal* clone() override;
};

class ContinueVal : public RunTimeVal {
public:
    ContinueVal(): RunTimeVal(ContinueType) {};

    size_t getSize() override { return sizeof(ContinueVal); };
    size_t getAlignment() override { return alignof(ContinueVal); };

    RunTimeVal* clone() override;
};

// not copyable, copying will result in shared_from_this()
class NativeFunctionVal : public RunTimeVal {
public:
    using FuncType = std::function<RunTimeVal*(std::vector<RunTimeVal*>&, SigmaInterpreter*)>;    
    FuncType func;
    std::vector<ArgInformation> arg_types;

    NativeFunctionVal(FuncType functio,
        std::vector<ArgInformation> arg_types_arg): RunTimeVal(NativeFunctionType),
        func(std::move(functio)), arg_types(arg_types_arg) {};

    size_t getSize() override { return sizeof(NativeFunctionVal); };
    size_t getAlignment() override { return alignof(NativeFunctionVal); };
    size_t calculateMinimumArgNumber(){
        size_t result = 0;
        for(auto& arg_type : arg_types){
            if(!arg_type.optional){
                result++;
            }
        }
        return result;
    }
    size_t calculateMaximumArgNumber(){
        return arg_types.size();
    }

    RunTimeVal* clone() override;
};

class RefrenceVal : public RunTimeVal {
public:
    RunTimeVal** val;
    RunTimeVal* actual_v;

    RefrenceVal(RunTimeVal** value): RunTimeVal(RefrenceType), val(value), actual_v(*value) {};

    size_t getSize() override { return sizeof(RefrenceVal); };
    size_t getAlignment() override { return alignof(RefrenceVal); };

    void markChildren() override { if(actual_v) actual_v->mark(); };
    void unMarkChildren() override { if(actual_v) actual_v->unMark(); };

    RunTimeVal* clone() override;
};

class BinaryVal : public RunTimeVal {
public:
    std::vector<unsigned char> binary_data;

    BinaryVal(std::vector<unsigned char> d): RunTimeVal(BinaryType), binary_data(d) {};

    size_t getSize() override { return sizeof(BinaryVal); };
    size_t getAlignment() override { return alignof(BinaryVal); };

    RunTimeVal* clone() override;
    std::string getString() override;
};

class HtmlElementVal : public RunTimeVal {
public:
    HTMLTag* target_tag;

    size_t getSize() override { return sizeof(HtmlElementVal); };
    size_t getAlignment() override { return alignof(HtmlElementVal); };

    HtmlElementVal(HTMLTag* tagg): RunTimeVal(HtmlType), target_tag(tagg) {}; 
    RunTimeVal* clone() override { return {}; };
};

class RunTimeMemory {
public:
    static std::pmr::synchronized_pool_resource pool;
    static std::mutex pool_mut;
};

class RunTimeFactory {
public:
    static std::vector<RunTimeVal*>* target_alloc_vec;

    template<typename ValType, typename ...ArgsType>
    static ValType* makeVal(ArgsType... args) {
        std::lock_guard<std::mutex> lock(RunTimeMemory::pool_mut);
        void* mem = RunTimeMemory::pool.allocate(sizeof(ValType), alignof(ValType));
        ValType* obj = new(mem)ValType(std::forward<ArgsType>(args)...);
        target_alloc_vec->push_back(obj);
        return obj;
    };
    template<typename ValType>
    static void freeVal(ValType** ptr){
        std::lock_guard<std::mutex> lock(RunTimeMemory::pool_mut);
        RunTimeMemory::pool.deallocate(*ptr, sizeof(ValType), alignof(ValType));
        *ptr = nullptr;
    }
    static NumVal* makeNum(double num);
    static StringVal* makeString(std::string str);
    static CharVal* makeChar(char ch);
    static ArrayVal* makeArray(std::vector<RunTimeVal*> vec);
    static ObjectVal* makeStruct(std::unordered_map<std::string,
         RunTimeVal*> vals);
    static ReturnVal* makeReturn(RunTimeVal* val);
    static BreakVal* makeBreak();
    static ContinueVal* makeContinue();
    static BoolVal* makeBool(bool boolean);
    static LambdaVal* makeLambda(std::vector<std::string> params,
        std::vector<Statement*> stmts, std::unordered_map<std::string, 
        RunTimeVal*> captured);
    static NativeFunctionVal* makeNativeFunction(
        NativeFunctionVal::FuncType func, std::vector<ArgInformation> arg_information
    );
    static RefrenceVal* makeRefrence(
        RunTimeVal** val
    );
    static BinaryVal* makeBinary(
        std::vector<unsigned char> d
    );
    static HtmlElementVal* makeHtmlElement(HTMLTag* tag);
};
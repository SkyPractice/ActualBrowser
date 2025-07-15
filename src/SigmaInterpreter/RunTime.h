#pragma once
#include <functional>
#include <memory>
#include <string>
#include <sys/cdefs.h>
#include <vector>
#include "SigmaAst.h"
#include <unordered_map>
#include <memory_resource>

enum RunTimeValType {
    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType, ReturnType,
    BreakType, ContinueType, NativeFunctionType
};

class RunTimeVal {
public:
    RunTimeValType type;
    RunTimeVal(RunTimeValType t): type(t) {};

    virtual ~RunTimeVal() {};
    virtual std::string getString() { return ""; };
};

class NumVal : public RunTimeVal {
public:
    double num;
    NumVal(double number): RunTimeVal(NumType), num(number) {};

    std::string getString() override {
        return std::to_string(num);
    };
};

// deprecated
class StringVal : public RunTimeVal {
public:
    std::string str;
    StringVal(std::string stri): RunTimeVal(StringType), str(stri) {};

    std::string getString() override {
        return str;
    };
};

class CharVal : public RunTimeVal {
public:
    char ch;
    CharVal(char cha): RunTimeVal(CharType), ch(cha) {};

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

    std::string getString() override {
        if(boolean) return "true";
        else return "false";
    };
};

class LambdaVal : public RunTimeVal {
public:
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Statement>> stmts;

    LambdaVal(std::vector<std::string> parameters,
        std::vector<std::shared_ptr<Statement>> statements):
        RunTimeVal(LambdaType), params(parameters), stmts(statements) {}; 

    std::string getString() override {
        return "<Lambda>";
    };
};

class ArrayVal : public RunTimeVal {
public:
    std::vector<std::shared_ptr<RunTimeVal>> vals;

    ArrayVal(std::vector<std::shared_ptr<RunTimeVal>> values):
        RunTimeVal(ArrayType), vals(values) {};
    std::string getString() override {
        std::string str = "[ ";
        for(auto val : vals){
            std::string strr = val->getString();
            str += strr;
            str += " ";
        }
        return str;
    }
};

class StructVal : public RunTimeVal {
public:
    std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> vals;

    StructVal(std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> values):
        RunTimeVal(StructType), vals(values) {};
    std::string getString() override {
        return "<struct>";
    }
};

class ReturnVal : public RunTimeVal {
public:
    std::shared_ptr<RunTimeVal> val;

    ReturnVal(std::shared_ptr<RunTimeVal> value): RunTimeVal(ReturnType), val(value) {};
};

class BreakVal : public RunTimeVal {
public:
    BreakVal(): RunTimeVal(BreakType) {};
};

class ContinueVal : public RunTimeVal {
public:
    ContinueVal(): RunTimeVal(ContinueType) {};
};

class NativeFunctionVal : public RunTimeVal {
public:
    using FuncType = std::function<std::shared_ptr<RunTimeVal>(std::vector<std::shared_ptr<RunTimeVal>>)>;    
    FuncType func;

    NativeFunctionVal(FuncType functio): RunTimeVal(NativeFunctionType), func(functio) {};
};

class RunTimeMemory {
public:
    static std::pmr::unsynchronized_pool_resource pool;
};

class RunTimeFactory {
public:
    template<typename ValType, typename ...ArgsType>
    static std::shared_ptr<ValType> makeVal(ArgsType... args) {
        void* mem = RunTimeMemory::pool.allocate(sizeof(ValType), alignof(ValType));
        ValType* obj = new(mem)ValType(args...);

        return std::shared_ptr<ValType>(obj, [&](ValType* val) {
            val->~ValType(); RunTimeMemory::pool.deallocate(val, sizeof(ValType));
        });
    };
    static std::shared_ptr<NumVal> makeNum(double num);
    static std::shared_ptr<StringVal> makeString(std::string str);
    static std::shared_ptr<ArrayVal> makeArray(std::vector<std::shared_ptr<RunTimeVal>> vec);
    static std::shared_ptr<StructVal> makeStruct(std::unordered_map<std::string,
         std::shared_ptr<RunTimeVal>> vals);
    static std::shared_ptr<ReturnVal> makeReturn(std::shared_ptr<RunTimeVal> val);
    static std::shared_ptr<BreakVal> makeBreak();
    static std::shared_ptr<ContinueVal> makeContinue();
    static std::shared_ptr<BoolVal> makeBool(bool boolean);
    static std::shared_ptr<LambdaVal> makeLambda(std::vector<std::string> params,
        std::vector<std::shared_ptr<Statement>> stmts);
    static std::shared_ptr<NativeFunctionVal> makeNativeFunction(
        NativeFunctionVal::FuncType func
    );
};
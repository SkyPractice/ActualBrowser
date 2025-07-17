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
    BreakType, ContinueType, NativeFunctionType, RefrenceType
};

class RunTimeVal {
public:
    RunTimeValType type;
    RunTimeVal(RunTimeValType t): type(t) {};

    virtual std::shared_ptr<RunTimeVal> clone() = 0;
    virtual ~RunTimeVal() {};
    virtual std::string getString() { return ""; };
};

class NumVal : public RunTimeVal {
public:
    double num;
    NumVal(double number): RunTimeVal(NumType), num(number) {};

    std::shared_ptr<RunTimeVal> clone() override;

    std::string getString() override {
        return std::to_string(num);
    };
};

// deprecated
class StringVal : public RunTimeVal {
public:
    std::string str;
    StringVal(std::string stri): RunTimeVal(StringType), str(std::move(stri)) {};

    std::shared_ptr<RunTimeVal> clone() override;

    std::string getString() override {
        return "\"" + str + "\"";
    };
};

class CharVal : public RunTimeVal {
public:
    char ch;
    CharVal(char cha): RunTimeVal(CharType), ch(cha) {};

    std::shared_ptr<RunTimeVal> clone() override;

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

    std::shared_ptr<RunTimeVal> clone() override;

    std::string getString() override {
        if(boolean) return "true";
        else return "false";
    };
};

class LambdaVal : public RunTimeVal {
public:
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Statement>> stmts;
    std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> captured;

    LambdaVal(std::vector<std::string> parameters,
        std::vector<std::shared_ptr<Statement>> statements,
        std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> captured_vals = {}):
        RunTimeVal(LambdaType), params(std::move(parameters)), stmts(std::move(statements)),
        captured(captured_vals) {}; 

    std::shared_ptr<RunTimeVal> clone() override;

    std::string getString() override {
        return "<Lambda>";
    };
};

class ArrayVal : public RunTimeVal {
public:
    std::vector<std::shared_ptr<RunTimeVal>> vals;

    ArrayVal(std::vector<std::shared_ptr<RunTimeVal>> values):
        RunTimeVal(ArrayType), vals(std::move(values)) {};
    std::string getString() override {
        std::string str = "[ ";
        for(auto val : vals){
            std::string strr = val->getString();
            str += strr;
            str += ", ";
        }
        str += "]";
        return str;
    }

    std::shared_ptr<RunTimeVal> clone() override;
};

class StructVal : public RunTimeVal {
public:
    std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> vals;

    StructVal(std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> values):
        RunTimeVal(StructType), vals(std::move(values)) {};
    std::string getString() override {
        return "<struct>";
    }

    std::shared_ptr<RunTimeVal> clone() override;
};

class ReturnVal : public RunTimeVal {
public:
    std::shared_ptr<RunTimeVal> val;

    ReturnVal(std::shared_ptr<RunTimeVal> value): RunTimeVal(ReturnType), val(std::move(value)) {};

    std::shared_ptr<RunTimeVal> clone() override;
};

class BreakVal : public RunTimeVal {
public:
    BreakVal(): RunTimeVal(BreakType) {};

    std::shared_ptr<RunTimeVal> clone() override;
};

class ContinueVal : public RunTimeVal {
public:
    ContinueVal(): RunTimeVal(ContinueType) {};

    std::shared_ptr<RunTimeVal> clone() override;
};

class NativeFunctionVal : public RunTimeVal {
public:
    using FuncType = std::function<std::shared_ptr<RunTimeVal>(std::vector<std::shared_ptr<RunTimeVal>>)>;    
    FuncType func;

    NativeFunctionVal(FuncType functio): RunTimeVal(NativeFunctionType), func(std::move(functio)) {};

    std::shared_ptr<RunTimeVal> clone() override;
};

class RefrenceVal : public RunTimeVal {
public:
    std::shared_ptr<RunTimeVal>* val;

    RefrenceVal(std::shared_ptr<RunTimeVal>* value): RunTimeVal(RefrenceType), val(value) {};

    std::shared_ptr<RunTimeVal> clone() override;
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
        ValType* obj = new(mem)ValType(std::forward<ArgsType>(args)...);

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
        std::vector<std::shared_ptr<Statement>> stmts, std::unordered_map<std::string, 
        std::shared_ptr<RunTimeVal>> captured = {});
    static std::shared_ptr<NativeFunctionVal> makeNativeFunction(
        NativeFunctionVal::FuncType func
    );
    static std::shared_ptr<RefrenceVal> makeRefrence(
        std::shared_ptr<RunTimeVal>* val
    );
};
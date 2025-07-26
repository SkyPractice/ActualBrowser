#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <sys/cdefs.h>
#include <vector>
#include "SigmaAst.h"
#include <unordered_map>
#include <memory_resource>
#include <iostream>
#include <atomic>
#include "../Interpreter/Ast.h"

enum RunTimeValType {
    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType, ReturnType,
    BreakType, ContinueType, NativeFunctionType, RefrenceType, BinaryType, HtmlType
};

class RunTimeVal {
public:
    RunTimeValType type;
    RunTimeVal(RunTimeValType t): type(t) {};

    virtual RunTimeVal* clone() = 0;
    virtual ~RunTimeVal() {};
    virtual void setValue(RunTimeVal* val) {};
    virtual std::string getString() { return ""; };
};

class NumVal : public RunTimeVal {
public:
    std::atomic<double> num;
    std::mutex mut;
    NumVal(double number): RunTimeVal(NumType), num(number) {};

    RunTimeVal* clone() override;
    void setValue(RunTimeVal* val) override {
        num.store(static_cast<NumVal*>(val)->num);
    }

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

    std::string getString() override {
        return "\"" + str + "\"";
    };
};

class CharVal : public RunTimeVal {
public:
    char ch;
    CharVal(char cha): RunTimeVal(CharType), ch(cha) {};

    RunTimeVal* clone() override;


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

    LambdaVal(std::vector<std::string> parameters,
        std::vector<Statement*> statements,
        std::unordered_map<std::string, RunTimeVal*> captured_vals):
        RunTimeVal(LambdaType), params(std::move(parameters)), stmts(std::move(statements)),
        captured(captured_vals) {}; 

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
        for(auto val : vals){
            std::string strr = val->getString();
            str += strr;
            str += ", ";
        }
        str += "]";
        return str;
    }

    void setValue(RunTimeVal* val) override {
        vals = dynamic_cast<ArrayVal*>(val)->vals;
    }
    RunTimeVal* clone() override;
    
};

class StructVal : public RunTimeVal {
public:
    std::unordered_map<std::string, RunTimeVal*> vals;

    StructVal(std::unordered_map<std::string, RunTimeVal*> values):
        RunTimeVal(StructType), vals(std::move(values)) {};
    std::string getString() override {
        return "<struct>";
    }

    void setValue(RunTimeVal* val) override {
        vals = dynamic_cast<StructVal*>(val)->vals;
    }

    RunTimeVal* clone() override;
};

class ReturnVal : public RunTimeVal {
public:
    RunTimeVal* val;

    ReturnVal(RunTimeVal* value): RunTimeVal(ReturnType), val(std::move(value)) {};

    RunTimeVal* clone() override;
};

class BreakVal : public RunTimeVal {
public:
    BreakVal(): RunTimeVal(BreakType) {};

    RunTimeVal* clone() override;
};

class ContinueVal : public RunTimeVal {
public:
    ContinueVal(): RunTimeVal(ContinueType) {};

    RunTimeVal* clone() override;
};

// not copyable, copying will result in shared_from_this()
class NativeFunctionVal : public RunTimeVal {
public:
    using FuncType = std::function<RunTimeVal*(std::vector<RunTimeVal*>&)>;    
    FuncType func;

    NativeFunctionVal(FuncType functio): RunTimeVal(NativeFunctionType), func(std::move(functio)) {};

    RunTimeVal* clone() override;
};

class RefrenceVal : public RunTimeVal {
public:
    RunTimeVal** val;
    RunTimeVal* actual_v;

    RefrenceVal(RunTimeVal** value): RunTimeVal(RefrenceType), val(value), actual_v(*value) {};

    RunTimeVal* clone() override;
};

class BinaryVal : public RunTimeVal {
public:
    std::vector<unsigned char> binary_data;

    BinaryVal(std::vector<unsigned char> d): RunTimeVal(BinaryType), binary_data(d) {};
    RunTimeVal* clone() override;
    std::string getString() override;
};

class HtmlElementVal : public RunTimeVal {
public:
    HTMLTag* target_tag;

    HtmlElementVal(HTMLTag* tagg): RunTimeVal(HtmlType), target_tag(tagg) {}; 
    RunTimeVal* clone() override { return {}; };
};

class RunTimeMemory {
public:
    static std::pmr::unsynchronized_pool_resource pool;
};

class RunTimeFactory {
public:
    template<typename ValType, typename ...ArgsType>
    static ValType* makeVal(ArgsType... args) {
        void* mem = RunTimeMemory::pool.allocate(sizeof(ValType), alignof(ValType));
        ValType* obj = new(mem)ValType(std::forward<ArgsType>(args)...);

        return obj;
    };
    template<typename ValType>
    static void freeVal(ValType** ptr){
        RunTimeMemory::pool.deallocate(*ptr, sizeof(ValType), alignof(ValType));
        *ptr = nullptr;
    }
    static NumVal* makeNum(double num);
    static StringVal* makeString(std::string str);
    static ArrayVal* makeArray(std::vector<RunTimeVal*> vec);
    static StructVal* makeStruct(std::unordered_map<std::string,
         RunTimeVal*> vals);
    static ReturnVal* makeReturn(RunTimeVal* val);
    static BreakVal* makeBreak();
    static ContinueVal* makeContinue();
    static BoolVal* makeBool(bool boolean);
    static LambdaVal* makeLambda(std::vector<std::string> params,
        std::vector<Statement*> stmts, std::unordered_map<std::string, 
        RunTimeVal*> captured);
    static NativeFunctionVal* makeNativeFunction(
        NativeFunctionVal::FuncType func
    );
    static RefrenceVal* makeRefrence(
        RunTimeVal** val
    );
    static BinaryVal* makeBinary(
        std::vector<unsigned char> d
    );
    static HtmlElementVal* makeHtmlElement(HTMLTag* tag);
};
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <sys/cdefs.h>
#include <vector>
#include "SigmaAst.h"
#include <unordered_map>
#include <memory_resource>
#include "../Interpreter/Ast.h"

enum RunTimeValType {
    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType, ReturnType,
    BreakType, ContinueType, NativeFunctionType, RefrenceType, BinaryType, HtmlType
};

class RunTimeVal {
public:
    RunTimeValType type;
    RunTimeVal(RunTimeValType t): type(t) {};

    virtual std::shared_ptr<RunTimeVal> clone() = 0;
    virtual ~RunTimeVal() {};
    virtual void setValue(std::shared_ptr<RunTimeVal> val) {};
    virtual std::string getString() { return ""; };
};

class NumVal : public RunTimeVal {
public:
    double num;
    NumVal(double number): RunTimeVal(NumType), num(number) {};

    std::shared_ptr<RunTimeVal> clone() override;

    void setValue(std::shared_ptr<RunTimeVal> val) override {
        num = std::dynamic_pointer_cast<NumVal>(val)->num;
    };

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
    void setValue(std::shared_ptr<RunTimeVal> val) override {
        str = std::dynamic_pointer_cast<StringVal>(val)->str;
    }

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
    void setValue(std::shared_ptr<RunTimeVal> val) override {
        boolean = std::dynamic_pointer_cast<BoolVal>(val)->boolean;
    }

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
        std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> captured_vals):
        RunTimeVal(LambdaType), params(std::move(parameters)), stmts(std::move(statements)),
        captured(captured_vals) {}; 

    std::shared_ptr<RunTimeVal> clone() override;
    void setValue(std::shared_ptr<RunTimeVal> val) override {
        auto real_val = std::dynamic_pointer_cast<LambdaVal>(val);
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

    void setValue(std::shared_ptr<RunTimeVal> val) override {
        vals = std::dynamic_pointer_cast<ArrayVal>(val)->vals;
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

    void setValue(std::shared_ptr<RunTimeVal> val) override {
        vals = std::dynamic_pointer_cast<StructVal>(val)->vals;
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

// not copyable, copying will result in shared_from_this()
class NativeFunctionVal : public RunTimeVal, public std::enable_shared_from_this<NativeFunctionVal> {
public:
    using FuncType = std::function<std::shared_ptr<RunTimeVal>(std::vector<std::shared_ptr<RunTimeVal>>&)>;    
    FuncType func;

    NativeFunctionVal(FuncType functio): RunTimeVal(NativeFunctionType), func(std::move(functio)) {};

    std::shared_ptr<RunTimeVal> clone() override;
};

class RefrenceVal : public RunTimeVal {
public:
    std::shared_ptr<RunTimeVal>* val;
    std::shared_ptr<RunTimeVal> actual_v;

    RefrenceVal(std::shared_ptr<RunTimeVal>* value): RunTimeVal(RefrenceType), val(value), actual_v(*value) {};

    std::shared_ptr<RunTimeVal> clone() override;
};

class BinaryVal : public RunTimeVal {
public:
    std::vector<unsigned char> binary_data;

    BinaryVal(std::vector<unsigned char> d): RunTimeVal(BinaryType), binary_data(d) {};
    std::shared_ptr<RunTimeVal> clone() override;
    std::string getString() override;
};

class HtmlElementVal : public RunTimeVal {
public:
    std::shared_ptr<HTMLTag> target_tag;

    HtmlElementVal(std::shared_ptr<HTMLTag> tagg): RunTimeVal(HtmlType), target_tag(tagg) {}; 
    std::shared_ptr<RunTimeVal> clone() override { return {}; };
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
        std::shared_ptr<RunTimeVal>> captured);
    static std::shared_ptr<NativeFunctionVal> makeNativeFunction(
        NativeFunctionVal::FuncType func
    );
    static std::shared_ptr<RefrenceVal> makeRefrence(
        std::shared_ptr<RunTimeVal>* val
    );
    static std::shared_ptr<BinaryVal> makeBinary(
        std::vector<unsigned char> d
    );
    static std::shared_ptr<HtmlElementVal> makeHtmlElement(std::shared_ptr<HTMLTag> tag);
};
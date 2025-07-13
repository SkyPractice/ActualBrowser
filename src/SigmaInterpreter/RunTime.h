#pragma once
#include <memory>
#include <string>
#include <sys/cdefs.h>
#include <vector>
#include "SigmaAst.h"
#include <unordered_map>

enum RunTimeValType {
    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType, ReturnType,
    BreakType, ContinueType
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
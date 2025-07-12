#pragma once
#include <string>
#include <vector>
#include "SigmaAst.h"
#include <unordered_map>

enum RunTimeValType {
    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType
};

class RunTimeVal {
public:
    RunTimeValType type;
    RunTimeVal(RunTimeValType t): type(t) {};

    virtual ~RunTimeVal() {};
};

class NumVal : public RunTimeVal {
public:
    double num;
    NumVal(double number): RunTimeVal(NumType), num(number) {};
};

class StringVal : public RunTimeVal {
public:
    std::string str;
    StringVal(std::string stri): RunTimeVal(StringType), str(stri) {};
};

class CharVal : public RunTimeVal {
public:
    char ch;
    CharVal(char cha): RunTimeVal(CharType), ch(cha) {};
};

class BoolVal : public RunTimeVal {
public:
    bool boolean;
    BoolVal(bool boolea): RunTimeVal(BoolType), boolean(boolea) {};
};

class LambdaVal : public RunTimeVal {
public:
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Statement>> stmts;

    LambdaVal(std::vector<std::string> parameters,
        std::vector<std::shared_ptr<Statement>> statements):
        RunTimeVal(LambdaType), params(parameters), stmts(statements) {};
};

class ArrayVal : public RunTimeVal {
public:
    std::vector<std::shared_ptr<RunTimeVal>> vals;

    ArrayVal(std::vector<std::shared_ptr<RunTimeVal>> values):
        RunTimeVal(ArrayType), vals(values) {};
};

class StructVal : public RunTimeVal {
public:
    std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> vals;

    StructVal(std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> values):
        RunTimeVal(StructType), vals(values) {};
};
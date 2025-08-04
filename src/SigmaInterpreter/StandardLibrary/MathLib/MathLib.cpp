#include "MathLib.h"
#include <stdexcept>
#include <unordered_map>

RunTimeVal* MathLib::getStruct(){
    std::unordered_map<std::string, RunTimeVal*> func_map = {
        {"sqrt", RunTimeFactory::makeNativeFunction(&MathLib::mathSqrt, {{"number", NumType}})},
        {"pow", RunTimeFactory::makeNativeFunction(&MathLib::mathPow, {{"number", NumType}, {"exponent", NumType}})},
        {"rand", RunTimeFactory::makeNativeFunction(&MathLib::mathRand, {})},
        {"PI", RunTimeFactory::makeNum(M_PI)}
    };

    return RunTimeFactory::makeStruct(std::move(func_map));
};

RunTimeVal* MathLib::mathSqrt(COMPILED_FUNC_ARGS) {
    if(args[0]->type != NumType)
        throw std::runtime_error("sqrt excepts args 0 to be a number");

    return RunTimeFactory::makeNum(sqrt(static_cast<NumVal*>(args[0])->num));
};
RunTimeVal* MathLib::mathPow(COMPILED_FUNC_ARGS) {
    if(args[0]->type != NumType)
        throw std::runtime_error("pow excepts args 0 to be a number");

    return RunTimeFactory::makeNum(pow(static_cast<NumVal*>(args[0])->num, 
        static_cast<NumVal*>(args[1])->num));
};
RunTimeVal* MathLib::mathRand(COMPILED_FUNC_ARGS) {
    return RunTimeFactory::makeNum(rand());
};
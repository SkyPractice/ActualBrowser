#include "GCLib.h"
#include <unordered_map>
#include "../../GarbageCollector/GarbageCollector.h"
#include "../../SigmaInterpreter.h"

StructVal* GCLib::getStruct() {
    std::unordered_map<std::string, RunTimeVal*> native_funcs = {
        {"mark", RunTimeFactory::makeNativeFunction(&GCLib::mark)},
        {"sweep", RunTimeFactory::makeNativeFunction(&GCLib::sweep)}
    };

    return RunTimeFactory::makeStruct(native_funcs);
};

RunTimeVal* GCLib::mark(COMPILED_FUNC_ARGS) {
    auto vals = interpreter->current_scope->flatten_as_vec();
    GarbageCollector::mark(vals);
    return nullptr;
};
RunTimeVal* GCLib::sweep(COMPILED_FUNC_ARGS) { 
    GarbageCollector::sweep(RunTimeMemory::pool);
    return nullptr; 
};
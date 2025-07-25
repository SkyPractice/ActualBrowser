#include "../SigmaInterpreter.h"
#include "StdLib.h"

std::shared_ptr<Scope> StdLib::current_calling_scope = nullptr;

void StdLibInitializer::declareStdLibMemberInScope(std::string struct_name, std::shared_ptr<RunTimeVal> struct_val,
    std::shared_ptr<Scope> target_scope) {
    target_scope->declareVar(struct_name, { struct_val, true });
};

void StdLib::addValToStruct(std::shared_ptr<StructVal> target_struct, std::string name,
    std::shared_ptr<RunTimeVal> val) {
    target_struct->vals.insert({name, val});
};
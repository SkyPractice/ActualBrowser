#include "../SigmaInterpreter.h"
#include "StdLib.h"

std::shared_ptr<Scope> StdLib::current_calling_scope = nullptr;

void StdLibInitializer::declareStdLibMemberInScope(std::string struct_name, RunTimeVal* struct_val,
    Scope* target_scope) {
    target_scope->declareVar(struct_name, { struct_val, true });
};

void StdLib::addValToStruct(ObjectVal* target_struct, std::string name,
    RunTimeVal* val) {
    target_struct->vals.insert({name, val});
};
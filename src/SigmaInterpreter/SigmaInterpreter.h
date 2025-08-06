#pragma once
#include "RunTime.h"
#include "SigmaAst.h"
#include "Util/Permissions/Permissions.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Scope.h"
#include "GarbageCollectionRestricter.h"

struct DOMAccessor;

struct StructDecleration {
    std::vector<VariableDecleration*> variable_decls = {};
    LambdaExpression* constructor = nullptr;
};

class SigmaInterpreter {
public:
    GCRestricter garbageCollectionRestricter;

    std::string this_str = "this";
    Gtk::Window* current_window;
    PermissionContainer perms;

    std::string doc_name;

    DOMAccessor* accessor;

    static std::unordered_set<RunTimeValType> non_copyable_types;
    static std::unordered_map<RunTimeValType, std::string> type_to_string_table;

    std::shared_ptr<Scope> current_scope;
    std::unordered_set<RunTimeValType> break_out_types = {
        BreakType, ContinueType, ReturnType
    };
    std::unordered_map<std::string, StructDecleration> struct_decls;

    SigmaInterpreter();
    SigmaInterpreter(Gtk::Window* wind){
        current_window = wind;
    };
    
    void initialize();
    void cleanUpBeforeExecuting();

    RunTimeVal* evaluate(Statement* stmt);
    RunTimeVal* evaluateProgram(SigmaProgram* program);
    RunTimeVal* evaluateBinaryExpression(BinaryExpression* expr);
    RunTimeVal* evaluateNumericBinaryExpression(NumVal* left,
        NumVal* right, std::string operat);
    RunTimeVal* evaluateBooleanBinaryExpression(BoolVal* left,
        BoolVal* right, std::string op);
    RunTimeVal* evaluateStringBinaryExpression(StringVal* left,
        StringVal* right, std::string op);
    RunTimeVal* evaluateFunctionCallExpression(FunctionCallExpression* expr);
    RunTimeVal* evaluateIndexAccessExpression(IndexAccessExpression* expr);
    RunTimeVal* evaluateMemberAccessExpression(MemberAccessExpression* expr);
    RunTimeVal* evaluateIncrementExpression(IncrementExpression* expr);
    RunTimeVal* evaluateDecrementExpression(DecrementExpression* expr);
    RunTimeVal* evaluateNegativeExpression(NegativeExpression* expr);
    RunTimeVal* evaluateJsObjectExpression(JsObjectExpression* expr);

    // Shadowing Is Allowed
    RunTimeVal* evaluateVariableDeclStatement(VariableDecleration* decl);
    RunTimeVal* evaluateVariableReInitStatement(VariableReInit* decl);
    RunTimeVal* evaluateIfStatement(IfStatement* if_Statement);
    RunTimeVal* evaluateWhileLoopStatement(WhileLoopStatement* while_loop);
    RunTimeVal* evaluateForLoopStatement(ForLoopStatement* for_loop);
    RunTimeVal* evaluateIndexReInitStatement(IndexReInitStatement* stmt);
    RunTimeVal* evaluateStructDeclStatement(StructDeclerationStatement* stmt);
    RunTimeVal* evaluateMemberReInitStatement(MemberReInitExpression* expr);
    RunTimeVal* evaluateCompoundAssignmentStatement(CompoundAssignmentStatement* stmt);


    RunTimeVal* toString(std::vector<RunTimeVal*>& args);
    RunTimeVal* numIota(std::vector<RunTimeVal*>& args);
    RunTimeVal* clone(std::vector<RunTimeVal*>& args);
    RunTimeVal* getCurrentTimeMillis(std::vector<RunTimeVal*>& args);
    RunTimeVal* getStringASCII(std::vector<RunTimeVal*>& args);
    RunTimeVal* toNum(std::vector<RunTimeVal*>& args);

    
    RunTimeVal* getElementById(std::vector<RunTimeVal*>& args);
    RunTimeVal* setElementInnerHtml(std::vector<RunTimeVal*>& args);
    RunTimeVal* getElementsByClassName(std::vector<RunTimeVal*>& args);
    RunTimeVal* setOnClick(std::vector<RunTimeVal*>& args);

    RunTimeVal* evaluateAnonymousLambdaCall(LambdaVal* lambda, std::vector<RunTimeVal*> args);

    static RunTimeVal* copyIfRecommended(RunTimeVal* val){
        if(non_copyable_types.contains(val->type) || !val->is_l_val)
            return val;
        return val->clone();
    }
    static bool shouldICopy(RunTimeVal* val){
        if(non_copyable_types.contains(val->type) || !val->is_l_val)
            return false;
        return true;
    }

    ObjectVal* getThis() {
        RunTimeVal* target_this = current_scope->getVal(this_str);
        if(target_this->type == StructType)
            return static_cast<ObjectVal*>(target_this);
        return nullptr;
    };

    std::vector<RunTimeVal*> getAccessibleValues();
    void garbageCollectIfNeeded();

};
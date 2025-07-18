#pragma once
#include "RunTime.h"
#include "SigmaAst.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef std::shared_ptr<Statement> Stmt;
typedef std::shared_ptr<Expression> Expr;
typedef std::shared_ptr<RunTimeVal> RunTimeValue;

struct Variable {
    RunTimeValue value;
    bool is_const;
};

class Scope : public std::enable_shared_from_this<Scope>{
public: 
    std::shared_ptr<Scope> parent;
    std::unordered_map<std::string, Variable> variables;

    Scope(std::shared_ptr<Scope> p): parent(p) {};

    std::shared_ptr<Scope> traverse(std::string var_name){
        if(variables.contains(var_name)) return shared_from_this();
        else if (parent) return parent->traverse(var_name);
        return nullptr;
    };

    RunTimeValue& getVal(std::string var_name){
        auto scope = traverse(var_name);
        if(!scope){ throw std::runtime_error("variable " + var_name + " not found"); };
        return scope->variables[var_name].value;
    };

    // shadowing is allowed
    void declareVar(std::string name, Variable val){
        variables[name] = val;
    };

    void reInitVar(std::string name, std::shared_ptr<RunTimeVal> val){
        auto scope = traverse(name);
        if(!scope) { throw std::runtime_error("identifier " + name + " not found"); };
        if(scope->variables[name].is_const) { throw std::runtime_error("Can't ReInitialize A Variable Marked As A Const"); };
        scope->variables[name].value = val;
    }

    std::unordered_map<std::string, RunTimeValue> flatten(){
        std::vector<std::pair<std::string, RunTimeValue>> actual_vec;
        flatten_recursively(actual_vec);

        std::unordered_map<std::string, RunTimeValue> hash_map;

        for(auto& [var_name, var_val] : actual_vec){
            hash_map.insert({var_name, var_val});
        }

        return hash_map;
    }

    void flatten_recursively(std::vector<std::pair<std::string, RunTimeValue>>& vec){
        for(const auto& [var_name, var_val] : variables) {
            vec.push_back({var_name, var_val.value});
        }
        if(parent){
            parent->flatten_recursively(vec);
        }
    }
};

class SigmaInterpreter {
public:
    std::shared_ptr<Scope> current_scope = std::make_shared<Scope>(nullptr);
    std::unordered_map<std::string, std::function<RunTimeValue(std::vector<RunTimeValue>)>> 
        native_functions;
    std::unordered_set<RunTimeValType> break_out_types = {
        BreakType, ContinueType, ReturnType
    };
    std::unordered_map<std::string, std::vector<std::shared_ptr<VariableDecleration>>> struct_decls;

    SigmaInterpreter();
    
    RunTimeValue evaluate(Stmt stmt);
    RunTimeValue evaluateProgram(std::shared_ptr<SigmaProgram> program);
    RunTimeValue evaluateBinaryExpression(std::shared_ptr<BinaryExpression> expr);
    RunTimeValue evaluateNumericBinaryExpression(std::shared_ptr<NumVal> left,
        std::shared_ptr<NumVal> right, std::string operat);
    RunTimeValue evaluateBooleanBinaryExpression(std::shared_ptr<BoolVal> left,
        std::shared_ptr<BoolVal> right, std::string op);
    RunTimeValue evaluateStringBinaryExpression(std::shared_ptr<StringVal> left,
        std::shared_ptr<StringVal> right, std::string op);
    RunTimeValue evaluateFunctionCallExpression(std::shared_ptr<FunctionCallExpression> expr);
    RunTimeValue evaluateIndexAccessExpression(std::shared_ptr<IndexAccessExpression> expr);
    RunTimeValue evaluateMemberAccessExpression(std::shared_ptr<MemberAccessExpression> expr);
    RunTimeValue evaluateIncrementExpression(std::shared_ptr<IncrementExpression> expr);
    RunTimeValue evaluateDecrementExpression(std::shared_ptr<DecrementExpression> expr);
    RunTimeValue evaluateNegativeExpression(std::shared_ptr<NegativeExpression> expr);

    // Shadowing Is Allowed
    RunTimeValue evaluateVariableDeclStatement(std::shared_ptr<VariableDecleration> decl);
    RunTimeValue evaluateVariableReInitStatement(std::shared_ptr<VariableReInit> decl);
    RunTimeValue evaluateIfStatement(std::shared_ptr<IfStatement> if_stmt);
    RunTimeValue evaluateWhileLoopStatement(std::shared_ptr<WhileLoopStatement> while_loop);
    RunTimeValue evaluateForLoopStatement(std::shared_ptr<ForLoopStatement> for_loop);
    RunTimeValue evaluateIndexReInitStatement(std::shared_ptr<IndexReInitStatement> stmt);
    RunTimeValue evaluateStructDeclStatement(std::shared_ptr<StructDeclerationStatement> stmt);
    RunTimeValue evaluateMemberReInitStatement(std::shared_ptr<MemberReInitExpression> expr);
    RunTimeValue evaluateCompoundAssignmentStatement(std::shared_ptr<CompoundAssignmentStatement> stmt);

    static RunTimeValue print(std::vector<RunTimeValue> args);
    static RunTimeValue println(std::vector<RunTimeValue> args);
    static RunTimeValue toString(std::vector<RunTimeValue> args);
    static RunTimeValue numIota(std::vector<RunTimeValue> args);
    static RunTimeValue readFileSync(std::vector<RunTimeValue> args);
    static RunTimeValue writeFileSync(std::vector<RunTimeValue> args);
    static RunTimeValue clone(std::vector<RunTimeValue> args);
    static RunTimeValue input(std::vector<RunTimeValue> args);
    static RunTimeValue getCurrentTimeMillis(std::vector<RunTimeValue> args);
    static RunTimeValue resizeArray(std::vector<RunTimeValue> args);
};
#include "SigmaInterpreter.h"
#include "RunTime.h"
#include "SigmaAst.h"
#include <algorithm>
#include <exception>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include "../Interpreter/Interpreter.h"
#include "../Interpreter/Parser.h"
#include "StandardLibrary/TypeWrappers/StringWrapper.h"
#include "Util/Util.h"
#include "StandardLibrary/TypeWrappers/ArrayWrapper.h"

SigmaInterpreter::SigmaInterpreter(){
    current_window = nullptr;
};


//    NumType, StringType, CharType, BoolType, LambdaType, ArrayType, StructType, ReturnType,
//    BreakType, ContinueType, NativeFunctionType, RefrenceType, BinaryType, HtmlType, AnyType

std::unordered_map<RunTimeValType, std::string> SigmaInterpreter::type_to_string_table = {
    {NumType, "Number"}, {StringType, "String"}, {CharType, "Char"}, 
    {BoolType, "Bool"}, {LambdaType, "Lambda"}, {ArrayType, "Array"},
    {StructType, "Struct"}, {NativeFunctionType, "CompiledFunction"},
    {RefrenceType, "Refrence"}, {BinaryType, "Binary"}, {HtmlType, "HtmlElement"},
    {AnyType, "Any"}
};


std::unordered_set<RunTimeValType> SigmaInterpreter::non_copyable_types = {
        StructType, LambdaType, StringType, ArrayType, BinaryType, NativeFunctionType, HtmlType
};

void SigmaInterpreter::initialize(){
    garbageCollectionRestricter.reset();

    current_scope = std::make_shared<Scope>(nullptr);
    struct_decls.clear();
    
    std::unordered_map<std::string, RunTimeValue> str_vals;
    str_vals.insert({"valueOf", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::toString, this, std::placeholders::_1), {{"val", AnyType}})});
    str_vals.insert({"ord", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::getStringASCII, this, std::placeholders::_1), {{"string", StringType}})});

    std::unordered_map<std::string, RunTimeValue> num_vals;
    num_vals.insert({"valueOf", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::toNum, this, std::placeholders::_1), {{"val", AnyType}})});


    std::unordered_map<std::string, RunTimeValue> obj_vals;

    obj_vals.insert({"ref", RunTimeFactory::makeNativeFunction(
        [this](std::vector<RunTimeVal*>& vals, SigmaInterpreter*) { 
        return RunTimeFactory::makeRefrence(&vals[0]);
        }, {{"val", AnyType}}
    )    
    });
    obj_vals.insert({"valByRef", RunTimeFactory::makeNativeFunction(
        [this](std::vector<RunTimeVal*>& vals, SigmaInterpreter*) { 
            return *static_cast<RefrenceVal*>(vals[0])->val;
        },
    {{"refrence", RefrenceType}})   
    });
    obj_vals.insert({"clone", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::clone, this, std::placeholders::_1), {{"val", AnyType}})});
    
    current_scope->declareVar("String", { RunTimeFactory::makeStruct((str_vals)), true });
    current_scope->declareVar("Number", { RunTimeFactory::makeStruct((num_vals)), true });
    current_scope->declareVar("Object", { RunTimeFactory::makeStruct((obj_vals)), true });

    Util::SigmaInterpreterHelper::initializeStandardLibraries(current_scope);

    ArrayWrapper::initializeWrapper();
    StringWrapper::initializeWrapper();

    garbageCollectionRestricter.registerWrapperTypeFunctions(ArrayWrapper::funcs);
    garbageCollectionRestricter.registerWrapperTypeFunctions(StringWrapper::funcs);
}

void SigmaInterpreter::cleanUpBeforeExecuting() {
    
};

RunTimeValue SigmaInterpreter::evaluate(Stmt stmt) {
    switch (stmt->type) {
        case NumericExpressionType:
            return RunTimeFactory::makeNum(static_cast<NumericExpression*>(stmt)->num);
        case StringExpressionType:
            return StringWrapper::genObject(RunTimeFactory::makeString(
                (static_cast<StringExpression*>(stmt)->str)));
        case BooleanExpressionType:
            return RunTimeFactory::makeBool(static_cast<BoolExpression*>(stmt)->val);
        case LambdaExpressionType:{
            return Util::SigmaInterpreterHelper::evaluateLambda(current_scope, stmt);
        }
        case CharExpressionType:
            return RunTimeFactory::makeChar(static_cast<CharExpression*>(stmt)->character);
        case ArrayExpressionType:{
            auto stm = static_cast<ArrayExpression*>(stmt);
            std::vector<RunTimeVal*> results;
            for(auto& expr : stm->exprs){
                results.push_back(copyIfRecommended(evaluate(expr)));
            }
            return ArrayWrapper::genObject(
                RunTimeFactory::makeArray(std::move(results)));
        }
        case StructExpressionType:{
            auto stm = static_cast<StructExpression*>(stmt);
            return Util::SigmaInterpreterHelper::evaluateStruct(this, stm);
        }
        case JsObjectExprType:
            return evaluateJsObjectExpression(static_cast<JsObjectExpression*>(stmt));
        case BinaryExpressionType:
            return evaluateBinaryExpression(static_cast<BinaryExpression*>(stmt));
        case ProgramType:
            return evaluateProgram(static_cast<SigmaProgram*>(stmt));    
        case VariableDeclerationType:
            return evaluateVariableDeclStatement(static_cast<VariableDecleration*>(stmt));
        case VariableReInitializationType:
            return evaluateVariableReInitStatement(static_cast<VariableReInit*>(stmt));
        case FunctionCallExpressionType:
            return evaluateFunctionCallExpression(static_cast<FunctionCallExpression*>(stmt));
        case IdentifierExpressionType:{
            IdentifierExpression* target = static_cast<IdentifierExpression*>(stmt);
            return Util::SigmaInterpreterHelper::evaluteIdentifier(this, target);
        }
        case IfStatementType:
            return evaluateIfStatement(static_cast<IfStatement*>(stmt));
        case WhileStatementType:
            return evaluateWhileLoopStatement(static_cast<WhileLoopStatement*>(stmt));
        case ForStatementType:
            return evaluateForLoopStatement(static_cast<ForLoopStatement*>(stmt));
        case ContinueStatementType:
            return RunTimeFactory::makeContinue();
        case BreakStatementType:
            return RunTimeFactory::makeBreak();
        case ReturnStatementType:
            return RunTimeFactory::makeReturn(
                evaluate(static_cast<ReturnStatement*>(stmt)->expr));
        case IndexAccessExpressionType:
            return evaluateIndexAccessExpression(static_cast<IndexAccessExpression*>(stmt));
        case IndexReInitStatementType:
            return evaluateIndexReInitStatement(static_cast<IndexReInitStatement*>(stmt));
        case StructDeclerationType:
            return evaluateStructDeclStatement(static_cast<StructDeclerationStatement*>(stmt));
        case MemberAccessExpressionType:
            return evaluateMemberAccessExpression(static_cast<MemberAccessExpression*>(stmt));
        case MemberReInitExpressionType:
            return evaluateMemberReInitStatement(static_cast<MemberReInitExpression*>(stmt));
        case IncrementExpressionType:
            return evaluateIncrementExpression(static_cast<IncrementExpression*>(stmt));
        case NegativeExpressionType:
            return evaluateNegativeExpression(static_cast<NegativeExpression*>(stmt));
        case NullExpressionType:
            return RunTimeFactory::makeVal<NullVal>();
        default: throw std::runtime_error("Not Implemented " + std::to_string(stmt->type));
    }
};

RunTimeValue SigmaInterpreter::evaluateIfStatement(IfStatement* if_stmt) {
    RunTimeVal* if_stmt_expr_val = evaluate(if_stmt->expr);

    if(if_stmt_expr_val->type != BoolType)
        throw std::runtime_error("if statement expression must result in a boolean value");

    if(static_cast<BoolVal*>(if_stmt_expr_val)->boolean)
        return Util::SigmaInterpreterHelper::evaluateIfCodeBlock(this, if_stmt->stmts);

    for(auto& else_if_stmt : if_stmt->else_if_stmts){
        RunTimeVal* else_if_expr_val = evaluate(if_stmt->expr);

        if(else_if_expr_val->type != BoolType){
            throw std::runtime_error("elseif statement expression must result in a boolean value"); 
        } 
    
        if(static_cast<BoolVal*>(else_if_expr_val)->boolean){
            return Util::SigmaInterpreterHelper::evaluateIfCodeBlock(this,
                else_if_stmt->stmts);
        }
    }

    if(!if_stmt->else_stmt)
        return nullptr;

    return Util::SigmaInterpreterHelper::evaluateIfCodeBlock(this,
        if_stmt->else_stmt->stmts);
};
RunTimeValue SigmaInterpreter::evaluateWhileLoopStatement(WhileLoopStatement* while_loop) {
    auto evaluation_scope = std::make_shared<Scope>(current_scope);
    current_scope = evaluation_scope;
    while(static_cast<BoolVal*>(evaluate(while_loop->expr))->boolean){

        bool gonna_break = false;

        auto eval_result = Util::SigmaInterpreterHelper::evaluteLoopCodeBlock(this,
            while_loop->stmts, gonna_break);
        
        if(eval_result)
            return eval_result;

        if(gonna_break) break;
        current_scope->variables.clear();
    }
    current_scope = current_scope->parent;
    garbageCollectIfNeeded();
    return nullptr;
};
RunTimeValue SigmaInterpreter::evaluateForLoopStatement(ForLoopStatement* for_loop) {
    auto scope = std::make_shared<Scope>(current_scope);
    current_scope = scope;

    if(for_loop->first_stmt)
        evaluate(for_loop->first_stmt);

    auto evaluation_scope = std::make_shared<Scope>(current_scope);
    current_scope = evaluation_scope;

    bool gonna_break = false;

    while(for_loop->expr ? static_cast<BoolVal*>(evaluate(for_loop->expr))->boolean : true){

        RunTimeVal* eval_result = Util::SigmaInterpreterHelper::evaluteLoopCodeBlock(this,
            for_loop->stmts, gonna_break);
        
        if(eval_result){
            current_scope = current_scope->parent;
            return eval_result;
        }

        if(gonna_break){
            break;
        }

        if(for_loop->last_stmt)
            evaluate(for_loop->last_stmt);

        if(!current_scope->variables.empty())
            current_scope->variables.clear();    
    }

    current_scope = current_scope->parent;
    current_scope = current_scope->parent;

    garbageCollectIfNeeded();

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateProgram(SigmaProgram* program) {
    initialize();
    auto v = static_cast<ForLoopStatement*>(program->stmts[0]);
    try{
        for(auto& stmt : program->stmts){
            evaluate(stmt);
        }
    } catch(std::exception& err){
        return RunTimeFactory::makeString(err.what());
    }

    return RunTimeFactory::makeNum(0);
};

RunTimeValue SigmaInterpreter::evaluateBinaryExpression(BinaryExpression* expr) {
    auto left = evaluate(expr->left);
    Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(&left);

    auto right = evaluate(expr->right);
    Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(&right);

    if(left->type == NumType && right->type == NumType){
        auto l = static_cast<NumVal*>(left);
        auto r = static_cast<NumVal*>(right);
        return evaluateNumericBinaryExpression(l, r, expr->op);
    }
    if(left->type == StringType && right->type == StringType){
        auto l = static_cast<StringVal*>(left);
        auto r = static_cast<StringVal*>(right);

        return evaluateStringBinaryExpression(l, r, expr->op);
    }
    if(left->type == BoolType && right->type == BoolType){
        auto l = static_cast<BoolVal*>(left);
        auto r = static_cast<BoolVal*>(right);

        return evaluateBooleanBinaryExpression(l, r, expr->op);
    }
    if(left->type == NullType && right->type == NullType){
        auto l = static_cast<NullVal*>(left);
        auto r = static_cast<NullVal*>(right);

        if(expr->op == "==")
            return RunTimeFactory::makeBool(true);
        else if (expr-> op == "!=")
            return RunTimeFactory::makeBool(false);
    }
    if((left->type == NullType && right->type != NullType) || 
        (right->type == NullType && left->type != NullType)){
        if(expr->op == "==")
            return RunTimeFactory::makeBool(false);
        else if(expr->op == "!=")
            return RunTimeFactory::makeBool(true);
    }
    throw std::runtime_error("Binary Operators Not Implemented For Operands " + 
        type_to_string_table.at(left->type) + "," + type_to_string_table.at(right->type));
};

RunTimeValue SigmaInterpreter::evaluateVariableDeclStatement(VariableDecleration* decl) {
    if(decl->expr == nullptr){
        current_scope->declareVar(decl->var_name, {RunTimeFactory::makeVal<NullVal>()});
        return nullptr;
    }
    auto val = evaluate(decl->expr);
    if(!shouldICopy(val)){
        current_scope->declareVar(decl->var_name, { val,
         decl->is_const });
        return nullptr;
    }
    current_scope->declareVar(decl->var_name, { val->clone(),
         decl->is_const });
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateVariableReInitStatement(VariableReInit* decl) {
    // extensive comments cause the level of the complexity

    // checking if there's a valid 'this' in any scope
    // ( if 'this' is valid it means that we are in a member function )
    // and if its valid it also means that we should check if the 'this'
    // object includes the variable which we wanna reinitialize
    // so we can implictly add the 'this.' before the variable name :pray:
    auto this_keyword_scope = current_scope->traverse(this_str);
    if(this_keyword_scope){
        // checking if the 'this' is actually an object
        auto this_val = this_keyword_scope->variables[this_str].value;
        if(this_val->type == StructType){
            // casting to an object value
            auto vall = static_cast<ObjectVal*>(
                this_val);
            // checking if the 'this' object has the variable name which we are
            // trying to reinitialize
            if(vall->vals.contains(decl->var_name)){

                // basic reinializement logic
                RunTimeVal* new_value = evaluate(decl->expr);
                ObjectVal* this_struct = static_cast<ObjectVal*>(this_val);

                if(this_struct->vals[decl->var_name]->type == RefrenceType){
                    auto actual_ref = static_cast<RefrenceVal*>(this_struct->vals[decl->var_name]);
                    *actual_ref->val = new_value;
                    return RunTimeFactory::makeVal<NullVal>();
                }
                if(shouldICopy(new_value))
                    this_struct->vals[decl->var_name]->setValue(new_value);
                else {
                    this_struct->vals[decl->var_name] = new_value;
                }
                return RunTimeFactory::makeVal<NullVal>();
            }
        }
    }

    // repeated basic intializement logic ( im a bit lazy )
    RunTimeVal* target_value = evaluate(decl->expr);
    RunTimeVal* previous_val = current_scope->getVal(decl->var_name);

    if(previous_val->type == RefrenceType){
        auto actual_ref = 
            static_cast<RefrenceVal*>(previous_val);
        *actual_ref->val = (target_value);
        return nullptr;
    }
    if(shouldICopy(previous_val)){
        previous_val->setValue(target_value);
        return nullptr;
    }
    current_scope->reInitVar(decl->var_name, target_value);

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateFunctionCallExpression(FunctionCallExpression* expr) {
    
    std::vector<RunTimeVal*> args(expr->args.size());
    auto func = evaluate(expr->func_expr);

    if(func->type == LambdaType)
    std::transform(expr->args.begin(), expr->args.end(), args.begin(),
        [this](Expr expr)-> RunTimeVal* { 
            RunTimeVal* val = evaluate(expr);
            return copyIfRecommended(val);
        });
    else if(func->type == NativeFunctionType){
        StdLib::current_calling_scope = current_scope;
        args = Util::SigmaInterpreterHelper::evaluateExprVectorForCompiledFunctions(this,
            expr->args);
    }

    if(func->type == NativeFunctionType){
        auto new_scope = std::make_shared<Scope>(current_scope);
        current_scope = new_scope;

        if(expr->func_expr->type == MemberAccessExpressionType){
            MemberAccessExpression* mem_expr = static_cast<MemberAccessExpression*>(
                expr->func_expr)->clone_expr();
            mem_expr->path.pop_back();
            RunTimeVal* mem_val = evaluate(mem_expr);
            current_scope->declareVar(this_str, { mem_val, true });
        }
        
        Util::SigmaInterpreterHelper::verifyCompiledFunctionCallArgs(
            this, expr, args, func);

        NativeFunctionVal* func_val = static_cast<NativeFunctionVal*>(func); 

        auto ret = func_val->func(args, this);
        current_scope = current_scope->parent;
        StdLib::current_calling_scope = nullptr;

        if(ret)
            garbageCollectionRestricter.protectValue(ret);

        garbageCollectIfNeeded();

        if(ret)
            garbageCollectionRestricter.clearValProtection();

        if(!ret)
            return RunTimeFactory::makeVal<NullVal>();
        return ret;
    }
    if (func->type != LambdaType){
        throw std::runtime_error(std::format("{} is not a callable", (int)func->type));
    }
    auto actual_func = static_cast<LambdaVal*>(func);
    
    auto last_scope = current_scope;

    auto arg_scope = std::make_shared<Scope>(current_scope);
    current_scope = arg_scope;

    for (int i = 0; i < args.size(); i++) {
      current_scope->declareVar(actual_func->params[i], {args[i], false});
    }

    for(auto& [var_name, var_val] : actual_func->captured){
         if(!current_scope->traverse(var_name))
             current_scope->declareVar(var_name, {var_val, true});
    }

    // making the 'this' keyword valid
    
    // getting the object by basically removing the latest member access string
    // and evaluating the remaining ( some tricks are applied )
    if(expr->func_expr->type == MemberAccessExpressionType){
        auto mem_expr = static_cast<MemberAccessExpression*>(expr->func_expr)->clone_expr();
        
        mem_expr->path.pop_back();
        
        current_scope = last_scope;
        auto mem_val = evaluate(mem_expr);
        current_scope = arg_scope;
        
        current_scope->declareVar(this_str, { mem_val, true });
    }

    auto func_scope = std::make_shared<Scope>(current_scope);
    
    current_scope = func_scope;

    RunTimeValue return_val;

    for (auto &stmt : actual_func->stmts) {
      auto val = evaluate(stmt);
      if (!val)
        continue;
      if (val->type == ReturnType) {
        return_val = static_cast<ReturnVal*>(val)->val;
        break;
      }
    }

    current_scope = current_scope->parent;
    current_scope = last_scope;

    if(return_val)
        garbageCollectionRestricter.protectValue(return_val);

    garbageCollectIfNeeded();

    if(return_val)
        garbageCollectionRestricter.clearValProtection();

    return return_val;
};

RunTimeValue SigmaInterpreter::
    evaluateIndexAccessExpression(IndexAccessExpression* expr) {
    RunTimeVal* val = dynamic_cast<ObjectVal*>(evaluate(expr->array_expr));

    Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(&val);
    
    for(auto& num : expr->path){
        
        auto numb = evaluate(num);

        if(numb->type != NumType) throw std::runtime_error("operator [] excepts a number");

        auto real_num = static_cast<NumVal*>(numb);

        if(val->type == StringType){
            auto real_val = static_cast<StringVal*>(val);    
            if(real_num->num >= real_val->str.size()) throw std::runtime_error("out of bounds array index");
            val = RunTimeFactory::makeChar(real_val->str[static_cast<int>(real_num->num)]);
            return val;
        }

        if(val->type != ArrayType) throw std::runtime_error("operator [] must be used on an array");
        auto real_val = static_cast<ArrayVal*>(val);

        if(real_num->num >= real_val->vals.size()) throw std::runtime_error("out of bounds array index");
        val = real_val->vals[static_cast<int>(real_num->num)];

        Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(&val);
    }


    return val;
};

RunTimeValue SigmaInterpreter::
    evaluateMemberAccessExpression(MemberAccessExpression* expr) {
    auto val = evaluate(expr->struct_expr);
    
    for(auto& str : expr->path){
        if(val->type != StructType) throw std::runtime_error("operator . must be used on an object the current type is: " + 
            std::to_string(val->type));
        auto real_val = static_cast<ObjectVal*>(val);

        if(!real_val->vals.contains(str)) throw std::runtime_error("member " + str + " not found in an object");
        
        val = real_val->vals[str];

    }


    return val;
};

RunTimeValue SigmaInterpreter::evaluateIndexReInitStatement(IndexReInitStatement* stmt) {
    RunTimeVal* val = dynamic_cast<ObjectVal*>(evaluate(stmt->array_expr));
    Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(&val);

    auto latest_num = 
        static_cast<NumVal*>(evaluate(stmt->path.back()));
    auto p = stmt->path;
    p.pop_back();

    for(auto& num : p){
        val = static_cast<ArrayVal*>(val)->vals[
            static_cast<int>(static_cast<NumVal*>(evaluate(num))->num)
        ];
        Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(&val);
    }

    if(val->type == StringType){
        auto latest_val = static_cast<StringVal*>(val);
        latest_val->str[static_cast<int>(latest_num->num)] = 
            static_cast<CharVal*>(evaluate(stmt->val))->ch; 
        return nullptr;
    }
    auto latest_val = static_cast<ArrayVal*>(val);

    if(latest_val->vals[static_cast<int>(latest_num->num)] && latest_val->vals[static_cast<int>(latest_num->num)]->type == RefrenceType){
        auto v = 
            static_cast<RefrenceVal*>(latest_val->vals[static_cast<int>(latest_num->num)]);
        *v->val = evaluate(stmt->val);
        return nullptr;
    }

    auto actual_val = evaluate(stmt->val);
    if(shouldICopy(actual_val)){
        
        latest_val->vals[static_cast<int>(latest_num->num)]->setValue(actual_val);
    }
    else {
        latest_val->vals[static_cast<int>(latest_num->num)] = actual_val;
    }
    
    garbageCollectIfNeeded();
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateNumericBinaryExpression(NumVal* left,
    NumVal* right, std::string operat) {
    if(operat == "+")
        return RunTimeFactory::makeNum(left->num + right->num);
    if(operat == "-")
        return RunTimeFactory::makeNum(left->num - right->num);
    if(operat == "*")
        return RunTimeFactory::makeNum(left->num * right->num);
    if(operat == "/")
        return RunTimeFactory::makeNum(left->num / right->num);
    if(operat == "%")
        return RunTimeFactory::makeNum((long)left->num % (long)right->num);
    if(operat == "&")
        return RunTimeFactory::makeNum((long)left->num & (long)right->num);
    if(operat == "|")
        return RunTimeFactory::makeNum((long)left->num | (long)right->num);
    if(operat == ">>")
        return RunTimeFactory::makeNum((long)left->num >> (long)right->num);
    if(operat == "<<")
        return RunTimeFactory::makeNum((long)left->num << (long)right->num);
    if(operat == "==")
        return RunTimeFactory::makeBool(left->num == right->num);
    if(operat == ">")
        return RunTimeFactory::makeBool(left->num > right->num);
    if(operat == "<")
        return RunTimeFactory::makeBool(left->num < right->num);
    if(operat == ">=")
        return RunTimeFactory::makeBool(left->num >= right->num);
    if(operat == "<=")
        return RunTimeFactory::makeBool(left->num <= right->num);
    if(operat == "!=")
        return RunTimeFactory::makeBool(left->num != right->num);

    throw std::runtime_error("operator " + operat + " isn't valid between operands Number, Number");
};
RunTimeValue SigmaInterpreter::evaluateBooleanBinaryExpression(BoolVal* left,
    BoolVal* right, std::string op) {
    if(op == "==")
        return RunTimeFactory::makeBool(left->boolean == right->boolean);
    if(op == "!=")
        return RunTimeFactory::makeBool(left->boolean != right->boolean);
    if(op == ">")
        return RunTimeFactory::makeBool(left->boolean > right->boolean);
    if(op == "<")
        return RunTimeFactory::makeBool(left->boolean < right->boolean);
    if(op == ">=")
        return RunTimeFactory::makeBool(left->boolean >= right->boolean);
    if(op == "<=")
        return RunTimeFactory::makeBool(left->boolean <= right->boolean);
    if(op == "|")
        return RunTimeFactory::makeNum(left->boolean | right->boolean);
    if(op == "&")
        return RunTimeFactory::makeNum(left->boolean & right->boolean);
    if(op == "&&")
        return RunTimeFactory::makeBool(left->boolean && right->boolean);
    if(op == "||")
        return RunTimeFactory::makeBool(left->boolean || right->boolean);
    if(op == ">>")
        return RunTimeFactory::makeNum(left->boolean >> right->boolean);
    if(op == "<<")
        return RunTimeFactory::makeNum(left->boolean << right->boolean);

    throw std::runtime_error("operator " + op + " isn't valid between operands Boolean, Boolean");
};
// deprecated
RunTimeValue SigmaInterpreter::evaluateStringBinaryExpression(StringVal* left,
    StringVal* right, std::string op) {
    if(op == "==")
        return RunTimeFactory::makeBool(left->str == right->str);
    if(op == "!=")
        return RunTimeFactory::makeBool(left->str != right->str);
    if(op == ">")
        return RunTimeFactory::makeBool(left->str > right->str);
    if(op == "<")
        return RunTimeFactory::makeBool(left->str < right->str);
    if(op == ">=")
        return RunTimeFactory::makeBool(left->str >= right->str);
    if(op == "<=")
        return RunTimeFactory::makeBool(left->str <= right->str);
    if(op == "+")
        return StringWrapper::genObject(RunTimeFactory::makeString(left->str + right->str));

    throw std::runtime_error("operator " + op + " isn't valid between operands String, String");
};

RunTimeValue SigmaInterpreter::
    evaluateStructDeclStatement(StructDeclerationStatement* stmt) {
    auto constructor_itr = std::find_if(stmt->props.begin(), stmt->props.end(),
        [](VariableDecleration* decl){
        return decl->var_name == "constructor" && decl->expr->type == LambdaExpressionType;
    });

    if(constructor_itr != stmt->props.end()){
        VariableDecleration* constructor_decl = *constructor_itr;
        stmt->props.erase(constructor_itr);

        struct_decls.insert({stmt->struct_name, {stmt->props, 
            static_cast<LambdaExpression*>(constructor_decl->expr)}});

        return nullptr;
    }


    struct_decls.insert({stmt->struct_name, {stmt->props, nullptr}});

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateMemberReInitStatement(
    MemberReInitExpression* expr) {
    auto val = evaluate(expr->struct_expr);
    auto path_copy = expr->path;
    auto latest_str = path_copy.back();
    path_copy.pop_back();

    for(auto& str : path_copy){
        val = static_cast<ObjectVal*>(val)->vals[
            str
        ];
    }

    auto latest_val = static_cast<ObjectVal*>(val);
    if(latest_val->vals[latest_str]->type == RefrenceType){
        auto v = static_cast<RefrenceVal*>(latest_val->vals[latest_str]);
        *v->val = evaluate(expr);
        return nullptr;
    }
    auto actual_v = evaluate(expr->val);
    if(shouldICopy(actual_v))
        latest_val->vals[latest_str]->setValue(actual_v);
    else {
        latest_val->vals[latest_str] = actual_v;
    }

    garbageCollectIfNeeded();
    return nullptr;
};

// some garbage code i guess
RunTimeValue SigmaInterpreter::evaluateIncrementExpression(IncrementExpression* expr) {
    auto current_val = evaluate(expr->expr);

    if(current_val->type != NumType) throw std::runtime_error("can't increment a non-number");
    double actual_val = static_cast<NumVal*>(current_val)->num;
    if(expr->expr->type == IdentifierExpressionType){
        if(!expr->cached_variable_reinit){
            auto iden_expr = static_cast<IdentifierExpression*>(expr->expr);
            expr->cached_variable_reinit = SigmaParser::makeAst<VariableReInit>((iden_expr->str), 
                SigmaParser::makeAst<NumericExpression>(actual_val + expr->amount));
        }
        else {
            static_cast<NumericExpression*>(
                expr->cached_variable_reinit->expr)->num = actual_val + expr->amount;
        }
        evaluateVariableReInitStatement(expr->cached_variable_reinit);
    } else if (expr->expr->type == MemberAccessExpressionType){
        if(!expr->cached_member_reinit){
            auto ac_expr = static_cast<MemberAccessExpression*>(expr->expr);
            expr->cached_member_reinit = SigmaParser::makeAst<MemberReInitExpression>(
                ac_expr->struct_expr, ac_expr->path, SigmaParser::makeAst<NumericExpression>(actual_val + expr->amount)
            );
        } else { static_cast<NumericExpression*>(expr->cached_member_reinit->val)->num = actual_val + expr->amount; }
        evaluateMemberReInitStatement(expr->cached_member_reinit);
    } else if (expr->expr->type == ArrayExpressionType){
        if(!expr->cached_index_reinit){
            auto ac_expr = static_cast<IndexAccessExpression*>(expr->expr);
            expr->cached_index_reinit = SigmaParser::makeAst<IndexReInitStatement>(
                ac_expr->array_expr, ac_expr->path, SigmaParser::makeAst<NumericExpression>(actual_val + expr->amount)
            );
        } else { static_cast<NumericExpression*>(expr->cached_index_reinit->val)->num = actual_val + expr->amount; }
        evaluateIndexReInitStatement(expr->cached_index_reinit);
    }
    return RunTimeFactory::makeNum(actual_val + expr->amount);
};
// no implementation cause the decrement expression is basically an increment but 
// adding -1 instead of 1
RunTimeValue SigmaInterpreter::evaluateDecrementExpression(DecrementExpression* expr) {
    return {};
};
RunTimeValue SigmaInterpreter::evaluateNegativeExpression(NegativeExpression* expr) {
    auto val = evaluate(expr->expr);
    if(val->type != NumType)
        throw std::runtime_error("can't make a non-number value negative");

    double num = static_cast<NumVal*>(val)->num;

    return RunTimeFactory::makeNum(-num);
};
RunTimeVal* SigmaInterpreter::evaluateJsObjectExpression(JsObjectExpression* expr) {
    std::unordered_map<std::string, RunTimeVal*> values;

    for(auto& [name, val]: expr->exprs){
        values.insert({name, evaluate(val)->clone()});
    }

    return RunTimeFactory::makeStruct(std::move(values));
};
#include "RunTime.h"
#include "SigmaInterpreter.h"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <memory>
#include <numeric>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <vector>
#include "../Interpreter/Interpreter.h"
#include "../Interpreter/Lexer.h"
#include "../Interpreter/Parser.h"
#include "Cryptography.h"

typedef RunTimeVal* RunTimeValue;

RunTimeValue SigmaInterpreter::toString(std::vector<RunTimeValue>& args) {
    if(args.size() > 0){
        return RunTimeFactory::makeString(args[0]->getString());
    } else return RunTimeFactory::makeString("");
};

// args -> [0] : size, [1] : start
RunTimeValue SigmaInterpreter::numIota(std::vector<RunTimeValue>& args) {
    return nullptr;
};

RunTimeValue SigmaInterpreter::clone(std::vector<RunTimeValue>& args) {
    return args[0]->clone();
};

RunTimeValue SigmaInterpreter::getCurrentTimeMillis(std::vector<RunTimeValue>& args) {
    std::chrono::time_point tim = std::chrono::high_resolution_clock::now();
    long actual_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        tim).time_since_epoch().count();
    
    return RunTimeFactory::makeNum(actual_time);
};


RunTimeValue SigmaInterpreter::getElementById(std::vector<RunTimeValue>& args) {
    std::string id = dynamic_cast<StringVal*>(args[0])->str;
    return RunTimeFactory::makeHtmlElement(accessor->id_ptrs.at(id).get());
};
RunTimeValue SigmaInterpreter::setElementInnerHtml(std::vector<RunTimeValue>& args){
    auto html_elm = dynamic_cast<HtmlElementVal*>(args[0]);
    Lexer lex;
    Parser pars;
    Interpreter interpret;
    std::string html_str = dynamic_cast<StringVal*>(args[1])->str;
    
    auto tokens = lex.tokenize(html_str);
    auto ast_val = pars.produceAst(tokens);
    for(auto& child : html_elm->target_tag->children)
        child->unRender();
    html_elm->target_tag->setChildren(ast_val.html_tags);
    interpret.renderTags(dynamic_cast<Gtk::Box*>(html_elm->target_tag->current_widget), ast_val);
    accessor->current_interp->refreshIdsAndClasses();
    return nullptr;
};

RunTimeValue SigmaInterpreter::getElementsByClassName(std::vector<RunTimeValue>& args) {
    std::string cl = dynamic_cast<StringVal*>(args[0])->str;
    auto elms = accessor->class_name_ptrs.equal_range(cl);
    auto [beg_itr, end_itr] = elms;

    std::vector<RunTimeValue> results;
    for(auto itr = beg_itr; itr != end_itr; itr++){
        results.push_back(RunTimeFactory::makeHtmlElement(itr->second.get()));
    }

    return RunTimeFactory::makeArray(results);
};

RunTimeValue SigmaInterpreter::
    evaluateAnonymousLambdaCall(LambdaVal* lambda, std::vector<RunTimeValue> args){
    auto actual_func = lambda;
    
    auto last_scope = current_scope;

    auto arg_scope = std::make_shared<Scope>(current_scope);
    current_scope = arg_scope;
    for (int i = 0; i < args.size(); i++) {
      current_scope->declareVar(actual_func->params[i], {args[i], false});
    }
    // for(auto& [var_name, var_val] : actual_func->captured){
    //     if(!current_scope->variables.contains(var_name))
    //         current_scope->declareVar(var_name, {var_val, true});
    // }

    auto func_scope = std::make_shared<Scope>(current_scope);
    
    current_scope = func_scope;

    RunTimeValue return_val;

    for (auto &stmt : actual_func->stmts) {
      auto val = evaluate(stmt);
      if (!val)
        continue;
      if (val->type == ReturnType) {
        return_val = dynamic_cast<ReturnVal*>(val)->val;
        break;
      }
    }

    current_scope = current_scope->parent;
    current_scope = last_scope;

    return return_val;
}
#include "GarbageCollector/GarbageCollector.h"
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
#include "StandardLibrary/TypeWrappers/StringWrapper.h"
#include "Util/Util.h"

typedef RunTimeVal* RunTimeValue;

RunTimeValue SigmaInterpreter::toString(std::vector<RunTimeValue>& args) {
    if(args.size() > 0){
        return StringWrapper::genObject(RunTimeFactory::makeString(args[0]->getString()));
    } else return StringWrapper::genObject(RunTimeFactory::makeString(""));
};

// args -> [0] : size, [1] : start
RunTimeValue SigmaInterpreter::numIota(std::vector<RunTimeValue>& args) {
    return nullptr;
};

RunTimeValue SigmaInterpreter::clone(std::vector<RunTimeValue>& args) {
    return Util::SigmaInterpreterHelper::cvtToWrapperIfPossible(args[0]->clone());
};

RunTimeValue SigmaInterpreter::getCurrentTimeMillis(std::vector<RunTimeValue>& args) {
    std::chrono::time_point tim = std::chrono::high_resolution_clock::now();
    long actual_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        tim).time_since_epoch().count();
    
    return RunTimeFactory::makeNum(actual_time);
};


RunTimeVal* SigmaInterpreter::getStringASCII(std::vector<RunTimeVal*>& args) {
  StringVal* str_val = dynamic_cast<StringVal*>(args[0]);

  double sum = std::accumulate(str_val->str.begin(), str_val->str.end(), 0);

  return RunTimeFactory::makeNum(sum);
};

RunTimeVal* SigmaInterpreter::toNum(std::vector<RunTimeVal*>& args){
  BoolVal* bool_v = dynamic_cast<BoolVal*>(args[0]);
  if(bool_v) return RunTimeFactory::makeNum(bool_v ? 1 : 0);

  StringVal* str_v = dynamic_cast<StringVal*>(args[0]);

  if(str_v) return RunTimeFactory::makeNum(std::stod(str_v->str));
  
  CharVal* ch_v = dynamic_cast<CharVal*>(args[0]);
  if(ch_v) return RunTimeFactory::makeNum(ch_v->ch);

  throw std::runtime_error("toNum excepts arg 0 to be bool or str or char");
}

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

    if(return_val)
      garbageCollectionRestricter.protectValue(return_val);

    garbageCollectIfNeeded();

    if(return_val)
      garbageCollectionRestricter.clearValProtection();

    return return_val;
}
std::vector<RunTimeVal*> SigmaInterpreter::getAccessibleValues() {
  std::vector<RunTimeVal*> mark_vals = current_scope->flatten_as_vec();
  std::vector<RunTimeVal*> restricted_vals = garbageCollectionRestricter.getRestrictedValues();

  mark_vals.insert(mark_vals.end(), restricted_vals.begin(), restricted_vals.end());

  return mark_vals;
};
void SigmaInterpreter::garbageCollectIfNeeded() {
  if(GarbageCollector::massiveGCShouldRun()){
      std::cout << "garbage collecting";

    std::vector<RunTimeVal*> values = getAccessibleValues();
    GarbageCollector::mark(values);
    GarbageCollector::sweep(RunTimeMemory::pool);
  }
};
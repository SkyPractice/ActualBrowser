#pragma once
#include "RunTime.h"
#include <string>
#include <unordered_map>

class GCRestricter {
    std::vector<RunTimeVal*> registered_event_handlers;
    std::unordered_map<std::string, RunTimeVal*> async_lambdas;
    std::vector<RunTimeVal*> wrapper_types_cache;
    std::vector<RunTimeVal*> protected_values;
public:

    void protectValue(RunTimeVal* val);
    void clearValProtection();

    void registerEventHandler(RunTimeVal* handler);
    void unRegisterEventHandlers();

    void registerWrapperTypeFunctions(std::unordered_map<std::string, RunTimeVal*>& functions);
    void clearWrapperTypeFunctions();

    void registerAsyncLambda(LambdaVal* lambda);
    void unRegisterAsyncLambda(std::string& uuid);

    std::vector<RunTimeVal*> getRestrictedValues();

    void reset();
};
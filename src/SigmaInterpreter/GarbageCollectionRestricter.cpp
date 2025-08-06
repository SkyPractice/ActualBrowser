#include "GarbageCollectionRestricter.h"
#include "RunTime.h"

void GCRestricter::protectValue(RunTimeVal* val) {
    protected_values.push_back(val);
};
void GCRestricter::clearValProtection() {
    protected_values.clear();
};
void GCRestricter::registerEventHandler(RunTimeVal* handler) {
    registered_event_handlers.push_back(handler);
};
void GCRestricter::unRegisterEventHandlers() {
    registered_event_handlers.clear();
};
void GCRestricter::registerWrapperTypeFunctions(std::unordered_map<std::string, RunTimeVal*>& functions) {
    for(auto& [uuid, func] : functions)
        wrapper_types_cache.push_back(func);
};
void GCRestricter::clearWrapperTypeFunctions() {
    wrapper_types_cache.clear();
};
void GCRestricter::registerAsyncLambda(LambdaVal* lambda) {
    async_lambdas.insert({lambda->lambda_uuid, lambda});
};
void GCRestricter::unRegisterAsyncLambda(std::string& uuid) {
    async_lambdas.erase(uuid);
};
std::vector<RunTimeVal*> GCRestricter::getRestrictedValues() {
    std::vector<RunTimeVal*> restricted_vals;

    restricted_vals.insert(restricted_vals.end(), registered_event_handlers.begin(), 
        registered_event_handlers.end());
    restricted_vals.insert(restricted_vals.end(), wrapper_types_cache.begin(), 
        wrapper_types_cache.end());
    restricted_vals.insert(restricted_vals.end(), protected_values.begin(),
        protected_values.end());
    
    for(auto& [name, lambda] : async_lambdas){
        restricted_vals.push_back(lambda);
    }

    return restricted_vals;
};

void GCRestricter::reset(){
    registered_event_handlers.clear();
    wrapper_types_cache.clear();
    protected_values.clear();
    async_lambdas.clear();
}
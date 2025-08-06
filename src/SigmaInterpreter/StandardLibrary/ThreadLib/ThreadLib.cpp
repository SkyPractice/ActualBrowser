#include "ThreadLib.h"
#include <chrono>
#include <memory>
#include <thread>
#include "../../../Concurrency/ThreadPool.h"
#include "../../SigmaInterpreter.h"
#include <boost/asio.hpp>
#include <unordered_map>


ObjectVal* ThreadLib::getStruct(){
    std::unordered_map<std::string, RunTimeValue> vals = {
        {"sleep", RunTimeFactory::makeNativeFunction(&ThreadLib::sleepCurrentThread, {{"duration_millis", NumType}})},
        {"detach", RunTimeFactory::makeNativeFunction(&ThreadLib::detachLambda, {{"target_lambda", LambdaType}})}
    };

    return RunTimeFactory::makeStruct(vals);
};
RunTimeValue ThreadLib::sleepCurrentThread(std::vector<RunTimeValue>& args, SigmaInterpreter*) {
    long duration = static_cast<long>(dynamic_cast<NumVal*>(args[0])->num);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    return nullptr;
};
RunTimeValue ThreadLib::detachLambda(std::vector<RunTimeValue>& args, SigmaInterpreter* interpreter) {
    auto lambda_val = dynamic_cast<LambdaVal*>(args[0]);
    interpreter->garbageCollectionRestricter.registerAsyncLambda(lambda_val);
    boost::asio::post(Concurrency::pool, [args, interpreter, lambda_val]() mutable {
        std::vector<RunTimeValue> lambda_args(args.begin() + 1, args.end());
        interpreter->evaluateAnonymousLambdaCall(lambda_val, lambda_args);
        interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda_val->lambda_uuid);
    });
    return nullptr;
};
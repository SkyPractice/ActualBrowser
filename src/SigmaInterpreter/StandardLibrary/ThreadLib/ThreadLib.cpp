#include "ThreadLib.h"
#include <chrono>
#include <memory>
#include <thread>
#include "../../../Concurrency/ThreadPool.h"
#include "../../SigmaInterpreter.h"
#include <boost/asio.hpp>
#include <unordered_map>


std::shared_ptr<StructVal> ThreadLib::getStruct(){
    std::unordered_map<std::string, RunTimeValue> vals = {
        {"sleep", RunTimeFactory::makeNativeFunction(&ThreadLib::sleepCurrentThread)},
        {"detach", RunTimeFactory::makeNativeFunction(&ThreadLib::detachLambda)}
    };

    return RunTimeFactory::makeStruct(vals);
};
RunTimeValue ThreadLib::sleepCurrentThread(std::vector<RunTimeValue>& args) {
    long duration = static_cast<long>(std::dynamic_pointer_cast<NumVal>(args[0])->num);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    return nullptr;
};
RunTimeValue ThreadLib::detachLambda(std::vector<RunTimeValue>& args) {
    std::vector<RunTimeValue> actual_args = args;
    std::shared_ptr<Scope> current_sc = StdLib::current_calling_scope;
    boost::asio::post(Concurrency::pool, [args, current_sc]() mutable {
        auto lambda_val = std::dynamic_pointer_cast<LambdaVal>(args[0]);
        SigmaInterpreter interpp;
        interpp.current_scope = current_sc;
        std::vector<RunTimeValue> lambda_args(args.begin() + 1, args.end());
        interpp.evaluateAnonymousLambdaCall(lambda_val, lambda_args);
    });
    return nullptr;
};
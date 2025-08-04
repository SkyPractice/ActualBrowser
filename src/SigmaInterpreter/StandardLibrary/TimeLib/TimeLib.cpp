#include "TimeLib.h"
#include <chrono>
#include <ctime>
#include <unordered_map>

RunTimeVal* TimeLib::getStruct(){
    std::unordered_map<std::string, RunTimeValue> vals = {
        {"getCurrentTimeMillis", RunTimeFactory::makeNativeFunction(&TimeLib::getCurrentTimeMillis, {})},
        {"getCurrentTimeMicros", RunTimeFactory::makeNativeFunction(&TimeLib::getCurrentTimeMicros, {})},
        {"getCurrentTimeSeconds", RunTimeFactory::makeNativeFunction(&TimeLib::getCurrentTimeSecs, {})},
        {"getCurrentTime", RunTimeFactory::makeNativeFunction(&TimeLib::getTime, {})},
    };

    return RunTimeFactory::makeStruct(vals);
}

RunTimeVal* TimeLib::getCurrentTimeMillis(COMPILED_FUNC_ARGS) {
    std::chrono::time_point tim = std::chrono::high_resolution_clock::now();
    long actual_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        tim).time_since_epoch().count();
    
    return RunTimeFactory::makeNum(actual_time);
};
RunTimeVal* TimeLib::getCurrentTimeMicros(COMPILED_FUNC_ARGS) {
    std::chrono::time_point tim = std::chrono::high_resolution_clock::now();
    long actual_time = std::chrono::time_point_cast<std::chrono::microseconds>(
        tim).time_since_epoch().count();
    
    return RunTimeFactory::makeNum(actual_time);
};
RunTimeVal* TimeLib::getCurrentTimeSecs(COMPILED_FUNC_ARGS) {
    return RunTimeFactory::makeNum(time(0));
};
RunTimeVal* TimeLib::getTime(COMPILED_FUNC_ARGS) {
    long current_time = time(0);
    tm* local_current_time = localtime(&current_time);

    std::unordered_map<std::string, RunTimeVal*> tm_members = {
        {"second", RunTimeFactory::makeNum(local_current_time->tm_sec)},
        {"minute", RunTimeFactory::makeNum(local_current_time->tm_min)},
        {"hour", RunTimeFactory::makeNum(local_current_time->tm_hour)},
        {"week_day", RunTimeFactory::makeNum(local_current_time->tm_wday + 1)},
        {"month_day", RunTimeFactory::makeNum(local_current_time->tm_mday)},
        {"year_day", RunTimeFactory::makeNum(local_current_time->tm_yday)},
        {"month", RunTimeFactory::makeNum(local_current_time->tm_mon + 1)},
        {"year", RunTimeFactory::makeNum(local_current_time->tm_year + 1900)}
    };

    return RunTimeFactory::makeStruct(tm_members);
};
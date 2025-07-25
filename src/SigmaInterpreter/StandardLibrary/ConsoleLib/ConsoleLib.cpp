#include "ConsoleLib.h"
#include <memory>
#include <numeric>
#include <iostream>
#include <ostream>
#include <string_view>
#include <unordered_map>

std::unordered_map<std::string_view, std::string_view> ConsoleLib::colorMap = {
        {"black", ConsoleColors::BLACK}, {"red", ConsoleColors::RED}, 
        {"green", ConsoleColors::GREEN}, {"yellow", ConsoleColors::YELLOW},
        {"blue", ConsoleColors::BLUE}, {"magenta", ConsoleColors::MAGENTA},
        {"cyan", ConsoleColors::CYAN}, {"white", ConsoleColors::WHITE},
        {"bg_black", ConsoleColors::BG_BLACK}, {"bg_red", ConsoleColors::BG_RED}, 
        {"bg_green", ConsoleColors::BG_GREEN}, {"bg_yellow", ConsoleColors::BG_YELLOW},
        {"bg_blue", ConsoleColors::BG_BLUE}, {"bg_magenta", ConsoleColors::BG_MAGENTA},
        {"bg_cyan", ConsoleColors::BG_CYAN}, {"bg_white", ConsoleColors::BG_WHITE}
};

std::shared_ptr<StructVal> ConsoleLib::getStruct() {
    std::unordered_map<std::string, std::shared_ptr<RunTimeVal>> vals = {
        {"println", RunTimeFactory::makeNativeFunction(&ConsoleLib::println)},
        {"print", RunTimeFactory::makeNativeFunction(&ConsoleLib::print)},
        {"input", RunTimeFactory::makeNativeFunction(&ConsoleLib::input)}
    };
    return RunTimeFactory::makeStruct(std::move(vals));
};

RunTimeValue ConsoleLib::println(std::vector<RunTimeValue>& args){

    std::string choosen_clr = "white";
    if(args.size() > 1){
        choosen_clr = std::dynamic_pointer_cast<StringVal>(args[1])->str;
    }

    std::cout << colorMap.at(choosen_clr) << 
        std::dynamic_pointer_cast<StringVal>(args[0])->str << ConsoleColors::RESET << std::endl;

    return RunTimeFactory::makeNum(std::accumulate(args.begin(), args.end(), 0,
        [](size_t current_total_size, RunTimeValue second) {
            return current_total_size +
                std::dynamic_pointer_cast<StringVal>(second)->str.size();
        }) + 1);
};
RunTimeValue ConsoleLib::print(std::vector<RunTimeValue>& args){
    std::string choosen_clr = "white";
    if(args.size() > 1){
        choosen_clr = std::dynamic_pointer_cast<StringVal>(args[1])->str;
    }

    std::cout << colorMap.at(choosen_clr) << 
        std::dynamic_pointer_cast<StringVal>(args[0])->str << ConsoleColors::RESET << std::flush;
    return RunTimeFactory::makeNum(std::accumulate(args.begin(), args.end(), 0,
        [](size_t current_total_size, RunTimeValue second) {
            return current_total_size +
                std::dynamic_pointer_cast<StringVal>(second)->str.size();
        }));
};


RunTimeValue ConsoleLib::input(std::vector<RunTimeValue>& args) {
    std::string str;
    std::getline(std::cin, str);
    return RunTimeFactory::makeString(str);
};

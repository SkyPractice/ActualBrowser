#include "ConsoleLib.h"
#include <memory>
#include <numeric>
#include <iostream>
#include <ostream>
#include <string_view>
#include <unordered_map>
#include "../TypeWrappers/StringWrapper.h"

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

ObjectVal* ConsoleLib::getStruct() {
    std::unordered_map<std::string, RunTimeVal*> vals = {
        {"println", RunTimeFactory::makeNativeFunction(&ConsoleLib::println, {{ "string", StringType, false}, { "color", StringType, true }})},
        {"print", RunTimeFactory::makeNativeFunction(&ConsoleLib::print, { { "string", StringType, false }, {"color", StringType, true} })},
        {"input", RunTimeFactory::makeNativeFunction(&ConsoleLib::input, {})}
    };
    return RunTimeFactory::makeStruct(std::move(vals));
};

RunTimeValue ConsoleLib::println(std::vector<RunTimeValue>& args, SigmaInterpreter*){

    std::string choosen_clr = "white";
    if(args.size() > 1){
        choosen_clr = dynamic_cast<StringVal*>(args[1])->str;
    }

    std::cout << colorMap.at(choosen_clr) << 
        dynamic_cast<StringVal*>(args[0])->str << ConsoleColors::RESET << std::endl;

    return RunTimeFactory::makeNum(dynamic_cast<StringVal*>(args[0])->str.size() + 1);
};
RunTimeValue ConsoleLib::print(std::vector<RunTimeValue>& args, SigmaInterpreter*){
    std::string choosen_clr = "white";
    if(args.size() > 1){
        choosen_clr = dynamic_cast<StringVal*>(args[1])->str;
    }

    std::cout << colorMap.at(choosen_clr) << 
        dynamic_cast<StringVal*>(args[0])->str << ConsoleColors::RESET << std::flush;

    return RunTimeFactory::makeNum(dynamic_cast<StringVal*>(args[0])->str.size() + 1);
};


RunTimeValue ConsoleLib::input(std::vector<RunTimeValue>& args, SigmaInterpreter*) {
    std::string str;
    std::getline(std::cin, str);
    return StringWrapper::genObject(RunTimeFactory::makeString(str));
};

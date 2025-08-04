#pragma once
#include "../StdLib.h"
#include <memory>
#include <string_view>
#include <unordered_map>

namespace ConsoleColors {

    using namespace std::literals::string_view_literals;

    constexpr std::string_view RESET = "\033[0m"sv;
    constexpr std::string_view BLACK  = "\033[30m"sv;      /* Black */
    constexpr std::string_view RED    = "\033[31m"sv;      /* Red */
    constexpr std::string_view GREEN  = "\033[32m"sv;      /* Green */
    constexpr std::string_view YELLOW = "\033[33m"sv;      /* Yellow */
    constexpr std::string_view BLUE   = "\033[34m"sv;      /* Blue */
    constexpr std::string_view MAGENTA= "\033[35m"sv;      /* Magenta */
    constexpr std::string_view CYAN   = "\033[36m"sv;      /* Cyan */
    constexpr std::string_view WHITE  = "\033[37m"sv;      /* White */

    constexpr std::string_view BG_BLACK   ="\033[40m"sv;
    constexpr std::string_view BG_RED     ="\033[41m"sv;
    constexpr std::string_view BG_GREEN   ="\033[42m"sv;
    constexpr std::string_view BG_YELLOW  ="\033[43m"sv;
    constexpr std::string_view BG_BLUE    ="\033[44m"sv;
    constexpr std::string_view BG_MAGENTA ="\033[45m"sv;
    constexpr std::string_view BG_CYAN    ="\033[46m"sv;
    constexpr std::string_view BG_WHITE   ="\033[47m"sv;

};
class ConsoleLib {
public:
    static std::unordered_map<std::string_view, std::string_view> colorMap;

    static ObjectVal* getStruct();

    static RunTimeValue print(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue println(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue input(std::vector<RunTimeValue>& args, SigmaInterpreter*);

};
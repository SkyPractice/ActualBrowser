#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std::literals::string_view_literals;

enum SigmaTokenType{
    Var, Number, Str, Bool, Char, Identifier, Lambda, 
    Quote, DoubleQuote, OpenBracket, CloseBracket, OpenParen, CloseParen,
    OpenBrace, CloseBrace, Enum, Import, ClassWord, Public, Private, Protected,
    Struct, True, False, If, ElseIf, Else, While, For, In, Equal,
    BinaryOperator, NameSpace, Const, ENDOFFILETOK, Comma, Continue, Break, Return, New,
    LambdaIndicator, Dot, Comment, Increment, Decrement, CompoundAssignmentOperator, Negative,
    Colon, Null, Function
};

struct SigmaToken {
    SigmaTokenType type;
    std::string symbol;
};

// The Lexer For The Actual Programming Language
class SigmaLexer {
public:

    std::unordered_map<std::string_view, SigmaTokenType> known_tokens = {
        {"var"sv, Var}, {"lambda"sv, Lambda}, {"\""sv, DoubleQuote}, {"\'"sv, Quote},
        {"["sv, OpenBracket}, {"]"sv, CloseBracket}, {"("sv, OpenParen}, {")"sv, CloseParen},
        {"{"sv, OpenBrace}, {"}"sv, CloseBrace}, {"enum"sv, Enum}, {"import"sv, Import},
        {"class"sv, ClassWord}, {"public"sv, Public}, {"private"sv, Private}, {"protected"sv, Protected},
        {"struct"sv, Struct}, {"true"sv, True}, {"false"sv, False}, {"if"sv, If}, {"elseif"sv, ElseIf},
        {"else"sv, Else}, {"while"sv, While}, {"for"sv, For}, {"in"sv, In},
         {"namespace"sv, NameSpace}, {"+"sv, BinaryOperator}, {"-"sv, BinaryOperator},
         {"*"sv, BinaryOperator}, {"/"sv, BinaryOperator}, {"%"sv, BinaryOperator},
         {"="sv, Equal},{"=="sv, BinaryOperator}, {">"sv, BinaryOperator}, {"<"sv, BinaryOperator},
         {">="sv, BinaryOperator}, {"<="sv, BinaryOperator}, {"&"sv, BinaryOperator},
         {">>"sv, BinaryOperator}, {"<<"sv, BinaryOperator}, {"|"sv, BinaryOperator},
         {"^"sv, BinaryOperator}, {"!="sv, BinaryOperator}, {"&&"sv, BinaryOperator},
         {"||"sv, BinaryOperator}, {"const"sv, Const}, {","sv, Comma},
         {"continue"sv, Continue}, {"break"sv, Break}, {"return"sv, Return}, {"new"sv, New},
         {"=>"sv, LambdaIndicator}, {"."sv, Dot}, {"//"sv, Comment}, {"++", Increment},
         {"--", Decrement}, {":", Colon}, {"null", Null}, {"function", Function}
    };
    std::unordered_set<char> skip_chars = {
        ' ', '\t', '\n', '\r'
    };
    std::vector<SigmaToken> tokenize(std::string& code);

    std::string *actual_code = nullptr;
    size_t current_pos = 0;
    char current_char = '0';

    void advance(){
        if(current_char != '\0' && current_pos < actual_code->size()){
            current_char = (*actual_code)[++current_pos];
        }
    }
};
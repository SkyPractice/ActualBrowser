#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum SigmaTokenType{
    Var, Number, Str, Bool, Char, Identifier, Lambda, 
    Quote, DoubleQuote, OpenBracket, CloseBracket, OpenParen, CloseParen,
    OpenBrace, CloseBrace, Enum, Import, ClassWord, Public, Private, Protected,
    Struct, True, False, If, ElseIf, Else, While, For, In, Equal,
    BinaryOperator, NameSpace, Const, ENDOFFILETOK, Comma, Continue, Break, Return, New,
    LambdaIndicator, Dot, Comment
};

struct SigmaToken {
    SigmaTokenType type;
    std::string symbol;
};

// The Lexer For The Actual Programming Language
class SigmaLexer {
public:
    std::unordered_map<std::string, SigmaTokenType> known_tokens = {
        {"var", Var}, {"lambda", Lambda}, {"\"", DoubleQuote}, {"\'", Quote},
        {"[", OpenBracket}, {"]", CloseBracket}, {"(", OpenParen}, {")", CloseParen},
        {"{", OpenBrace}, {"}", CloseBrace}, {"enum", Enum}, {"import", Import},
        {"class", ClassWord}, {"public", Public}, {"private", Private}, {"protected", Protected},
        {"struct", Struct}, {"true", True}, {"false", False}, {"if", If}, {"elseif", ElseIf},
        {"else", Else}, {"while", While}, {"for", For}, {"in", In},
         {"namespace", NameSpace}, {"+", BinaryOperator}, {"-", BinaryOperator},
         {"*", BinaryOperator}, {"/", BinaryOperator}, {"%", BinaryOperator},
         {"=", Equal},{"==", BinaryOperator}, {">", BinaryOperator}, {"<", BinaryOperator},
         {">=", BinaryOperator}, {"<=", BinaryOperator}, {"&", BinaryOperator},
         {">>", BinaryOperator}, {"<<", BinaryOperator}, {"|", BinaryOperator},
         {"^", BinaryOperator}, {"!=", BinaryOperator}, {"&&", BinaryOperator},
         {"||", BinaryOperator}, {"const", Const}, {",", Comma},
         {"continue", Continue}, {"break", Break}, {"return", Return}, {"new", New},
         {"=>", LambdaIndicator}, {".", Dot}, {"//", Comment}
    };
    std::unordered_set<char> skip_chars = {
        ' ', '\t', '\n', '\r'
    };
    std::vector<SigmaToken> tokenize(std::string code);

    std::string *actual_code = nullptr;
    size_t current_pos = 0;
    char current_char = '0';

    void advance(){
        if(current_char != '\0' && current_pos < actual_code->size()){
            current_char = (*actual_code)[++current_pos];
        }
    }
};
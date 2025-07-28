#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std::literals::string_view_literals;

enum StyleSheetTokenType {
    StyleClassSelector, StyleIdSelector, StyleIdentifier, StyleOpenBrace, StyleCloseBrace,
    StyleOpenParen, StyleCloseParen, StyleNumber, StylePx, StylePercentage, StyleEm,
    StyleEOF
};

struct StyleSheetToken {
    StyleSheetTokenType type;
    std::string symbol;
};

class StyleSheetLexer {
public:
    std::unordered_map<std::string_view, StyleSheetTokenType> known_style_tokens = {
        {".", StyleClassSelector}, {"#", StyleIdSelector}, {"{", StyleOpenBrace},
        {"}", StyleCloseBrace}, {"(", StyleOpenParen}, {")", StyleCloseParen},
    };
    

    std::unordered_set<char> skip_chars = {
        ' ', '\t', '\n', '\r'
    };
    std::vector<StyleSheetToken> tokenize(std::string& code);

    std::string *actual_code = nullptr;
    size_t current_pos = 0;
    char current_char = '0';

    void advance(){
        if(current_char != '\0' && current_pos < actual_code->size()){
            current_char = (*actual_code)[++current_pos];
        }
    }
};
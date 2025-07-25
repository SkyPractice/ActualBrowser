#pragma once
#include <string>
#include <unordered_map>

using namespace std::literals::string_view_literals;

enum StyleSheetTokenType {
    StyleClassSelector, StyleIdSelector, StyleIdentifier, StyleOpenBrace, StyleCloseBrace,
    StyleOpenParen, StyleCloseParen, StyleNumber, StylePx, StylePercentage, StyleEm
};


class StyleSheetLexer {
public:
    std::unordered_map<std::string_view, StyleSheetTokenType> known_style_tokens = {
        {".", StyleClassSelector}, {"#", StyleIdSelector}, {"{", StyleOpenBrace},
        {"}", StyleCloseBrace}, {"(", StyleOpenParen}, {")", StyleCloseParen},
    };
};
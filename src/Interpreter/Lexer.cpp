#include "Lexer.h"
#include <iostream>
#include <unordered_map>

std::vector<Token> Lexer::tokenize(std::string& code){
    current_code = &code;
    current_pos = 0;
    current_char = (*current_code)[0];

    std::vector<Token> tokens;

    while(current_pos < current_code->size() - 1 && current_char != '\0'){
        while(skip_chars.contains(current_char))
            advance();

        if(current_char == '<'){
            std::string str;
            while (current_char != '>' && current_char != '\0' && current_pos < current_code->size() - 1 &&
                current_char != ' ') {
                str.push_back(current_char);
                advance();
            }
            str.push_back('>');
            if(known_tokens.contains(str))
                tokens.push_back({ known_tokens[str], str });
            if(current_char == '>')advance();
            if(current_char == ' '){
                while (current_char != '>' && current_char != '\0' && current_pos < current_code->size() - 1) {
                        while(skip_chars.contains(current_char))
                            advance();

                        std::string prop_name;
                        while(current_char != '>' && current_char != '=' && current_char != '\0'){
                            prop_name.push_back(current_char);
                            advance();
                            
                        }
                        tokens.push_back({PROPNAME, prop_name});
                        if(current_char == '='){
                            advance();
                            if(current_char == '\"' || current_char == '\''){
                                advance();
                                std::string prop_val;
                                while(current_char != '\"' && current_char != '\''){
                                    prop_val.push_back(current_char);
                                    advance();
                                }
                                advance();
                                tokens.push_back({PROPVAL, prop_val});
                            }
                        }
                    }
            advance();

                }
        }
        else {
            std::string str;
            while (current_char != '\0' && current_pos < current_code->size() - 1 &&
                current_char != '<') {
                str.push_back(current_char);
                advance();
            }
            tokens.push_back({ STRING, str });
        }
    }
    tokens.push_back({ EndOfFile, "0" });
    return tokens;
};

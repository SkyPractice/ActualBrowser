#include "SigmaLexer.h"
#include <cctype>
#include <stdexcept>
#include <string>

std::vector<SigmaToken> SigmaLexer::tokenize(std::string code) {
    current_char = code[0];
    current_pos = 0;
    actual_code = &code;

    std::vector<SigmaToken> tokens;

    while(current_char != '\0' && current_pos < code.size() - 1){
        while(skip_chars.contains(current_char))
            advance();

        if(isalpha(current_char)){
            std::string str;
            while(isalpha(current_char)){
                str.push_back(current_char);
                advance();
            }

            if(known_tokens.contains(str))
                tokens.push_back({ known_tokens[str], str });
            else tokens.push_back({ Identifier, str });
        }
        else if (isdigit(current_char)){
            std::string str;

            while(isdigit(current_char) || current_char == '.' ){
                str.push_back(current_char);
                advance();
            }
            tokens.push_back({ Number, str });
        }
        else {
            std::string the_str;
            the_str.push_back(current_char);

            if((the_str == ">" || the_str == "<" || the_str == "!") && code[current_pos + 1] == '='){
                the_str.push_back('=');
                advance();
            } else if (the_str == ">" && code[current_pos + 1] == '>'){
                the_str.push_back('>');
                advance();
            } else if (the_str == "<" && code[current_pos + 1] == '<'){
                the_str.push_back('<');
                advance();
            } else if (the_str == "&" && code [current_pos + 1] == '&'){
                the_str.push_back('&');
                advance();
            } else if (the_str == "|" && code [current_pos + 1] == '|'){
                the_str.push_back('&');
                advance();
            } else if(the_str == "=" && code [current_pos + 1] == '='){
                the_str.push_back('=');
                advance();
            }

            if(known_tokens.contains(the_str))
                tokens.push_back({ known_tokens[the_str], the_str });
            else throw std::runtime_error("Unknown Sigma Token In Lexer Line " + 
                std::to_string(__LINE__) + ", Token " + the_str);
            
            advance();
        }
    }

    tokens.push_back({ ENDOFFILETOK, "0" });
    actual_code = nullptr;
    current_char = '0';
    current_pos = 0;
};

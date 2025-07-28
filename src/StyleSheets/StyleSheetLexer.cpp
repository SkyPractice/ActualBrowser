#include "StyleSheetLexer.h"

std::vector<StyleSheetToken> StyleSheetLexer::tokenize(std::string& code){
    current_char = code[0];
    current_pos = 0;
    actual_code = &code;

    std::vector<StyleSheetToken> tokens;
    
    while(current_char != '\0' && current_pos < actual_code->size()){
        while(skip_chars.contains(current_char) && current_char < '\0')
            advance();
    }

    actual_code = nullptr;
    current_pos = 0;
    current_char = '0';
};
#pragma once
#include <cctype>
#include <cstring>
#include <string>
#include <unordered_set>
#include <fstream>

class PreProcessor {
public:

    std::string* current_code = nullptr;
    std::string code_dir;
    size_t current_index = 0;
    char current_char = '0';
    std::unordered_set<char> skip_chars = {
        '\n', ' ', '\t', '\r'
    };

    void advance(){
        if(current_index < current_code->size() && current_char != '\0'){
            current_char = (*current_code)[++current_index];
        }
    }

    std::string readFile(std::string file_n){
        std::ifstream strea(file_n, std::ios::ate);
        size_t s = strea.tellg();
        std::string strr;
        strr.resize(s);
        strea.seekg(0, std::ios::beg);
        strea.read(&strr[0], s);
        strea.close();
        return strr;
    }

    void processCode(std::string& code){
        current_code = &code;
        current_index = 0;
        current_char = code[0];

        while(current_index < code.size() && current_char != '\0'){
            if(current_char == '#'){
                int starting_index = current_index;
                advance();
                if(code.size() - current_index < strlen("preprocessor include")) break;
                std::string_view preprocessor_word_view(&code[current_index],
                    strlen("preprocessor"));
                if(preprocessor_word_view == "preprocessor"){
                    for(int i = 0; i < strlen("preprocessor"); i++)
                        advance();
                    if(skip_chars.contains(current_char))
                        advance();

                    std::string_view include_word_view(&code[current_index],
                        strlen("include"));
                    if(include_word_view == "include"){
                        for(int i = 0; i < strlen("include"); i++)
                            advance();
                    }
                    while(skip_chars.contains(current_char))
                        advance();
                    advance(); // through the first "

                    std::string file_name;
                    while(current_char != '\"'){
                        file_name.push_back(current_char);
                        advance();
                    }
                    
                    advance(); // through the second "

                    int ending_index = current_index;

                    current_index = starting_index;
                    if(!code_dir.empty())
                    code.replace(starting_index, ending_index - starting_index,
                        readFile(code_dir + "/" + file_name));
                    else code.replace(starting_index, ending_index - starting_index,
                        readFile(file_name));

                }
            } else advance();
            
        }
        current_code = nullptr;
        current_char = '0';
        current_index = 0;
    }



};
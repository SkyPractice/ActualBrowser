#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>

enum TokenType {
    DOCKTYPE, HTML, HEAD, BODY, H1, H2, H3, H4, H5, P, STRING,
    CLOSEHTML, CLOSEHEAD, CLOSEH1, CLOSEH2, CLOSEH3, CLOSEH4, CLOSEH5, CLOSEP,
    CLOSEBODY, EndOfFile, PROPNAME, PROPVAL, IMAGE, CLOSEIMAGE,
    BUTTON, CLOSEBUTTON, INPUT, CLOSEINPUT, DIV, CLOSEDIV, OPENSTYLE, CLOSESTYLE,
    OPENSCRIPT, CLOSESCRIPT
};

struct Token {
    TokenType type;
    std::string symbol;
};

class Lexer {
public:
    std::unordered_map<std::string, TokenType> known_tokens = {
        { "<html>", HTML }, { "<!docktype html>", DOCKTYPE },
        { "<head>", HEAD },{ "<body>", BODY },{ "<h1>", H1 },{ "<h2>", H2 },
        { "<h3>", H3 },{ "<h4>", H4 },{ "<h5>", H5 },{ "<p>", P },
        { "</html>", CLOSEHTML },
        { "</head>", CLOSEHEAD },{ "</body>", CLOSEBODY },{ "</h1>", CLOSEH1 },
        { "</h2>", CLOSEH2 },
        { "</h3>", CLOSEH3 },{ "</h4>", CLOSEH4 },{ "</h5>", CLOSEH5 },
        { "</p>", CLOSEP }, {"<img>", IMAGE}, {"</img>", CLOSEIMAGE},
        {"<button>", BUTTON}, {"</button>", CLOSEBUTTON}, {"<input>", INPUT},
         {"</input>", CLOSEINPUT}, {"<div>", DIV}, {"</div>", CLOSEDIV},
         {"<script>", OPENSCRIPT}, {"</script>", CLOSESCRIPT },
         {"<style>", OPENSTYLE}, {"</style>", CLOSESTYLE}
    };
    std::unordered_set<char> skip_chars = {
        ' ', '\t', '\r', '\n'
    };
    


    std::vector<Token> tokenize(std::string code);
    char current_char = '0';
    size_t current_pos = 0;
    std::string* current_code = nullptr;

    void advance(){
        if(current_pos < current_code->size() - 1 && current_char != '\0'){
            current_char = (*current_code)[++current_pos];
        }
    };
};
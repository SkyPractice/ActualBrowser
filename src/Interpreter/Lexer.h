#pragma once
#include <string_view>
#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std::literals::string_view_literals;

enum TokenType {
    DOCKTYPE, HTML, HEAD, BODY, H1, H2, H3, H4, H5, P, STRING,
    CLOSEHTML, CLOSEHEAD, CLOSEH1, CLOSEH2, CLOSEH3, CLOSEH4, CLOSEH5, CLOSEP,
    CLOSEBODY, EndOfFile, PROPNAME, PROPVAL, IMAGE, CLOSEIMAGE,
    BUTTON, CLOSEBUTTON, INPUT, CLOSEINPUT, DIV, CLOSEDIV, OPENSTYLE, CLOSESTYLE,
    OPENSCRIPT, CLOSESCRIPT, SPAN, HEADER, NAV, FOOTER, MAIN, ARTICLE, ASIDE, SECTION,
    CLOSESPAN, CLOSEHEADER, CLOSENAV, CLOSEFOOTER, CLOSEMAIN, CLOSEARTICLE, CLOSEASIDE, CLOSESECTION,
    VIDEO, CLOSEVIDEO
};

struct Token {
    TokenType type;
    std::string symbol;
};

class Lexer {
public:
    std::unordered_map<std::string_view, TokenType> known_tokens = {
        { "<html>"sv, HTML }, { "<!docktype html>"sv, DOCKTYPE },
        { "<head>"sv, HEAD },{ "<body>"sv, BODY },{ "<h1>"sv, H1 },{ "<h2>"sv, H2 },
        { "<h3>"sv, H3 },{ "<h4>"sv, H4 },{ "<h5>"sv, H5 },{ "<p>"sv, P },
        { "</html>"sv, CLOSEHTML },
        { "</head>"sv, CLOSEHEAD },{ "</body>"sv, CLOSEBODY },{ "</h1>"sv, CLOSEH1 },
        { "</h2>"sv, CLOSEH2 },
        { "</h3>"sv, CLOSEH3 },{ "</h4>"sv, CLOSEH4 },{ "</h5>"sv, CLOSEH5 },
        { "</p>"sv, CLOSEP }, {"<img>"sv, IMAGE}, {"</img>"sv, CLOSEIMAGE},
        {"<button>"sv, BUTTON}, {"</button>"sv, CLOSEBUTTON}, {"<input>"sv, INPUT},
         {"</input>"sv, CLOSEINPUT}, {"<div>"sv, DIV}, {"</div>"sv, CLOSEDIV},
         {"<script>"sv, OPENSCRIPT}, {"</script>"sv, CLOSESCRIPT },
         {"<style>"sv, OPENSTYLE}, {"</style>"sv, CLOSESTYLE},
         {"<span>", SPAN}, {"<nav>", NAV}, {"<header>", HEADER}, {"<footer>", FOOTER},
        {"<main>", MAIN},{"<article>", ARTICLE}, {"<aside>", ASIDE},
        {"<section>", SECTION}, {"</span>", CLOSESPAN}, {"</nav>", CLOSENAV},
         {"</header>", CLOSEHEADER}, {"</footer>", CLOSEFOOTER}, {"</main>", CLOSEMAIN},
         {"</article>", CLOSEARTICLE}, {"</aside>", CLOSEASIDE}, {"</section>", CLOSESECTION},
         {"<video>", VIDEO}, {"</video>", CLOSEVIDEO}
    };
    std::unordered_set<char> skip_chars = {
        ' ', '\t', '\r', '\n'
    };
    


    std::vector<Token> tokenize(std::string& code);
    char current_char = '0';
    size_t current_pos = 0;
    std::string* current_code = nullptr;

    void advance(){
        if(current_pos < current_code->size() - 1 && current_char != '\0'){
            current_char = (*current_code)[++current_pos];
        }
    };
};
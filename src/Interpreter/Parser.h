#pragma once
#include "Ast.h"
#include "Lexer.h"
#include <exception>
#include <memory>
#include <stdexcept>
#include <unordered_map>

using Tag = std::shared_ptr<HTMLTag>;

class Parser {
public:
    Tag parseDocumentHtmlTag();

    Tag parseBodyTag();
    Tag parseHTag(TokenType type);
    Tag parsePTag();
    Tag parseString();
    Tag parseImageTag();
    Tag parseInputTag();
    Tag parseButtonTag();
    Tag parseDivTag();
    Tag parseStyleTag();
    Tag parseScriptTag();

    std::unordered_map<std::string, std::string> parseProps();

    std::shared_ptr<HTMLTag> parseHtmlTag();

    Program produceAst(std::vector<Token> tok_vec){
        itr = tok_vec.begin();
        std::vector<std::shared_ptr<HTMLTag>> tag_vec;
        while(itr->type != EndOfFile){
            tag_vec.push_back(parseHtmlTag());
        }

        return { tag_vec, {}, {} };
    }


    std::vector<Token>::iterator itr;

    Token advance(){
        if(itr->type != EndOfFile){
            Token tok = *itr;
            itr++;
            return tok;
        }
        return *itr;
    }
};
#pragma once
#include "Ast.h"
#include "Lexer.h"
#include <memory>
#include <unordered_map>

using Tag = std::shared_ptr<HTMLTag>;

class Parser {
public:
    std::string current_p_tag_cl_name = "body";

    Tag parseDocumentHtmlTag();

    Tag parseBodyTag();
    Tag parseHTag(TokenType type);
    Tag parsePTag();
    Tag parseString(std::string p_class_name = "p");
    Tag parseImageTag();
    Tag parseInputTag();
    Tag parseButtonTag();
    Tag parseDivTag();
    Tag parseStyleTag();
    Tag parseScriptTag();
    Tag parseVideoTag();
    template<typename ContainerType, TokenType CloseTokenType>
    Tag parseContainerTag(){
        const Token html_tok = advance();
        const auto tag = std::make_shared<ContainerType>();
        tag->props = parseProps();

        while(itr->type != CloseTokenType){
            tag->children.push_back(parseHtmlTag());
        }
        advance();
        return tag;
    }

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
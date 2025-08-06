#include "Parser.h"
#include "Ast.h"
#include "Lexer.h"
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

std::shared_ptr<HTMLTag> Parser::parseHtmlTag() {
  switch (itr->type) {
  case HTML:
    return parseDocumentHtmlTag();
  case BODY:
    return parseBodyTag();
  case H1:
  case H2:
  case H3:
  case H4:
  case H5:
    return parseHTag(itr->type);
  case P:
    return parsePTag();
  case STRING:
    return parseString();
  case BUTTON:
    return parseButtonTag();
  case INPUT:
    return parseInputTag();
  case IMAGE:
    return parseImageTag();
  case DIV:
    return parseDivTag();
  case SPAN:
    return parseContainerTag<SpanTag, CLOSESPAN>();
  case HEADER:
    return parseContainerTag<HeaderTag, CLOSEHEADER>();
  case NAV:
    return parseContainerTag<NavTag, CLOSENAV>();
  case FOOTER:
    return parseContainerTag<FooterTag, CLOSEFOOTER>();
  case MAIN:
    return parseContainerTag<MainTag, CLOSEMAIN>();
  case ARTICLE:
    return parseContainerTag<ArticleTag, CLOSEARTICLE>();
  case ASIDE:
    return parseContainerTag<AsideTag, CLOSEASIDE>();
  case SECTION:
    return parseContainerTag<SectionTag, CLOSESECTION>();
  case OPENSTYLE:
    return parseStyleTag();
  case OPENSCRIPT:
    return parseScriptTag();
  case VIDEO:
    return parseVideoTag();
  default:
    throw std::runtime_error(("Unknown Token " + itr->symbol).c_str());
  }
};

Tag Parser::parseDocumentHtmlTag(){
    const Token html_tok = advance();
    const auto tag = std::make_shared<HTMLTag>(Html, "html", "html");
    tag->props = parseProps();

    while(itr->type != CLOSEHTML){
        tag->children.push_back(parseHtmlTag());
    }
    advance();
    return tag;
};
Tag Parser::parseImageTag(){
    const Token html_tok = advance();
    const auto tag = std::make_shared<ImageTag>();
    tag->props = parseProps();
    
    advance();
    return tag;
};
Tag Parser::parseVideoTag(){
    const Token html_tok = advance();
    const auto tag = std::make_shared<VideoTag>();
    tag->props = parseProps();
    
    advance();
    return tag;
}
Tag Parser::parseBodyTag(){
    const Token html_tok = advance();
    const auto tag = std::make_shared<BodyTag>();
    tag->props = parseProps();

    while(itr->type != CLOSEBODY){
        tag->children.push_back(parseHtmlTag());
    }
    advance();
    return tag;
};
Tag Parser::parseDivTag(){
    const Token html_tok = advance();
    const auto tag = std::make_shared<DivTag>();
    tag->props = parseProps();

    while(itr->type != CLOSEDIV){
        tag->children.push_back(parseHtmlTag());
    }
    advance();
    return tag;
};
Tag Parser::parseHTag(TokenType t){
    const Token h_tok = advance();
    if (t == H1) {
        const auto tag = std::make_shared<H1Tag>();
        tag->props = parseProps();

        while (itr->type != CLOSEH1) {
            tag->children.push_back(parseHtmlTag());
        }
        advance();
        return tag;
    }
    if (t == H2) {
        const auto tag = std::make_shared<H2Tag>();
        tag->props = parseProps();

        while (itr->type != CLOSEH2) {
            tag->children.push_back(parseHtmlTag());
        }
        advance();
        return tag;
    }
    if (t == H3) {
        const auto tag = std::make_shared<H3Tag>();
        tag->props = parseProps();

        while (itr->type != CLOSEH3) {
            tag->children.push_back(parseHtmlTag());
        }
        advance();
        return tag;
    }
    if (t == H4) {
        const auto tag = std::make_shared<H4Tag>();
        tag->props = parseProps();

        while (itr->type != CLOSEH4) {
            tag->children.push_back(parseHtmlTag());
        }
        advance();
        return tag;
    }
    if (t == H5) {
        const auto tag = std::make_shared<H5Tag>();
        tag->props = parseProps();

        while (itr->type != CLOSEH5) {
            tag->children.push_back(parseHtmlTag());
        }
        advance();
        return tag;
    }

    throw std::runtime_error("mister goofy sigma ahh");
};

Tag Parser::parsePTag(){
    const Token tok = advance();
    const auto tag = std::make_shared<PTag>();
    tag->props = parseProps();

    while (itr->type != CLOSEP) {
        tag->children.push_back(parseHtmlTag());
    }
    advance();
    return tag;
    
};

Tag Parser::parseString(std::string p_class_name){
    const Token str = advance();

    return std::make_shared<StringTag>(str.symbol);
}

std::unordered_map<std::string, std::string> Parser::parseProps(){
    std::unordered_map<std::string, std::string> map;

    while(itr->type == PROPNAME){
        std::string name = advance().symbol;
        std::string val;
        if(itr->type == PROPVAL) { val = advance().symbol; };
        map.insert({name, val});
    }

    return map;

};

Tag Parser::parseInputTag(){
    const Token tok = advance();
    const auto tag = std::make_shared<InputTag>();
    tag->props = parseProps();
    advance();

    return tag;
};
Tag Parser::parseButtonTag(){
    const Token tok = advance();
    const auto tag = std::make_shared<ButtonTag>();
    tag->props = parseProps();
    while(itr->type != CLOSEBUTTON){
        const auto html_tag = parseHtmlTag();
        if(html_tag->tag_information.type == String){
            tag->str += std::dynamic_pointer_cast<StringTag>(html_tag)->str;
        };
    }
    advance();

    return tag;
};

Tag Parser::parseStyleTag() {
    const Token tok = advance();
    const auto tag = std::make_shared<StyleTag>();
    tag->props = parseProps();
    for(auto& prop : tag->props){
        if(prop.first == "src"){
            tag->src = prop.second;
        }
    }
    advance();
    return tag;
};
Tag Parser::parseScriptTag() {
    const Token tok = advance();
    const auto tag = std::make_shared<ScriptTag>();
    tag->props = parseProps();
    for(auto& prop : tag->props){
        if(prop.first == "src"){
            tag->src = prop.second;
        }
    }
    advance();
    return tag;
};
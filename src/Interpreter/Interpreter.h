#pragma once
#include "Ast.h"
#include "Parser.h"
#include <fstream>
#include <gdkmm/display.h>
#include <glibmm/refptr.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/cssprovider.h>
#include <glibmm/refptr.h>
#include <memory>
#include <filesystem>
#include "../SigmaInterpreter/SigmaLexer.h"
#include "../SigmaInterpreter/SigmaParser.h"
#include "../SigmaInterpreter/SigmaInterpreter.h"
#include <iostream>

namespace fs = std::filesystem;

class Interpreter {
public:
    SigmaLexer scripting_lexer;
    SigmaParser scripting_parser;
    SigmaInterpreter scripting_interpreter;
    Lexer lexer;
    Parser parser;

    std::vector<Glib::RefPtr<Gtk::CssProvider>> css_providers;

    void renderTags(Gtk::Box* target_box, Program tags) {
        for(auto& tag : tags.html_tags){
            tag->render(target_box);
        }
        std::vector<std::shared_ptr<HTMLTag>> flattened_tags;
        for(auto& tag : tags.html_tags){
            tag->flatten(flattened_tags);
        }

        for(auto& tag : flattened_tags){
            if(tag->type == Stylee){
                auto style_tag = std::dynamic_pointer_cast<StyleTag>(tag);
                if(fs::exists(style_tag->src)){
                    auto provider = Gtk::CssProvider::create();
                    provider->load_from_path(style_tag->src);
                    Gtk::CssProvider::add_provider_for_display(Gdk::Display::get_default(), provider,
                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                     css_providers.push_back(provider);
                }
            } else if (tag->type == Scriptt){
                auto script_tag = std::dynamic_pointer_cast<ScriptTag>(tag);
                if(fs::exists(script_tag->src)){
                    std::ifstream file_stream(script_tag->src, std::ios::ate);
                    std::string code;

                    size_t size = file_stream.tellg();

                    code.resize(size);
                    file_stream.seekg(0, std::ios::beg);
                    file_stream.read(&code[0], size);

                    file_stream.close();

                    std::cout << "Reached Lexing" << std::endl;
                    auto tokens = scripting_lexer.tokenize(code);
                    std::cout << "Successfully Lexed" << std::endl;
                    auto ast = scripting_parser.produceAst(tokens);
                    std::cout << "Successfully parsed" << std::endl;
                    auto result = scripting_interpreter.evaluate(ast);
                }
            }
        }

    };

    void reset(){
        for(auto& prov : css_providers){
            Gtk::CssProvider::remove_provider_for_display(Gdk::Display::get_default(),
                prov);
        }
        css_providers.clear();
    }



};
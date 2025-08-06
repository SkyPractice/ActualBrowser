#pragma once
#include "Ast.h"
#include "Parser.h"
#include <fstream>
#include <gdkmm/display.h>
#include <glibmm/refptr.h>
#include <gtkmm/box.h>
#include <gtkmm/enums.h>
#include <gtkmm/label.h>
#include <gtkmm/cssprovider.h>
#include <glibmm/refptr.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/window.h>
#include <memory>
#include <filesystem>
#include "../SigmaInterpreter/SigmaLexer.h"
#include "../SigmaInterpreter/SigmaParser.h"
#include "../SigmaInterpreter/SigmaInterpreter.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../SigmaInterpreter/GarbageCollector/GarbageCollector.h"
#include "../SigmaInterpreter/Util/Permissions/Permissions.h"
#include "../SigmaInterpreter/PreProcessor/PreProcessor.h"
#include "../SigmaInterpreter/StandardLibrary/WindowLib/WindowLib.h"

namespace fs = std::filesystem;

class Interpreter;

struct DOMAccessor {
    std::unordered_multimap<std::string, std::shared_ptr<HTMLTag>> class_name_ptrs;
    std::unordered_map<std::string, std::shared_ptr<HTMLTag>> id_ptrs;
    Interpreter* current_interp;
};

class Interpreter {
public:
    SigmaLexer scripting_lexer;
    SigmaParser scripting_parser;
    SigmaInterpreter scripting_interpreter;
    Gtk::Window* target_window = nullptr;
    Lexer lexer;
    Parser parser;

    std::vector<Glib::RefPtr<Gtk::CssProvider>> css_providers;
    std::vector<std::shared_ptr<HTMLTag>> current_tags;

    DOMAccessor accessor;

    void refreshIdsAndClasses(){
        std::vector<std::shared_ptr<HTMLTag>> flattened_tags;
        for(auto& tag : current_tags){
            tag->flatten(flattened_tags);
        }
        std::unordered_multimap<std::string, std::shared_ptr<HTMLTag>> class_name_ptrs;
        std::unordered_map<std::string, std::shared_ptr<HTMLTag>> id_ptrs;

        for(auto& tag : flattened_tags){
            if(tag->props.contains("id"))
                id_ptrs.insert({tag->props["id"], tag});
            if (tag->props.contains("class"))
                for(auto& cl : tag->getClassNames())
                    class_name_ptrs.insert({cl, tag});
        }
        accessor = {.class_name_ptrs=class_name_ptrs, .id_ptrs=id_ptrs, .current_interp=this};
        scripting_interpreter.accessor = &accessor;
    }
    void renderTags(Gtk::Box* target_box, Program tags, std::string document_or_host) {
        PermissionContainer old_perms;
        std::string perms_file_path = "./Config/Permissions/" + document_or_host;
        if(std::filesystem::exists(perms_file_path)){
            old_perms = PermissionFileController::readPermsFromFile(perms_file_path);
        }
        reset();
        current_tags.clear();
        for(auto& tag : tags.html_tags){
            tag->render(target_box);
        }
        std::vector<std::shared_ptr<HTMLTag>> flattened_tags;
        for(auto& tag : tags.html_tags){
            tag->flatten(flattened_tags);
        }
        std::unordered_multimap<std::string, std::shared_ptr<HTMLTag>> class_name_ptrs;
        std::unordered_map<std::string, std::shared_ptr<HTMLTag>> id_ptrs;

        for(auto& tag : flattened_tags){
            if(tag->props.contains("id"))
                id_ptrs.insert({tag->props["id"], tag});
            if (tag->props.contains("class"))
                for(auto& cl : tag->getClassNames())
                    class_name_ptrs.insert({cl, tag});
        }
        accessor = { class_name_ptrs, id_ptrs, this };
        current_tags = tags.html_tags;
        for(auto& tag : flattened_tags){
            if(tag->tag_information.type == Stylee){
                auto style_tag = std::dynamic_pointer_cast<StyleTag>(tag);
                size_t index = document_or_host.rfind('/');
                if(index != std::string::npos){
                    std::string new_src = document_or_host.substr(0, index) + "/" + style_tag->src;
                    style_tag->src = std::move(new_src);
                }
                if(fs::exists(style_tag->src)){
                    auto provider = Gtk::CssProvider::create();
                    provider->load_from_path(style_tag->src);
                    Gtk::CssProvider::add_provider_for_display(Gdk::Display::get_default(), provider,
                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                     css_providers.push_back(provider);
                }
            } else if (tag->tag_information.type == Scriptt){
                auto script_tag = std::dynamic_pointer_cast<ScriptTag>(tag);
                
                size_t index = document_or_host.rfind('/');
                if(index != std::string::npos){
                    std::string new_src = document_or_host.substr(0, index) + "/" + script_tag->src;
                    script_tag->src = std::move(new_src);
                }
                
                if(fs::exists(script_tag->src)){
                    std::ifstream file_stream(script_tag->src, std::ios::ate);
                    std::string code;

                    size_t size = file_stream.tellg();

                    code.resize(size);
                    file_stream.seekg(0, std::ios::beg);
                    file_stream.read(&code[0], size);

                    file_stream.close();

                    PreProcessor preprocessor;
                    std::string& src = script_tag->src;
                    if(src.rfind('/') != std::string::npos){
                        std::string parent_dir = src.substr(0, src.rfind('/'));
                        preprocessor.code_dir = parent_dir;
                    }
                    else { preprocessor.code_dir = "./"; }

                    preprocessor.processCode(code);

                    std::cout << "Reached Lexing" << std::endl;
                    auto tokens = scripting_lexer.tokenize(code);
                    std::cout << "Successfully Lexed" << std::endl;
                    auto ast = scripting_parser.produceAst(tokens);
                    std::cout << "Successfully parsed" << std::endl;

                    scripting_interpreter.current_window = target_window;
                    scripting_interpreter.accessor = &accessor;
                    scripting_interpreter.initialize();
                    scripting_interpreter.perms = old_perms;
                    scripting_interpreter.doc_name = document_or_host;

                    auto result = scripting_interpreter.evaluate(ast);

                    if(result->type == StringType){
                        auto casted_result = static_cast<StringVal*>(result);
                        WindowLib::showAlert({"Error While Executing A Script",
                            casted_result->str, target_window, 
                            {{"ok", [](Gtk::Dialog* dialo){dialo->close();}}}});
                    }
                    std::vector<RunTimeVal*> mark_vals = scripting_interpreter.getAccessibleValues();
                    GarbageCollector::mark(mark_vals);
                    GarbageCollector::sweep(RunTimeMemory::pool);
                    old_perms = scripting_interpreter.perms;
                }
            }
        }
        current_tags = tags.html_tags;
        PermissionFileController::writePermsToFile(perms_file_path, old_perms);
    };

    void reset(){
        for(auto& prov : css_providers){
            Gtk::CssProvider::remove_provider_for_display(Gdk::Display::get_default(),
                prov);
        }
        css_providers.clear();

        GarbageCollector::alive_vals.clear();
        RunTimeMemory::pool.release();
        SigmaParser::memory_pool.release();
    }

    std::shared_ptr<HTMLTag> cloneHtmlTagWithoutRendering() {
        return nullptr;
    };


};
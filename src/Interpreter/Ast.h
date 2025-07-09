#pragma once
#include "Lexer.h"
#include <glibmm/refptr.h>
#include <gtk/gtk.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/entry.h>
#include <gtkmm/enums.h>
#include <gtkmm/image.h>
#include <gtkmm/object.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <filesystem>
#include <iostream>
#include <gtkmm/button.h>
#include <glibmm/refptr.h>
#include <uuid/uuid.h>

enum TagType {
    Html, Body, Head, h1, h2, h3, h4, h5, p, String, Image, Input, Button, Div, Stylee, Scriptt
};


class HTMLTag : public std::enable_shared_from_this<HTMLTag>{
public:
    TagType type;
    std::vector<std::shared_ptr<HTMLTag>> children;
    std::unordered_map<std::string, std::string> props;
    Glib::RefPtr<Gtk::CssProvider> css_provider;

    HTMLTag(TagType t): type(t) {};

    virtual void render(Gtk::Box* box) {
        for(auto& child : children){
            child->render(box);
        }
    }

    void flatten(std::vector<std::shared_ptr<HTMLTag>>& tags){
        tags.push_back(shared_from_this());

        for(auto& child : children){
            child->flatten(tags);
        }
    }
};

class Program {
public:
    std::vector<std::shared_ptr<HTMLTag>> html_tags;
    std::vector<std::string> style_srcs;
    std::vector<std::string> script_srcs;
};

class BodyTag : public HTMLTag {
public:
    BodyTag(): HTMLTag(Body) {};
    Gtk::Box* bxx;

    void render(Gtk::Box* bx) override {
        bxx = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
        bx->append(*bxx);
        if(props.contains("class")) bxx->add_css_class(props["class"]);
        if(props.contains("style")){
            css_provider = Gtk::CssProvider::create();
            css_provider->load_from_data("box {" + props["style"] + "}");
            bxx->get_style_context()->add_provider(css_provider,
                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        bxx->add_css_class("body");
        for(auto& child : children){
            child->render(bxx);
        }
    };
};

class DivTag : public HTMLTag {
public:
    DivTag(): HTMLTag(Div) {};
    Gtk::Box* bxx;

    void render(Gtk::Box* bx) override {
        bxx = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
        bxx->set_hexpand(true);
        bxx->set_valign(Gtk::Align::START);
        bxx->set_halign(Gtk::Align::START);
        bx->append(*bxx);
        if(props.contains("class")) bxx->add_css_class(props["class"]);
        if(props.contains("style")){
            css_provider = Gtk::CssProvider::create();
            css_provider->load_from_data("box {" + props["style"] + "}");
            bxx->get_style_context()->add_provider(css_provider,
                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        bxx->add_css_class("div");
        for(auto& child : children){
            child->render(bxx);
        }
    };
};

class HeadTag : public HTMLTag {
public:
    HeadTag(): HTMLTag(Head) {};

    void render(Gtk::Box* box)  override {
    };
};


class StringTag : public HTMLTag {
public:
    std::string str;
    StringTag(std::string stringg): HTMLTag(String), str(stringg) {};
    Gtk::Label* lab;

    void render(Gtk::Box* targ_box)  override {
        Gtk::Label* lab = new Gtk::Label(str);

        lab->set_hexpand(true);
        lab->set_valign(Gtk::Align::START);
        lab->set_halign(Gtk::Align::START);
        lab->add_css_class("p");
        targ_box->append(*lab);
    }
    void renderText(TagType class_type, Gtk::Box* targ_box){

        lab = Gtk::manage(new Gtk::Label(str));
        if(class_type == h1)
            lab->add_css_class("h1");
        if(class_type == h2)
            lab->add_css_class("h2");
        if(class_type == h3)
            lab->add_css_class("h3");
        if(class_type == h4)
            lab->add_css_class("h4");
        if(class_type == h5)
            lab->add_css_class("h5");
        if(class_type == p)
            lab->add_css_class("p");
        
        lab->set_hexpand(true);
        lab->set_valign(Gtk::Align::START);
        lab->set_halign(Gtk::Align::START);
        if(props.contains("class")) lab->add_css_class(props["class"]);
        if(props.contains("style")){
            css_provider = Gtk::CssProvider::create();
            css_provider->load_from_data("label {" + props["style"] + "}");
            lab->get_style_context()->add_provider(css_provider,
                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        if(props.contains("width")){
            lab->set_max_width_chars(std::stoi(props["width"]));
        }
        lab->set_wrap();
        lab->set_natural_wrap_mode(Gtk::NaturalWrapMode::WORD);
        targ_box->append(*lab);
    }
};


class H1Tag : public HTMLTag {
public:
    H1Tag(): HTMLTag(h1) {};

    void render(Gtk::Box* box)  override {
        for(auto& child : children){
            if(child->type == String){
                auto real_child = std::dynamic_pointer_cast<StringTag>(child);
                if(props.contains("class")) real_child->props["class"] = props["class"]; 
                if(props.contains("style")) real_child->props["style"] = props["style"];
                if(props.contains("width")) real_child->props["width"] = props["width"];
                real_child->renderText(h1, box);
            }
        }
    }
};

class H2Tag : public HTMLTag {
public:
    H2Tag(): HTMLTag(h2) {};

    void render(Gtk::Box* box)  override {
        for(auto& child : children){
            if(child->type == String){
                auto real_child = std::dynamic_pointer_cast<StringTag>(child);
                if(props.contains("class")) real_child->props["class"] = props["class"]; 
                if(props.contains("style")) real_child->props["style"] = props["style"];
                if(props.contains("width")) real_child->props["width"] = props["width"];
                real_child->renderText(h2, box);
            }
        }
    }
};

class H3Tag : public HTMLTag {
public:
    H3Tag(): HTMLTag(h3) {};

    void render(Gtk::Box* box)  override {
        for(auto& child : children){
            if(child->type == String){
                auto real_child = std::dynamic_pointer_cast<StringTag>(child);
                if(props.contains("class")) real_child->props["class"] = props["class"];
                if(props.contains("style")) real_child->props["style"] = props["style"];
                if(props.contains("width")) real_child->props["width"] = props["width"];
                real_child->renderText(h3, box);
            }
        }
    }
};

class H4Tag : public HTMLTag {
public:
    H4Tag(): HTMLTag(h4) {};

    void render(Gtk::Box* box)  override {
        for(auto& child : children){
            if(child->type == String){
                auto real_child = std::dynamic_pointer_cast<StringTag>(child);
                if(props.contains("class")) real_child->props["class"] = props["class"]; 
                if(props.contains("style")) real_child->props["style"] = props["style"];
                if(props.contains("width")) real_child->props["width"] = props["width"];
                real_child->renderText(h4, box);
            }
        }
    }
};

class H5Tag : public HTMLTag {
public:
    H5Tag(): HTMLTag(h5) {};

    void render(Gtk::Box* box)  override {
        for(auto& child : children){
            if(child->type == String){
                auto real_child = std::dynamic_pointer_cast<StringTag>(child);
                if(props.contains("class")) real_child->props["class"] = props["class"];
                if(props.contains("style")) real_child->props["style"] = props["style"];
                if(props.contains("width")) real_child->props["width"] = props["width"];
                real_child->renderText(h5, box);
            }
        }
    }
};


class PTag : public HTMLTag {
public:
    PTag(): HTMLTag(p) {};

    void render(Gtk::Box* box) override {
        for(auto& child : children){
            if(child->type == String){
                auto real_child = std::dynamic_pointer_cast<StringTag>(child);
                if(props.contains("class")) real_child->props["class"] = props["class"]; 
                if(props.contains("style")) real_child->props["style"] = props["style"];
                if(props.contains("width")) real_child->props["width"] = props["width"];
                real_child->renderText(p, box);
            }
        }
    }
};

class ImageTag : public HTMLTag {
public:
    ImageTag(): HTMLTag(Image) {};
    Gtk::Image* image;

    void render(Gtk::Box* box) override {
        if(props.contains("src")){
            if(std::filesystem::exists(props["src"])){
                std::cout << "image exists" << '\n';
                image = Gtk::manage(new Gtk::Image(props["src"]));
                std::cout << props["src"] << '\n';
                image->set_halign(Gtk::Align::START);
                image->set_hexpand(false);
                image->add_css_class("img");
                if (props.contains("class"))
                  image->add_css_class(props["class"]);
                if (props.contains("style")) {
                  css_provider = Gtk::CssProvider::create();
                  css_provider->load_from_data("image {" + props["style"] +
                                               "}");
                  image->get_style_context()->add_provider(
                      css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                }
                box->append(*image);
            }
        }
    }

};

class InputTag : public HTMLTag {
public:
    InputTag(): HTMLTag(Input){};
    Gtk::Entry* input;
    void render(Gtk::Box* box) override {
        input = Gtk::manage(new Gtk::Entry);
        input->add_css_class("input");
        if(props.contains("class")) input->add_css_class(props["class"]); 
        input->set_halign(Gtk::Align::START);
        input->set_hexpand(false);
                if(props.contains("style")){
            css_provider = Gtk::CssProvider::create();
            css_provider->load_from_data("entry {" + props["style"] + "}");
            input->get_style_context()->add_provider(css_provider,
                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        box->append(*input);
    }
};


class ButtonTag : public HTMLTag {
public:
    ButtonTag(): HTMLTag(Button) {};
    Gtk::Button* button;
    std::string str;
    
    void render(Gtk::Box* box) override {
        button = Gtk::manage(new Gtk::Button(str));
        button->set_halign(Gtk::Align::START);
        button->set_hexpand(false);
        button->add_css_class("button");
        if(props.contains("class")) button->add_css_class(props["class"]); 
        if(props.contains("style")){
            css_provider = Gtk::CssProvider::create();
            css_provider->load_from_data("button {" + props["style"] + "}");
            button->get_style_context()->add_provider(css_provider,
                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        box->append(*button);
    }
};

class StyleTag : public HTMLTag{
public:
    StyleTag(): HTMLTag(Stylee) {};
    std::string src;
};

class ScriptTag : public HTMLTag {
public:
    ScriptTag(): HTMLTag(Scriptt) {};
    std::string src;
};
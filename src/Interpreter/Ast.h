#pragma once
#include <glibmm/refptr.h>
#include <gtk/gtk.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/entry.h>
#include <gtkmm/enums.h>
#include <gtkmm/image.h>
#include <gtkmm/object.h>
#include <gtkmm/widget.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <glibmm/refptr.h>
#include <uuid/uuid.h>
#include <glibmm/dispatcher.h>

enum TagType {
    Html, Body, Head, h1, h2, h3, h4, h5, p, String, Image, Input, Button, Div, Stylee, Scriptt,
    Span, Header, Nav, Footer, Main, Article, Section, Aside
};


class HTMLTag : public std::enable_shared_from_this<HTMLTag> {
public:
    TagType type;
    std::vector<std::shared_ptr<HTMLTag>> children;
    std::unordered_map<std::string, std::string> props;
    Glib::RefPtr<Gtk::CssProvider> css_provider;

    std::string html_elm_name;
    std::string elm_name;
    Gtk::Widget* current_widget = nullptr;
    Gtk::Box* parent_widget = nullptr;

    HTMLTag(TagType t, std::string html_element_name, std::string element_name): type(t),
        html_elm_name(html_element_name), elm_name(element_name){};
    
    virtual void render(Gtk::Box* box);
    void flatten(std::vector<std::shared_ptr<HTMLTag>>& tags);
    virtual void setChildren(std::vector<std::shared_ptr<HTMLTag>> tags);
    virtual std::vector<std::string> getClassNames();

    virtual void applyCssClasses() {};
    virtual void applyStyle() {};
    virtual void setInnerHtml(std::vector<std::shared_ptr<HTMLTag>> tags) {
        children = tags;
    };
    virtual void unRender(){
        if(parent_widget && current_widget){
            parent_widget->remove(*current_widget);
            current_widget = nullptr;
        }
    }
    void unRenderRecurseCallable(){
        unRenderRecurse();
        children.clear();
    }
    virtual void unRenderRecurse(){
        for(auto& child : children){
            child->unRenderRecurse();
        }
        unRender();
    }

    virtual ~HTMLTag() {
        
    }
};

class Program {
public:
    std::vector<std::shared_ptr<HTMLTag>> html_tags;
    std::vector<std::string> style_srcs;
    std::vector<std::string> script_srcs;
};

class ContainerTag : public HTMLTag {
public:
    ContainerTag(TagType type, std::string html_element_name,
        std::string element_name): HTMLTag(type, html_element_name, element_name) {};
    Gtk::Box* container_box = nullptr;

    void applyCssClasses() override;
    void applyStyle() override;
    void render(Gtk::Box* parent_box) override;
    
    ~ContainerTag() {};
};

class BodyTag : public ContainerTag {
public:
    BodyTag(): ContainerTag(Body, "body", "box") {};

    ~BodyTag() {
        if(parent_widget && current_widget)
            parent_widget->remove(*current_widget);
    }
};

class DivTag : public ContainerTag {
public:
    DivTag(): ContainerTag(Div, "div", "box") {};
};

class SpanTag : public ContainerTag {
public:
    SpanTag(): ContainerTag(Span, "span", "box") {};
};

class HeaderTag : public ContainerTag {
public:
    HeaderTag(): ContainerTag(Header, "header", "box") {};
};

class NavTag : public ContainerTag {
public:
    NavTag(): ContainerTag(Nav, "span", "box") {};
};

class FooterTag : public ContainerTag {
public:
    FooterTag(): ContainerTag(Footer, "footer", "box") {};
};

class MainTag : public ContainerTag {
public:
    MainTag(): ContainerTag(Main, "main", "box") {};
};

class HeadTag : public ContainerTag {
public:
    HeadTag(): ContainerTag(Head, "head", "box") {};
};

class ArticleTag : public ContainerTag {
public:
    ArticleTag(): ContainerTag(Article, "article", "box") {};
};

class SectionTag : public ContainerTag {
public:
    SectionTag(): ContainerTag(Section, "section", "box") {};
};

class AsideTag : public ContainerTag {
public:
    AsideTag(): ContainerTag(Aside, "aside", "box") {};
};

class StringTag : public HTMLTag {
public:
    std::string str;
    std::string parent_class_name;
    StringTag(std::string stringg, std::string p_class_name = "none"):
        HTMLTag(String, "String", "label"), str(stringg), parent_class_name(p_class_name) {};
    Gtk::Label* lab;

    void applyCssClasses() override;
    void applyStyle() override;
    void render(Gtk::Box* targ_box) override;

    ~StringTag() {};
};

class TextTag : public HTMLTag {
public:
    TextTag(TagType type, std::string html_element_name,
        std::string element_name): HTMLTag(type, html_element_name, element_name) {};
    Gtk::Box* box;

    void render(Gtk::Box* parent_box) override;
};

class H1Tag : public TextTag {
public:
    H1Tag(): TextTag(h1, "h1", "label") {};
};

class H2Tag : public TextTag {
public:
    H2Tag(): TextTag(h2, "h2", "label") {};

};

class H3Tag : public TextTag {
public:
    H3Tag(): TextTag(h3, "h3", "label") {};
};

class H4Tag : public TextTag {
public:
    H4Tag(): TextTag(h4, "h4", "label") {};
};

class H5Tag : public TextTag {
public:
    H5Tag(): TextTag(h5, "h5", "label") {};
};


class PTag : public HTMLTag {
public:
    PTag(): HTMLTag(p, "p", "label") {};
};

class ImageTag : public HTMLTag {
public:
    ImageTag(): HTMLTag(Image, "img", "image") {};
    Gtk::Image* image;
    std::string img_path;
    Glib::Dispatcher disp;

    void applyCssClasses() override;
    void applyStyle() override;
    void render(Gtk::Box* box) override;

    ~ImageTag() {}
};

class InputTag : public HTMLTag {
public:
    InputTag(): HTMLTag(Input, "input", "input"){};
    Gtk::Entry* input;

    void applyCssClasses() override;
    void applyStyle() override;
    void render(Gtk::Box* box) override;

    ~InputTag() {}
};


class ButtonTag : public HTMLTag {
public:
    ButtonTag(): HTMLTag(Button, "button", "button") {};
    Gtk::Button* button;
    std::string str;
    
    void applyCssClasses() override;
    void applyStyle() override;
    void render(Gtk::Box* box) override;

    ~ButtonTag() {}
};

class StyleTag : public HTMLTag{
public:
    StyleTag(): HTMLTag(Stylee, "style", "") {};
    std::string src;
};

class ScriptTag : public HTMLTag {
public:
    ScriptTag(): HTMLTag(Scriptt, "script", "") {};
    std::string src;
};
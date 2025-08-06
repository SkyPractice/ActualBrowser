#pragma once
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <glibmm/refptr.h>
#include <gtk/gtk.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/entry.h>
#include <gtkmm/enums.h>
#include <gtkmm/image.h>
#include <gtkmm/object.h>
#include <gtkmm/video.h>
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
#include "TagRendering/TagRenderers.h"

class LambdaVal;

enum TagType {
    Html, Body, Head, h1, h2, h3, h4, h5, p, String, Image, Input, Button, Div, Stylee, Scriptt,
    Span, Header, Nav, Footer, Main, Article, Section, Aside, Video
};

struct TagInformation {
    TagType type;
    std::string uuid = "";
    std::string html_elm_name = "";
    std::string elm_name = "";
    Gtk::Widget* current_widget = nullptr;
    Gtk::Box* parent_widget = nullptr;
};

class HTMLTag;

typedef std::unordered_map<std::string, std::string> Properties;
typedef std::vector<std::shared_ptr<HTMLTag>> Children;
typedef std::unique_ptr<HTMLTagRenderer> HTMLTagRendererPtr;

class HTMLTag : public std::enable_shared_from_this<HTMLTag> {
public:
    
    Children children;
    Properties props;
    TagInformation tag_information;

    std::unique_ptr<HTMLTagRenderer> renderer;

    Glib::RefPtr<Gtk::CssProvider> css_provider;

    HTMLTag(TagType t, std::string html_element_name, std::string element_name):
        tag_information({t, "", html_element_name,
             element_name, nullptr, nullptr}),
             renderer(std::make_unique<HTMLTagRenderer>()){
        boost::uuids::random_generator guid_gen;
        boost::uuids::uuid file_uuid = guid_gen();
        tag_information.uuid = boost::uuids::to_string(file_uuid); 
    };
    
    HTMLTag(TagType t, std::string html_element_name, std::string element_name,
        HTMLTagRendererPtr renderer_ptr):
        tag_information({t, "", html_element_name,
            element_name, nullptr, nullptr}),
            renderer(std::move(renderer_ptr)){
        boost::uuids::random_generator guid_gen;
        boost::uuids::uuid file_uuid = guid_gen();
        tag_information.uuid = boost::uuids::to_string(file_uuid); 
    };

    void render(Gtk::Box* box);

    void flatten(std::vector<std::shared_ptr<HTMLTag>>& tags);
    void setChildren(std::vector<std::shared_ptr<HTMLTag>> tags);
    std::vector<std::string> getClassNames();

    void setInnerHtml(std::vector<std::shared_ptr<HTMLTag>> tags) {
        children = tags;
    };

    void unRender();
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

    //virtual std::shared_ptr<HTMLTag> cloneSelf(); // without rendering
    //virtual void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);

    virtual ~HTMLTag() {
        
    }

    virtual std::string unTokenizeHirarichy();
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
        std::string element_name): HTMLTag(type, html_element_name, element_name, 
        std::make_unique<ContainerTagRenderer>()) {};
    Gtk::Box* container_box = nullptr;

    std::shared_ptr<HTMLTag> cloneSelf();
    void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);
    
    ~ContainerTag() {};
};

class BodyTag : public ContainerTag {
public:
    BodyTag(): ContainerTag(Body, "body", "box") {};

    ~BodyTag() {
        if(tag_information.parent_widget && tag_information.current_widget)
            tag_information.parent_widget->remove(*tag_information.current_widget);
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
        HTMLTag(String, "String", "label",
            std::make_unique<StringTagRenderer>()),
            str(stringg), parent_class_name(p_class_name) {};
    Gtk::Label* lab;

    std::string unTokenizeHirarichy() override;

    std::shared_ptr<HTMLTag> cloneSelf();
    void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);

    ~StringTag() {};
};

class TextTag : public HTMLTag {
public:
    TextTag(TagType type, std::string html_element_name,
        std::string element_name): HTMLTag(type, html_element_name, element_name,
            std::make_unique<TextTagRenderer>()) {};
    Gtk::Box* box;

    std::shared_ptr<HTMLTag> cloneSelf();
    void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);
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
    ImageTag(): HTMLTag(Image, "img", "image",
        std::make_unique<ImageTagRenderer>()) {};
    Gtk::Image* image;
    std::string img_path;
    Glib::Dispatcher disp;

    ~ImageTag() {
        if(std::filesystem::exists(img_path)){
            std::filesystem::remove(img_path);
        }
    }

    std::shared_ptr<HTMLTag> cloneSelf();
    void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);
};

class InputTag : public HTMLTag {
public:
    InputTag(): HTMLTag(Input, "input", "input",
        std::make_unique<InputTagRenderer>()){};
    Gtk::Entry* input;

    std::shared_ptr<HTMLTag> cloneSelf();
    void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);

    ~InputTag() {}
};

class ButtonTag : public HTMLTag {
public:
    ButtonTag(): HTMLTag(Button, "button", "button", 
        std::make_unique<ButtonTagRenderer>()) {};
    Gtk::Button* button;
    std::string str;
    
    std::shared_ptr<HTMLTag> cloneSelf();
    void cloneHirarichy(std::vector<std::shared_ptr<HTMLTag>>& result_tags);

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

class VideoTag : public HTMLTag {
public:
    std::string src;
    Gtk::Video* vid;
    VideoTag(): HTMLTag(Video, "video", "video", 
        std::make_unique<VideoTagRenderer>()) {};
};
#pragma once
#include <gtkmm/box.h>
#include <memory>
#include "../StyleManagement/CssManagers.h"

//
//  A Tag Renderer Renders And Applies Hirarichy RelationShips
//  During The Render Function
//

class HTMLTag;
typedef std::unique_ptr<HTMLTagCssManager> HTMLTagCssManagerPtr;

class HTMLTagRenderer {
public:
    HTMLTagCssManagerPtr css_manager;

    HTMLTagRenderer(): css_manager(std::make_unique<HTMLTagCssManager>()) {};
    
    HTMLTagRenderer(HTMLTagCssManagerPtr target_css_manager):
        css_manager(std::move(target_css_manager)) {};

    virtual void render(HTMLTag* target_tag, Gtk::Box* target_box);
    virtual void unRender(HTMLTag* target_tag);
    void renderChildren(HTMLTag* target_children_tag, Gtk::Box* box);
};

class ContainerTagRenderer : public HTMLTagRenderer {
public:
    ContainerTagRenderer(): HTMLTagRenderer(
        std::make_unique<ContainerTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box) override;
};

class TextTagRenderer : public HTMLTagRenderer {
public:
    TextTagRenderer(): HTMLTagRenderer(
        std::make_unique<TextTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box) override;
};

class StringTagRenderer : public HTMLTagRenderer {
public:
    StringTagRenderer(): HTMLTagRenderer(
        std::make_unique<StringTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box) override;
};


class ImageTagRenderer : public HTMLTagRenderer {
public:
    ImageTagRenderer(): HTMLTagRenderer(
        std::make_unique<ImageTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box) override;
};

class ButtonTagRenderer : public HTMLTagRenderer {
public:
    ButtonTagRenderer(): HTMLTagRenderer(
        std::make_unique<ButtonTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box) override;
};

class InputTagRenderer : public HTMLTagRenderer {
public:
    InputTagRenderer(): HTMLTagRenderer(
        std::make_unique<InputTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box) override;
};

class VideoTagRenderer : public HTMLTagRenderer {
public:
    VideoTagRenderer(): HTMLTagRenderer(
        std::make_unique<VideoTagCssManager>()) {};

    void render(HTMLTag* target_tag, Gtk::Box* target_box);
};
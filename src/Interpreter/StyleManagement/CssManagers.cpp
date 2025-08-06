#include "CssManagers.h"
#include "../Ast.h"
#include <format>

void BasicTagCssManagerUtil::applyCssClassesUtil(HTMLTag* target_tag, Gtk::Widget* target_widget) {
    auto css_classes = target_tag->props.find("class");
    if(css_classes != target_tag->props.end()){
        for(auto& css_class : getCssClasses(css_classes->second))
            target_widget->add_css_class(css_class);
    }
    target_widget->add_css_class(target_tag->tag_information.html_elm_name);
};

void BasicTagCssManagerUtil::applyStyleUtil(HTMLTag* target_tag, Gtk::Widget* target_widget) {
    auto style_itr = target_tag->props.find("style");

    if(style_itr != target_tag->props.end()){
        target_tag->css_provider = Gtk::CssProvider::create();
        target_tag->css_provider->load_from_data(std::format("{} {{{}}}",
            target_tag->tag_information.elm_name, style_itr->second));
        target_widget->get_style_context()->add_provider(target_tag->css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    normalizePositioning(target_widget);
};

void HTMLTagCssManager::normalizePositioning(Gtk::Widget* target_widget) {
    target_widget->set_valign(Gtk::Align::START);
    target_widget->set_halign(Gtk::Align::START);
    target_widget->set_vexpand(false);
    target_widget->set_hexpand(false);  
};

CssClassList HTMLTagCssManager::getCssClasses(std::string target_string) {
    std::istringstream stream(target_string);
    CssClassList class_list;
    std::string current_class_name;

    while((stream >> current_class_name))
        class_list.push_back(current_class_name);
        
    return class_list;
};

void ContainerTagCssManager::applyStyle(HTMLTag* target_tag){
    ContainerTag* casted_tag = static_cast<ContainerTag*>(target_tag);
    applyStyleUtil(target_tag, casted_tag->container_box);
};

void ContainerTagCssManager::applyCssClasses(HTMLTag* target_tag){
    ContainerTag* casted_tag = static_cast<ContainerTag*>(target_tag);
    applyCssClassesUtil(target_tag, casted_tag->container_box);
};

void TextTagCssManager::applyCssClasses(HTMLTag* target_tag) {
    TextTag* casted_tag = static_cast<TextTag*>(target_tag);
    
    for(auto& child : casted_tag->children){
        auto class_names_itr = casted_tag->props.find("class");
        if(child->tag_information.type == String){
            StringTag* casted_str_tag = static_cast<StringTag*>(child.get());
            if(class_names_itr != casted_tag->props.end()){
                child->props["class"] = class_names_itr->second;
            }
            casted_str_tag->parent_class_name = casted_tag->tag_information.html_elm_name;
        } else {
            child->props["class"] = child->props["class"] + " " + casted_tag->tag_information.html_elm_name;
        }
    }
};

void TextTagCssManager::applyStyle(HTMLTag* target_tag){
    TextTag* casted_tag = static_cast<TextTag*>(target_tag);

    auto style_itr = casted_tag->props.find("style");
    auto width_itr = casted_tag->props.find("width");

    for(auto& child : casted_tag->children){
        for(auto& child : casted_tag->children){
            if(style_itr != casted_tag->props.end()) { child->props["style"] = style_itr->second; };
            if(width_itr != casted_tag->props.end()) { child->props["width"] = width_itr->second; };
        }
    }

    normalizePositioning(casted_tag->box);
};

void StringTagCssManager::applyCssClasses(HTMLTag* target_tag){
    StringTag* casted_tag = static_cast<StringTag*>(target_tag);
    auto itr = casted_tag->props.find("class");

    if(itr != casted_tag->props.end()){
        for(auto& css_class : getCssClasses(itr->second)){
            casted_tag->lab->add_css_class(css_class);
        }
    }

    casted_tag->lab->add_css_class(casted_tag->parent_class_name);
};

void StringTagCssManager::applyStyle(HTMLTag* target_tag){
    StringTag* casted_tag = static_cast<StringTag*>(target_tag);
    auto style_itr = casted_tag->props.find("style");

    if(style_itr != casted_tag->props.end()){
        casted_tag->css_provider = Gtk::CssProvider::create();
        casted_tag->css_provider->load_from_data(std::format("{} {{{}}}",
            casted_tag->tag_information.elm_name, style_itr->second));
        casted_tag->lab->get_style_context()->add_provider(casted_tag->css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    normalizePositioning(casted_tag->lab);
};

void ButtonTagCssManager::applyStyle(HTMLTag* target_tag){
    ButtonTag* casted_tag = static_cast<ButtonTag*>(target_tag);
    applyStyleUtil(target_tag, casted_tag->button);
};

void ButtonTagCssManager::applyCssClasses(HTMLTag* target_tag){
    ButtonTag* casted_tag = static_cast<ButtonTag*>(target_tag);
    applyCssClassesUtil(target_tag, casted_tag->button);
};

void InputTagCssManager::applyStyle(HTMLTag* target_tag){
    InputTag* casted_tag = static_cast<InputTag*>(target_tag);
    applyStyleUtil(target_tag, casted_tag->input);
};

void InputTagCssManager::applyCssClasses(HTMLTag* target_tag){
    InputTag* casted_tag = static_cast<InputTag*>(target_tag);
    applyCssClassesUtil(target_tag, casted_tag->input);
};

void ImageTagCssManager::applyStyle(HTMLTag* target_tag){
    ImageTag* casted_tag = static_cast<ImageTag*>(target_tag);
    applyStyleUtil(target_tag, casted_tag->image);
};

void ImageTagCssManager::applyCssClasses(HTMLTag* target_tag){
    ImageTag* casted_tag = static_cast<ImageTag*>(target_tag);
    applyCssClassesUtil(target_tag, casted_tag->image);
};

void VideoTagCssManager::applyStyle(HTMLTag* target_tag){
    VideoTag* casted_tag = static_cast<VideoTag*>(target_tag);
    applyStyleUtil(target_tag, casted_tag->vid);
};

void VideoTagCssManager::applyCssClasses(HTMLTag* target_tag){
    VideoTag* casted_tag = static_cast<VideoTag*>(target_tag);
    applyCssClassesUtil(target_tag, casted_tag->vid);
};
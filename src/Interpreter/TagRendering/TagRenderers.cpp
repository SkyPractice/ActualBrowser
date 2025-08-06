#include "TagRenderers.h"
#include "../Ast.h"
#include "../../Concurrency/ThreadPool.h"
#include "../../HttpManager/HttpManager.h"
#include <boost/asio/post.hpp>
#include <iostream>

void HTMLTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    target_tag->tag_information.parent_widget = target_box;
    renderChildren(target_tag, target_box);
};

void HTMLTagRenderer::unRender(HTMLTag* target_tag){
    if(target_tag->tag_information.parent_widget && target_tag->tag_information.current_widget){
        for(auto& child : target_tag->children){
            child->unRender();
        }
        target_tag->tag_information.parent_widget->remove(*target_tag->tag_information.current_widget);
        target_tag->tag_information.current_widget = nullptr;
    }
};

void HTMLTagRenderer::renderChildren(HTMLTag* target_children_tag, Gtk::Box* box) {
    for(auto& child : target_children_tag->children){
        child->render(box);
    }
};

void ContainerTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    ContainerTag* casted_tag = static_cast<ContainerTag*>(target_tag);
    
    casted_tag->container_box = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->tag_information.current_widget = casted_tag->container_box;
    
    target_box->append(*casted_tag->container_box);

    css_manager->applyCssClasses(casted_tag);
    css_manager->applyStyle(casted_tag);

    renderChildren(casted_tag, casted_tag->container_box);
};

void TextTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    TextTag* casted_tag = static_cast<TextTag*>(target_tag);
    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->box = Gtk::manage(new Gtk::Box(Gtk::Orientation::HORIZONTAL));
    casted_tag->tag_information.current_widget = casted_tag->box;

    target_box->append(*casted_tag->box);

    css_manager->applyCssClasses(casted_tag);
    css_manager->applyStyle(casted_tag);

    renderChildren(casted_tag, casted_tag->box);
};

void StringTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    StringTag* casted_tag = static_cast<StringTag*>(target_tag);
    casted_tag->lab = Gtk::manage(new Gtk::Label(casted_tag->str));
    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->tag_information.current_widget = casted_tag->lab;

    target_box->append(*casted_tag->lab);

    css_manager->applyCssClasses(casted_tag);
    css_manager->applyStyle(casted_tag);
};

void ButtonTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    ButtonTag* casted_tag = static_cast<ButtonTag*>(target_tag);
    casted_tag->button = Gtk::manage(new Gtk::Button(casted_tag->str));

    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->tag_information.current_widget = casted_tag->button;

    target_box->append(*casted_tag->button);
    
    css_manager->applyCssClasses(casted_tag);
    css_manager->applyStyle(casted_tag);
};
void VideoTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    VideoTag* casted_tag = static_cast<VideoTag*>(target_tag);
    casted_tag->vid = Gtk::manage(new Gtk::Video(casted_tag->props["src"]));
    std::cout << casted_tag->props["src"];

    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->tag_information.current_widget = casted_tag->vid;

    target_box->append(*casted_tag->vid);
    
    css_manager->applyCssClasses(casted_tag);
    css_manager->applyStyle(casted_tag);
};
void InputTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    InputTag* casted_tag = static_cast<InputTag*>(target_tag);
    casted_tag->input = Gtk::manage(new Gtk::Entry);

    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->tag_information.current_widget = casted_tag->input;

    target_box->append(*casted_tag->input);

    css_manager->applyCssClasses(casted_tag);
    css_manager->applyStyle(casted_tag);
};
void ImageTagRenderer::render(HTMLTag* target_tag, Gtk::Box* target_box) {
    ImageTag* casted_tag = static_cast<ImageTag*>(target_tag);
    casted_tag->tag_information.parent_widget = target_box;
    casted_tag->image = Gtk::manage(new Gtk::Image);
    casted_tag->tag_information.parent_widget->append(*casted_tag->image);
    casted_tag->disp.connect([casted_tag](){
        casted_tag->image->set(casted_tag->img_path);
    });
    boost::asio::post(Concurrency::pool, [casted_tag, this](){
        if(casted_tag->props.contains("src")){
            const std::string pathh = HttpExposer::current_http_manager->getImage(
                casted_tag->props["src"]);
        
            casted_tag->tag_information.current_widget = casted_tag->image;
            css_manager->applyCssClasses(casted_tag);
            css_manager->applyStyle(casted_tag);
            casted_tag->img_path = pathh;
            casted_tag->disp.emit();
        }
    });
};
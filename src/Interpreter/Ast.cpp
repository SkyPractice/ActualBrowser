#include "Ast.h"

void HTMLTag::render(Gtk::Box* box){
    renderer->render(this, box);
}
void HTMLTag::unRender(){
    renderer->unRender(this);
}

void HTMLTag::flatten(std::vector<std::shared_ptr<HTMLTag>>& tags){
    tags.push_back(shared_from_this());
     for(auto& child : children){
        child->flatten(tags);
    }
};

void HTMLTag::setChildren(std::vector<std::shared_ptr<HTMLTag>> tags){
    children = tags;
};

std::vector<std::string> HTMLTag::getClassNames(){
    if(props.contains("class")){
        std::istringstream stream(props["class"]);
        std::vector<std::string> class_names;
        std::string current;

         while((stream >> current))
            class_names.push_back(current);
        
        return class_names;
    } else return {};
};


// void ContainerTag::applyCssClasses() {
//     for(const auto& class_name : getClassNames()){
//         container_box->add_css_class(class_name);
//     }
//     container_box->add_css_class(tag_information.html_elm_name);
// };
// void ContainerTag::applyStyle() {
//     if(props.contains("style")){
//         css_provider = Gtk::CssProvider::create();
//         css_provider->load_from_data(std::format("{} {{{}}}", tag_information.elm_name, props["style"]));
//         container_box->get_style_context()->add_provider(css_provider,
//              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//     }
//     container_box->set_valign(Gtk::Align::START);
//     container_box->set_halign(Gtk::Align::START);
//     container_box->set_vexpand(false);
//     container_box->set_hexpand(false);
// };
// void ContainerTag::render(Gtk::Box* parent_box) {
//     container_box = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
//     tag_information.parent_widget = parent_box;
//     tag_information.current_widget = container_box;
//     parent_box->append(*container_box);
//     applyCssClasses();
//     applyStyle();
//     container_box->add_css_class(tag_information.html_elm_name);
//     for(auto& child : children){
//         child->render(container_box);
//     }
// };


// void StringTag::applyCssClasses() {
//     for(auto& class_name : getClassNames()){
//         lab->add_css_class(class_name);
//     }
// };
// void StringTag::applyStyle() {
//     if(props.contains("style")){
//         css_provider = Gtk::CssProvider::create();
//         css_provider->load_from_data(std::format("{} {{{}}}", tag_information.elm_name, props["style"]));
//         lab->get_style_context()->add_provider(css_provider,
//             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//     }
// };
// void StringTag::render(Gtk::Box* targ_box) {
//     lab = Gtk::manage(new Gtk::Label(str));
//     tag_information.parent_widget = targ_box;
//     tag_information.current_widget = lab;
//     lab->add_css_class(parent_class_name);
//     lab->set_hexpand(false);
//     lab->set_vexpand(false);
//     lab->set_valign(Gtk::Align::START);
//     lab->set_halign(Gtk::Align::START);
//     applyCssClasses();
//     applyStyle();
//     if(props.contains("width")){
//         lab->set_max_width_chars(std::stoi(props["width"]));
//     }
//     lab->set_wrap();
//     lab->set_natural_wrap_mode(Gtk::NaturalWrapMode::WORD);
//     targ_box->append(*lab);
// };

// void TextTag::render(Gtk::Box* parent_box){
//     tag_information.parent_widget = parent_box;
//     box = Gtk::manage(new Gtk::Box(Gtk::Orientation::HORIZONTAL));
//     tag_information.current_widget = box;
//     box->set_hexpand(false);
//     box->set_vexpand(false);
//     box->set_valign(Gtk::Align::START);
//     box->set_halign(Gtk::Align::START);
//     parent_box->append(*box);
//     for(auto& child : children){
//         if(child->tag_information.type == String){
//             auto real_child = std::dynamic_pointer_cast<StringTag>(child);
//             if(props.contains("class")) real_child->props["class"] = props["class"]; 
//             if(props.contains("style")) real_child->props["style"] = props["style"];
//             if(props.contains("width")) real_child->props["width"] = props["width"];
//             real_child->parent_class_name = tag_information.html_elm_name;
//             real_child->render(box);
//         } else {

//             child->props["class"] = child->props["class"] + " " + tag_information.html_elm_name;
//             child->render(box);
//         }
//     }
// };

// void ImageTag::applyCssClasses() {
//     for(auto& class_name : getClassNames()){
//         image->add_css_class(class_name);
//     }
//     image->add_css_class(tag_information.html_elm_name);
// };
// void ImageTag::applyStyle() {
//     if(props.contains("style")){
//         css_provider = Gtk::CssProvider::create();
//         css_provider->load_from_data(std::format("{} {{{}}}", tag_information.elm_name, props["style"]));
//         image->get_style_context()->add_provider(css_provider,
//             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//     }
// };
// void ImageTag::render(Gtk::Box* box) {
//     tag_information.parent_widget = box;
//     image = Gtk::manage(new Gtk::Image);
//     tag_information.parent_widget->append(*image);
//     disp.connect([this](){
//         image->set(img_path);
//     });
//     boost::asio::post(Concurrency::pool, [this](){
//         if(props.contains("src")){
//             const std::string pathh = HttpExposer::current_http_manager->getImage(
//                 props["src"]);
        
//             std::cout << pathh << std::endl;
//             tag_information.current_widget = image;
//             image->set_halign(Gtk::Align::START);
//             image->set_hexpand(false);
//             applyCssClasses();
//             applyStyle();
//             img_path = pathh;
//             disp.emit();
        
//     }});
// };

// void InputTag::applyCssClasses() {
//     for(auto& css_class : getClassNames()){
//         input->add_css_class(css_class);
//     }
//     input->add_css_class("input");
// };
// void InputTag::applyStyle() {
//     if(props.contains("style")){
//         css_provider = Gtk::CssProvider::create();
//         css_provider->load_from_data(std::format("{} {{{}}}", tag_information.elm_name, props["style"]));
//         input->get_style_context()->add_provider(css_provider,
//             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//     }
// };
// void InputTag::render(Gtk::Box* box) {
//     input = Gtk::manage(new Gtk::Entry);
//     tag_information.parent_widget = box;
//     tag_information.current_widget = input;
//     applyCssClasses();
//     input->set_halign(Gtk::Align::START);
//     input->set_hexpand(false);
//     applyStyle();
//     box->append(*input);
// };

// void ButtonTag::applyCssClasses() {
//     for(auto& css_class : getClassNames()){
//         button->add_css_class(css_class);
//     }
//     button->add_css_class("button");
// };
// void ButtonTag::applyStyle() {
//     if(props.contains("style")){
//         css_provider = Gtk::CssProvider::create();
//         css_provider->load_from_data(std::format("{} {{{}}}", tag_information.elm_name, props["style"]));
//         button->get_style_context()->add_provider(css_provider,
//             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//     }
// };
// void ButtonTag::render(Gtk::Box* box) {
//     button = Gtk::manage(new Gtk::Button(str));
//     tag_information.parent_widget = box;
//     tag_information.current_widget = button;
//     button->set_halign(Gtk::Align::START);
//     button->set_hexpand(false);
//     applyCssClasses(); 
//     applyStyle();
//     box->append(*button);
// };

std::string HTMLTag::unTokenizeHirarichy() {
    std::ostringstream result_stream;
    result_stream << "<" << tag_information.html_elm_name;
    for(auto& [prop_name, prop_val] : props){
        result_stream << " " << prop_name << "=" << prop_val;
    }
    result_stream << ">";

    for(auto& child : children){
        result_stream << child->unTokenizeHirarichy();
    }

    result_stream << "</" << tag_information.html_elm_name << ">";
    return result_stream.str();
};

std::string StringTag::unTokenizeHirarichy() {
    return str;
};

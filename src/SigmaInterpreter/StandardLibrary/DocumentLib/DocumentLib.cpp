#include "DocumentLib.h"
#include "../../SigmaInterpreter.h"
#include "../../../Interpreter/Interpreter.h"
#include <algorithm>
#include <gtkmm/box.h>
#include <memory>
#include "../TypeWrappers/StringWrapper.h"
#include "../TypeWrappers/ArrayWrapper.h"

std::unordered_set<TagType> DocumentLib::txt_tags = {
    h1, h2, h3, h4, h5, p
};

std::unordered_map<std::string, std::function<std::unique_ptr<Event>(LambdaVal*)>> 
    DocumentLib::event_table = {
    {"click", [](LambdaVal* handler){ return std::make_unique<ClickEvent>(handler); }},
    {"key_down", [](LambdaVal* handler){ return std::make_unique<KeyBoardEvent>(handler); }},
    {"mouse_in", [](LambdaVal* handler){ return std::make_unique<MouseInEvent>(handler); }},
    {"mouse_out", [](LambdaVal* handler){ return std::make_unique<MouseOutEvent>(handler); }},
    {"mouse_move", [](LambdaVal* handler){ return std::make_unique<MouseMotionEvent>(handler); }}

};

RunTimeVal* DocumentLib::getStruct() {
    LibMapType lib_map = {
        {"getElementById", RunTimeFactory::makeNativeFunction(&DocumentLib::getElementById, {{"element_id", StringType}})},
        {"getElementsByClassName", RunTimeFactory::makeNativeFunction(&DocumentLib::getElementsByClassName, {{"element_class_name", StringType}})},
        {"setInnerHtml", RunTimeFactory::makeNativeFunction(&DocumentLib::setElementInnerHtml, {{"element", HtmlType}, {"new_inner_html", StringType}})},
        {"getInnerHtml", RunTimeFactory::makeNativeFunction(&DocumentLib::getElementInnerHtml, {{"element", HtmlType}})},
        {"getText", RunTimeFactory::makeNativeFunction(&DocumentLib::getText, {{"element", HtmlType}})},
        {"setText", RunTimeFactory::makeNativeFunction(&DocumentLib::setText, {{"element", HtmlType}, {"new_text", StringType}})},
        {"getOuterHtml", RunTimeFactory::makeNativeFunction(&DocumentLib::getOuterHtml, {{"element", HtmlType}})},
        {"getHtmlContents", RunTimeFactory::makeNativeFunction(&DocumentLib::getContents, {{"element", HtmlType}})},
       {"appendChild", RunTimeFactory::makeNativeFunction(&DocumentLib::appendChild, {{"element", HtmlType},{"child", HtmlType}})},
       {"appendChildren", RunTimeFactory::makeNativeFunction(&DocumentLib::appendChildren, {{"element", HtmlType},{"children", ArrayType}})},
       {"popChild", RunTimeFactory::makeNativeFunction(&DocumentLib::popChild, {{"element", HtmlType}})},
       {"removeElement", RunTimeFactory::makeNativeFunction(&DocumentLib::removeElement, {{"parent", HtmlType}, {"element", HtmlType}})},
       {"createElementsFromHtml", RunTimeFactory::makeNativeFunction(&DocumentLib::createElementsFromHtml, {{"html_text", StringType}})},
       {"clearChildren", RunTimeFactory::makeNativeFunction(&DocumentLib::clearChildren, {{"element", HtmlType}})},
       {"connectEventHandler", RunTimeFactory::makeNativeFunction(&DocumentLib::connectEventHandler, {{"element", HtmlType}, {"event_type", StringType}, {"handler", LambdaType}})}
    };

    return RunTimeFactory::makeStruct(std::move(lib_map));
};

RunTimeValue DocumentLib::getElementById(COMPILED_FUNC_ARGS) {
    std::string id = dynamic_cast<StringVal*>(args[0])->str;
    return RunTimeFactory::makeHtmlElement(interpreter->accessor->id_ptrs.at(id).get());
};
RunTimeValue DocumentLib::setElementInnerHtml(COMPILED_FUNC_ARGS){
    auto html_elm = dynamic_cast<HtmlElementVal*>(args[0]);
    Lexer lex;
    Parser pars;
    Interpreter interpret;
    std::string html_str = dynamic_cast<StringVal*>(args[1])->str;
    
    auto tokens = lex.tokenize(html_str);
    auto ast_val = pars.produceAst(tokens);
    for(auto& child : html_elm->target_tag->children)
        child->unRender();
    html_elm->target_tag->setChildren(ast_val.html_tags);
    interpret.renderTags(dynamic_cast<Gtk::Box*>(html_elm->target_tag->tag_information.current_widget), ast_val, "");
    interpreter->accessor->current_interp->refreshIdsAndClasses();
    return nullptr;
};

RunTimeValue DocumentLib::getElementsByClassName(COMPILED_FUNC_ARGS) {
    std::string cl = dynamic_cast<StringVal*>(args[0])->str;
    auto elms = interpreter->accessor->class_name_ptrs.equal_range(cl);
    auto [beg_itr, end_itr] = elms;

    std::vector<RunTimeValue> results;
    for(auto itr = beg_itr; itr != end_itr; itr++){
        results.push_back(RunTimeFactory::makeHtmlElement(itr->second.get()));
    }

    return ArrayWrapper::genObject(RunTimeFactory::makeArray(results));
};

RunTimeVal* DocumentLib::setOnClick(COMPILED_FUNC_ARGS) {
    HtmlElementVal* elm_val = static_cast<HtmlElementVal*>(args[0]);
    LambdaVal* lambda_val = static_cast<LambdaVal*>(args[1]);
    interpreter->garbageCollectionRestricter.registerEventHandler(lambda_val);

    Gtk::Button* btn = static_cast<Gtk::Button*>(elm_val->target_tag->tag_information.current_widget);
    btn->signal_clicked().connect([lambda_val, interpreter](){
        interpreter->evaluateAnonymousLambdaCall(lambda_val, {});
    });
    return nullptr;
};

RunTimeVal* DocumentLib::getText(COMPILED_FUNC_ARGS) {
    HtmlElementVal* element = static_cast<HtmlElementVal*>(args[0]);
    HTMLTag* tag = element->target_tag;

    if(txt_tags.contains(tag->tag_information.type)){
        StringTag* str_tag = static_cast<StringTag*>(tag->children[0].get());
        return  StringWrapper::genObject(RunTimeFactory::makeString(
            str_tag->str
        ));
    } else if (tag->tag_information.type == Button){
        return  StringWrapper::genObject(RunTimeFactory::makeString(
            static_cast<ButtonTag*>(tag)->str
        ));
    } else if(tag->tag_information.type == Input){
        return  StringWrapper::genObject(RunTimeFactory::makeString(
            static_cast<InputTag*>(tag)->input->get_text()
        ));
    }

    return nullptr;
    
}; // for inputs or text areas 
RunTimeVal* DocumentLib::setText(COMPILED_FUNC_ARGS) {
    HtmlElementVal* element = static_cast<HtmlElementVal*>(args[0]);
    std::string& target_str = static_cast<StringVal*>(args[1])->str;
    HTMLTag* tag = element->target_tag;

    if(txt_tags.contains(tag->tag_information.type)){
        StringTag* str_tag = static_cast<StringTag*>(tag->children[0].get());
        str_tag->str = target_str;
        str_tag->lab->set_text(target_str);
    } else if (tag->tag_information.type == Button){
        ButtonTag* btn_tag = static_cast<ButtonTag*>(tag);
        btn_tag->str = target_str;
        btn_tag->button->set_label(target_str);
    } else if(tag->tag_information.type == Input){
        InputTag* input_tag = static_cast<InputTag*>(tag);
        input_tag->input->set_text(target_str);
    }

    return nullptr;
}; // for inputs or text areas

RunTimeVal* DocumentLib::getElementInnerHtml(COMPILED_FUNC_ARGS) {
    std::string result_str;
    HtmlElementVal* elm = static_cast<HtmlElementVal*>(args[0]);

    for(auto& child : elm->target_tag->children){
        result_str += child->unTokenizeHirarichy();
    }

    return  StringWrapper::genObject(RunTimeFactory::makeString(result_str));
};


RunTimeVal* DocumentLib::getOuterHtml(COMPILED_FUNC_ARGS) {
    HtmlElementVal* elm = static_cast<HtmlElementVal*>(args[0]);
    return  StringWrapper::genObject(RunTimeFactory::makeString(elm->target_tag->unTokenizeHirarichy()));
};
RunTimeVal* DocumentLib::getContents(COMPILED_FUNC_ARGS) {
    std::vector<RunTimeVal*> result_elements;
    HtmlElementVal* elm = static_cast<HtmlElementVal*>(args[0]);

    for(auto& child : elm->target_tag->children){
        result_elements.push_back(RunTimeFactory::makeHtmlElement(child.get()));
    }

    return ArrayWrapper::genObject(RunTimeFactory::makeArray(result_elements));
};
RunTimeVal* DocumentLib::cloneHtmlElements(COMPILED_FUNC_ARGS) {
    return nullptr;
};

RunTimeVal* DocumentLib::appendChild(COMPILED_FUNC_ARGS) {
    Interpreter interp;
    HtmlElementVal* target_elm = static_cast<HtmlElementVal*>(args[0]);
    HtmlElementVal* appended_elm = static_cast<HtmlElementVal*>(args[1]);

    target_elm->target_tag->children.push_back(std::shared_ptr<HTMLTag>(appended_elm->target_tag));
    appended_elm->target_tag->render(dynamic_cast<Gtk::Box*>(target_elm->target_tag->tag_information.current_widget));
    
    interpreter->accessor->current_interp->refreshIdsAndClasses();
    
    return nullptr;
};
RunTimeVal* DocumentLib::popChild(COMPILED_FUNC_ARGS) {
    HtmlElementVal* target_elm = static_cast<HtmlElementVal*>(args[0]);
    target_elm->target_tag->children.back()->unRender();
    target_elm->target_tag->children.pop_back();

    interpreter->accessor->current_interp->refreshIdsAndClasses();

    return nullptr;
};
RunTimeVal* DocumentLib::unRenderElement(COMPILED_FUNC_ARGS) {
    HtmlElementVal* target_elm = static_cast<HtmlElementVal*>(args[0]);
    target_elm->target_tag->unRender();

    return nullptr;
};
RunTimeVal* DocumentLib::createElementsFromHtml(COMPILED_FUNC_ARGS) {
    Lexer lex;
    Parser pars;
    std::string html_str = dynamic_cast<StringVal*>(args[0])->str;
    
    auto tokens = lex.tokenize(html_str);
    auto ast_val = pars.produceAst(tokens);

    std::vector<RunTimeVal*> vals;

    for(auto& elm : ast_val.html_tags){
        vals.push_back(RunTimeFactory::makeHtmlElement(elm.get()));
    }

    return ArrayWrapper::genObject(RunTimeFactory::makeArray(vals));
};

RunTimeVal* DocumentLib::connectEventHandler(COMPILED_FUNC_ARGS) {
    HtmlElementVal* elm = static_cast<HtmlElementVal*>(args[0]);
    StringVal* event_name = static_cast<StringVal*>(args[1]);
    LambdaVal* handler = static_cast<LambdaVal*>(args[2]);

    interpreter->garbageCollectionRestricter.registerEventHandler(handler);

    EventPtr evt = event_table.at(event_name->str)(handler);
    evt->bindEventToWidget(elm->target_tag->tag_information.current_widget,
        interpreter);
    
    return nullptr;
};
RunTimeVal* DocumentLib::appendChildren(COMPILED_FUNC_ARGS) {
    HtmlElementVal* target_parent = static_cast<HtmlElementVal*>(args[0]);
    ArrayVal* target_elms = static_cast<ArrayVal*>(args[1]);

    std::vector<std::shared_ptr<HTMLTag>> actual_elms;
    for(auto& elm : target_elms->vals){
        actual_elms.push_back(std::shared_ptr<HTMLTag>(dynamic_cast<HtmlElementVal*>(elm)->target_tag));
    }


    target_parent->target_tag->children.insert(target_parent->target_tag->children.end(),
        actual_elms.begin(), actual_elms.end());

    for(auto& elm : actual_elms){
        elm->render(dynamic_cast<Gtk::Box*>(target_parent->target_tag->tag_information.current_widget));
    }

    return nullptr;
};
RunTimeVal* DocumentLib::removeElement(COMPILED_FUNC_ARGS) {
    HtmlElementVal* target_parent = static_cast<HtmlElementVal*>(args[0]);
    HtmlElementVal* target_elm = static_cast<HtmlElementVal*>(args[1]);

    target_parent->target_tag->children.erase(std::remove_if(target_parent->target_tag->children.begin(),
       target_parent->target_tag->children.end(),
        [&](std::shared_ptr<HTMLTag> current_tag){ return current_tag->tag_information.uuid == target_elm->target_tag->tag_information.uuid; }),
            target_parent->target_tag->children.end());

    target_elm->target_tag->unRender();
    return nullptr;
};
RunTimeVal* DocumentLib::clearChildren(COMPILED_FUNC_ARGS) {
    HtmlElementVal* target_elm = static_cast<HtmlElementVal*>(args[0]);
    for(auto& child : target_elm->target_tag->children){
        child->unRender();
    }
    target_elm->target_tag->setChildren({});

    return nullptr;
};
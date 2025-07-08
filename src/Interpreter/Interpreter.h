#pragma once
#include "Ast.h"
#include "Parser.h"
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/cssprovider.h>

class Interpreter {
public:
    void renderTags(Gtk::Box* target_box, std::vector<std::shared_ptr<HTMLTag>> tags) {
        for(auto& tag : tags){
            tag->render(target_box);
        }
    };



};
#pragma once
#include "../StdLib.h"
#include <gtkmm/dialog.h>
#include <gtkmm/widget.h>

struct AlertButtonInformation {
    std::string text;
    std::function<void(Gtk::Dialog*)> on_click;
};

struct AlertInformation {
    std::string title;
    std::string description;
    Gtk::Window* target_window;
    std::vector<AlertButtonInformation> buttons_information;
};

class WindowLib {
public:
    static RunTimeVal* getStruct();

    static RunTimeVal* alert(COMPILED_FUNC_ARGS);
    static Gtk::Dialog* showAlert(AlertInformation information);
    static void normalizeWidget(Gtk::Widget* target_widget);
};
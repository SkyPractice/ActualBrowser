#include "WindowLib.h"
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/constraint.h>
#include <gtkmm/constrainttarget.h>
#include <gtkmm/dialog.h>
#include <gtkmm/enums.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <gtkmm/window.h>
#include <gtkmm/constraintlayout.h>
#include <pangomm/layout.h>
#include "../../SigmaInterpreter.h"

RunTimeVal* WindowLib::getStruct(){
    LibMapType funcs = {{"alert", RunTimeFactory::makeNativeFunction(&WindowLib::alert, {{"description", StringType}})}};
    
    return RunTimeFactory::makeStruct(funcs);
};

RunTimeVal* WindowLib::alert(COMPILED_FUNC_ARGS) {
    showAlert({ "Alert!", static_cast<StringVal*>(args[0])->str,
        interpreter->current_window,
        {{"Close", [](Gtk::Dialog* dia){ dia->close(); }}} });
    return nullptr;
};

Gtk::Dialog* WindowLib::showAlert(AlertInformation information) {
    Gtk::Dialog* dialog = Gtk::manage(new Gtk::Dialog("Alert", true));
    
    dialog->set_transient_for(*information.target_window);
    dialog->add_css_class("BrowserAlertWindow");
    Gtk::Box* box_sizer = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
    Gtk::Label* alert_label = Gtk::manage(new Gtk::Label(information.title));
    alert_label->add_css_class("BrowserAlertLabel");

    normalizeWidget(alert_label);

    Gtk::Label* desc_label = Gtk::manage(new Gtk::Label(information.description));
    desc_label->add_css_class("BrowserDescLabel");

    desc_label->set_max_width_chars(25);
    desc_label->set_wrap();
    desc_label->set_wrap_mode(Pango::WrapMode::WORD);

    normalizeWidget(desc_label);

    Gtk::Box* button_box = Gtk::manage(new Gtk::Box(Gtk::Orientation::HORIZONTAL));

    
    for(auto& btn : information.buttons_information){
        Gtk::Button* created_btn = Gtk::manage(new Gtk::Button(btn.text));

        created_btn->set_cursor("hand");

        normalizeWidget(created_btn);

        created_btn->add_css_class("dialogButton");
        created_btn->signal_clicked().connect([dialog, btn](){
            btn.on_click(dialog);
        });
        button_box->append(*created_btn);
    }

    dialog->set_child(*box_sizer);

    normalizeWidget(button_box);
    button_box->set_valign(Gtk::Align::END);

    box_sizer->append(*alert_label);
    box_sizer->append(*desc_label);
    box_sizer->append(*button_box);

    dialog->set_default_size(300, 150);
    dialog->show();

    return dialog;
};

void WindowLib::normalizeWidget(Gtk::Widget* target_widget) {
    target_widget->set_vexpand(false);
    target_widget->set_hexpand(false);
    target_widget->set_valign(Gtk::Align::START);
    target_widget->set_halign(Gtk::Align::START);
};
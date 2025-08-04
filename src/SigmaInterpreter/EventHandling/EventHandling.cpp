#include "../SigmaInterpreter.h"
#include <gdk/gdkkeysyms.h>
#include <gdkmm/enums.h>
#include <glib.h>
#include <gtkmm/eventcontroller.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/eventcontrollermotion.h>
#include <gtkmm/gestureclick.h>
#include "EventHandling.h"
#include "../StandardLibrary/TypeWrappers/StringWrapper.h"

void ClickEvent::bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor){
    LambdaVal* handler_ptr = handler;
    auto func = [handler_ptr, executor](int, double, double){
        executor->evaluateAnonymousLambdaCall(handler_ptr, {});
    };
    Gtk::Button* btn = dynamic_cast<Gtk::Button*>(target_widget);
    if(!btn){
        auto evt_ctrl = Gtk::GestureClick::create();
        evt_ctrl->signal_pressed().connect(func);
        target_widget->add_controller(evt_ctrl);
        return;
    }
    btn->signal_clicked().connect([executor, handler_ptr](){
        executor->evaluateAnonymousLambdaCall(handler_ptr, {});
    });
}

void KeyBoardEvent::bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor){
    LambdaVal* handler_ptr = handler;
    auto func = [executor, handler_ptr](guint keyval, guint keycode, Gdk::ModifierType modf){
        std::string str(1, keyval);
        auto k = 0xff0d;
        executor->evaluateAnonymousLambdaCall(handler_ptr, {
             StringWrapper::genObject(RunTimeFactory::makeString(str))});
    };

    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_released().connect(func);
    target_widget->add_controller(key_controller);

};

void MouseInEvent::bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor){
    LambdaVal* handler_ptr = handler;

    auto func = [executor, handler_ptr](double x, double y){
        executor->evaluateAnonymousLambdaCall(handler_ptr, {});
    };
    
    auto motion_controller = Gtk::EventControllerMotion::create();
    motion_controller->signal_enter().connect(func);

    target_widget->add_controller(motion_controller);
};

void MouseOutEvent::bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor){
    LambdaVal* handler_ptr = handler;

    auto func = [executor, handler_ptr](){
        executor->evaluateAnonymousLambdaCall(handler_ptr, {});
    };
    
    auto motion_controller = Gtk::EventControllerMotion::create();
    motion_controller->signal_leave().connect(func);
    
    target_widget->add_controller(motion_controller);
};

void MouseMotionEvent::bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor){
    LambdaVal* handler_ptr = handler;

    auto func = [executor, handler_ptr](double x, double y){
        executor->evaluateAnonymousLambdaCall(handler_ptr, {
            RunTimeFactory::makeNum(x), RunTimeFactory::makeNum(y)
        });
    };
    
    auto motion_controller = Gtk::EventControllerMotion::create();
    motion_controller->signal_motion().connect(func);
    
    target_widget->add_controller(motion_controller);
};
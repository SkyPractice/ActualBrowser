#pragma once
#include <gtkmm/widget.h>

class LambdaVal;
class SigmaInterpreter;

class Event {
public:
    LambdaVal* handler;
    virtual void bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor) = 0;
    Event(LambdaVal* evt_handler): handler(evt_handler) {};
};

class ClickEvent : public Event {
public:
    ClickEvent(LambdaVal* evt_handler): Event(evt_handler) {};

    void bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor) override;
};

class KeyBoardEvent : public Event{
public:
    KeyBoardEvent(LambdaVal* evt_handler): Event(evt_handler) {};

    void bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor) override;
};

class MouseInEvent : public Event {
public:
    MouseInEvent(LambdaVal* evt_handler): Event(evt_handler) {};

    void bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor) override;
};

class MouseOutEvent : public Event {
public:
    MouseOutEvent(LambdaVal* evt_handler): Event(evt_handler) {};

    void bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor) override;
};

class MouseMotionEvent : public Event {
public:
    MouseMotionEvent(LambdaVal* evt_handler): Event(evt_handler) {};

    void bindEventToWidget(Gtk::Widget* target_widget, SigmaInterpreter* executor) override;
};
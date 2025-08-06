#pragma once
#include <gtkmm/widget.h>
#include <string>
#include <vector>

class HTMLTag;
typedef std::vector<std::string> CssClassList;

class HTMLTagCssManager {
public:
     virtual void applyCssClasses(HTMLTag* target_tag) {};
     virtual void applyStyle(HTMLTag* target_tag) {};
     void normalizePositioning(Gtk::Widget* target_widget);
     CssClassList getCssClasses(std::string target_string);
};
class BasicTagCssManagerUtil : public HTMLTagCssManager {
public:
     void applyCssClassesUtil(HTMLTag* target_tag, Gtk::Widget* target_widget);
     void applyStyleUtil(HTMLTag* target_tag, Gtk::Widget* target_widget);
};
class ContainerTagCssManager : public BasicTagCssManagerUtil {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};
class TextTagCssManager : public HTMLTagCssManager {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};
class StringTagCssManager : public HTMLTagCssManager {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};

class ImageTagCssManager : public BasicTagCssManagerUtil {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};
class VideoTagCssManager : public BasicTagCssManagerUtil {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};
class ButtonTagCssManager : public BasicTagCssManagerUtil {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};
class InputTagCssManager : public BasicTagCssManagerUtil {
public:
     void applyCssClasses(HTMLTag* target_tag) override;
     void applyStyle(HTMLTag* target_tag) override;
};
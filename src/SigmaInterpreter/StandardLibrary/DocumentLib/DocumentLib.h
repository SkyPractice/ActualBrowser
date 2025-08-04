#pragma once
#include "../StdLib.h"
#include "../../EventHandling/EventHandling.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>

typedef std::unique_ptr<Event> EventPtr;

class DocumentLib {
public:
    static std::unordered_set<TagType> txt_tags;
    static std::unordered_map<std::string, std::function<std::unique_ptr<Event>(
        LambdaVal*
    )>> event_table;
    
    static RunTimeVal* getStruct();

    static RunTimeVal* getElementById(COMPILED_FUNC_ARGS);
    static RunTimeVal* setElementInnerHtml(COMPILED_FUNC_ARGS);
    static RunTimeVal* getElementInnerHtml(COMPILED_FUNC_ARGS);
    static RunTimeVal* getOuterHtml(COMPILED_FUNC_ARGS);
    static RunTimeVal* getContents(COMPILED_FUNC_ARGS);
    static RunTimeVal* cloneHtmlElements(COMPILED_FUNC_ARGS);
    static RunTimeVal* getText(COMPILED_FUNC_ARGS); // for inputs or text areas 
    static RunTimeVal* setText(COMPILED_FUNC_ARGS); // for inputs or text areas
    static RunTimeVal* appendChild(COMPILED_FUNC_ARGS);
    static RunTimeVal* popChild(COMPILED_FUNC_ARGS);
    static RunTimeVal* unRenderElement(COMPILED_FUNC_ARGS);
    static RunTimeVal* appendChildren(COMPILED_FUNC_ARGS);
    static RunTimeVal* removeElement(COMPILED_FUNC_ARGS);
    static RunTimeVal* createElementsFromHtml(COMPILED_FUNC_ARGS);
    static RunTimeVal* getElementsByClassName(COMPILED_FUNC_ARGS);
    static RunTimeVal* setOnClick(COMPILED_FUNC_ARGS);
    static RunTimeVal* connectEventHandler(COMPILED_FUNC_ARGS);
    static RunTimeVal* clearChildren(COMPILED_FUNC_ARGS);


};
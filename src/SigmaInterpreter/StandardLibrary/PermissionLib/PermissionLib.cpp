#include "PermissionLib.h"
#include "../WindowLib/WindowLib.h"
#include "../../SigmaInterpreter.h"
#include <gtkmm/dialog.h>
#include <unordered_map>

std::unordered_map<std::string, PermissionType> PermissionLib::perms_table = {
    {"FileReading", FileReading}, {"FileWriting", FileWriting},
    {"FileFullAccess", FileFullAccess}
};
std::unordered_map<PermissionType, std::string> PermissionLib::reverse_perms_table = {
    {FileReading, "FileReading"}, {FileWriting, "FileWriting"},
    {FileFullAccess, "FileFullAccess"}
};

RunTimeVal* PermissionLib::getStruct(){
    std::unordered_map<std::string, RunTimeVal*> func_table = {
        {"isPermissionGranted", RunTimeFactory::makeNativeFunction(&PermissionLib::isPermissionGranted, {{"permission_name", StringType}})},
        {"requestPermission", RunTimeFactory::makeNativeFunction(&PermissionLib::requestPermission, {{"permission_name", StringType}, {"handler", LambdaType}})}
    };

    return RunTimeFactory::makeStruct(func_table);
};

RunTimeVal* PermissionLib::isPermissionGranted(COMPILED_FUNC_ARGS) {
    return RunTimeFactory::makeBool(interpreter->perms.isPermissionGranted(
        perms_table.at(static_cast<StringVal*>(args[0])->str)
    ));
};

RunTimeVal* PermissionLib::requestPermission(COMPILED_FUNC_ARGS) {
    std::string perm = static_cast<StringVal*>(args[0])->str;
    LambdaVal* lambda = static_cast<LambdaVal*>(args[1]);
    interpreter->garbageCollectionRestricter.registerAsyncLambda(lambda);

    WindowLib::showAlert({"Permission Request: ",
        "The Site Is Requesting " + perm + "Permissions", 
        interpreter->current_window, {
            {"Accept", [interpreter, lambda, perm](Gtk::Dialog* dialog)
                {
                    dialog->close();
                    interpreter->perms.grantPermission(perms_table.at(perm));
                    PermissionFileController::writePermsToFile("./Config/Permissions/" + interpreter->doc_name
                        , interpreter->perms);
                    interpreter->evaluateAnonymousLambdaCall(lambda, {RunTimeFactory::makeBool(true)});
                    interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda->lambda_uuid);
                }}, {"Deny", [interpreter, lambda, perm](Gtk::Dialog* dialog)
                {
                    dialog->close();
                    interpreter->evaluateAnonymousLambdaCall(lambda, {RunTimeFactory::makeBool(false)});
                    interpreter->garbageCollectionRestricter.unRegisterAsyncLambda(lambda->lambda_uuid);
                }}}});

    return nullptr;
};
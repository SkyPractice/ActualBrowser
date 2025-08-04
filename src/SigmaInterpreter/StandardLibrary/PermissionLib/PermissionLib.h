#pragma once
#include "../StdLib.h"
#include "../../Util/Permissions/Permissions.h"

class PermissionLib {
public:
    static std::unordered_map<std::string, PermissionType> perms_table;
    static std::unordered_map<PermissionType, std::string> reverse_perms_table;

    static RunTimeVal* getStruct();

    static RunTimeVal* isPermissionGranted(COMPILED_FUNC_ARGS);
    static RunTimeVal* requestPermission(COMPILED_FUNC_ARGS);

};
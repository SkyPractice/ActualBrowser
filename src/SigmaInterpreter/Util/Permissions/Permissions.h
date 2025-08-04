#pragma once
#include <string>
#include <unordered_set>

enum PermissionType {
    NullPerm, FileReading, FileWriting, FileFullAccess,
};

//
// anything in the unordered_set<PermissionType>
// is a granted permission
//

class PermissionContainer {
public:
    std::unordered_set<PermissionType> perms;

    PermissionContainer() {};
    PermissionContainer(const PermissionContainer& other){
        perms = other.perms;
    }
    PermissionContainer(PermissionContainer&& other_container){
        perms = std::move(other_container.perms);
    }
    PermissionContainer(std::unordered_set<PermissionType> perms_arg): perms(perms_arg){};

    void operator=(PermissionContainer&& other){
        perms = std::move(other.perms);
    }
    void operator=(const PermissionContainer& other){
        perms = other.perms;
    }

    bool isPermissionGranted(PermissionType perm){
        return perms.contains(perm);
    }
    void grantPermission(PermissionType perm){
        perms.insert(perm);
    }
    void unGrantPermission(PermissionType perm){
        if(perms.contains(perm)){
            perms.erase(perm);
        }
    }
};

class PermissionFileController {
public:
    static void writePermsToFile(std::string file_name, PermissionContainer permissions);
    static PermissionContainer readPermsFromFile(std::string file_name);
};
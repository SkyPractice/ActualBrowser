#include "Permissions.h"
#include <fstream>

void PermissionFileController::writePermsToFile(std::string file_name,
    PermissionContainer permissions) {
    std::ofstream stream(file_name, std::ios::binary);

    size_t perms_size = permissions.perms.size();
    stream.write(reinterpret_cast<const char*>(&perms_size), sizeof(size_t));

    for(auto& perm : permissions.perms){
        stream.write(reinterpret_cast<const char*>(&perm), sizeof(PermissionType));
    }

    stream.close();
};
PermissionContainer PermissionFileController::readPermsFromFile(
    std::string file_name) {
    PermissionContainer perms;

    std::ifstream stream(file_name, std::ios::binary);

    size_t perms_size = 0;
    stream.read(reinterpret_cast<char*>(&perms_size), sizeof(size_t));

    for(size_t i = 0; i < perms_size; i++){
        PermissionType perm = NullPerm;
        stream.read(reinterpret_cast<char*>(&perm), sizeof(PermissionType));
        perms.perms.insert(perm);
    }

    return perms;
};
#include "FileSystem.h"

#include <stdexcept>

void FileSystem::setRoot(const Path& newRoot) {
    root = newRoot;
}

void FileSystem::set(const std::string& name, const Path& path) {
    paths[name] = path;
}

const FileSystem::Path& FileSystem::get(const std::string& name) const {
    auto it = paths.find(name);

    if (it == paths.end()) {
        throw std::runtime_error(
            "Path not registered: " + name
        );
    }

    return it->second;
}

bool FileSystem::has(const std::string& name) const {
    return paths.find(name) != paths.end();
}

FileSystem::Path FileSystem::resolve(const std::string& name,const Path& relativePath) const {
    return get(name) / relativePath;
}
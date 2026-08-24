#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

class FileSystem {
public:
    using Path = std::filesystem::path;

    void setRoot(const Path& root);
    void set(const std::string& name,const Path& path);

    const Path& get(const std::string& name) const;

    bool has(const std::string& name) const;

    Path resolve(const std::string& name,const Path& relativePath = {}) const;

private:
    Path root;
    std::unordered_map<std::string, Path> paths;
};
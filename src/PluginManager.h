#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "PlatformSymbols.h"
#include "PluginAPI.h"

namespace fs = std::filesystem;

struct LoadedPlugin {
    PluginHandle handle = nullptr;
    PluginAPI api{};
    PluginDecoder decoder = nullptr;
};

class PluginManager {
public:
    PluginManager() = default;

    ~PluginManager(){
        unloadPlugins();
    }

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    // load all supported shared libraries
    void loadPluginsFromFolder(const std::string& folderPath) {
        if (!fs::exists(folderPath)) {
            std::cerr
                << "<Plugins> ERROR: Plugin folder does not exist: "
                << folderPath
                << "\n";

            return;
        }

        if (!fs::is_directory(folderPath)) {
            std::cerr
                << "<Plugins> ERROR: Plugin path is not a directory: "
                << folderPath
                << "\n";

            return;
        }

        for (const auto& entry: fs::directory_iterator(folderPath)) {
            if (!entry.is_regular_file())
                continue;

            if (!isSharedLibrary(entry.path()))
                continue;

            loadPlugin(entry.path());
        }

        std::cout
            << "<Plugins> "
            << plugins.size()
            << " plugin(s) loaded\n";
    }

    LoadedPlugin* findDecoderForFile(const std::string& filePath) {
        std::string extension = fs::path(filePath).extension().string();

        // make extension lowercase
        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );

        LoadedPlugin* bestPlugin = nullptr;

        for (auto& plugin : plugins) {
            if (!plugin.api.supportsExtension) continue;

            bool supported = plugin.api.supportsExtension(
                plugin.decoder,
                extension.c_str()
            );

            if (!supported) continue;

            if (!bestPlugin 
                || plugin.api.priority 
                >  bestPlugin->api.priority
            ) {
                bestPlugin = &plugin;
            }
        }

        return bestPlugin;
    }
    
    bool decodeImage(const std::string& filePath, PluginImageData& imageData) {
        imageData = {};

        LoadedPlugin* plugin =
            findDecoderForFile(filePath);

        if (!plugin) {
            std::cerr
                << "<Plugins> ERROR: No suitable decoder was found for file: "
                << filePath
                << "\n";

            return false;
        }

        if (!plugin->api.decodeImage) {
            std::cerr
                << "<Plugins> ERROR: Plugin has no function \"decodeImage\": "
                << plugin->api.name
                << "\n";

            return false;
        }

        imageData = plugin->api.decodeImage(
            plugin->decoder,
            filePath.c_str()
        );

        if (!imageData.pixels) {
            std::cerr
                << "<Plugins> ERROR: Failed to decode file: "
                << filePath
                << " using "
                << plugin->api.name
                << "\n";

            return false;
        }

        return true;
    }

    void freeImageData(const std::string& filePath, PluginImageData& imageData) {
        if (!imageData.pixels) return;

        LoadedPlugin* plugin = findDecoderForFile(filePath);
        if (!plugin) return;
  
        if (plugin->api.freeImageData) {
            plugin->api.freeImageData(
                plugin->decoder,
                &imageData
            );
        }

        imageData = {};
    }

    void freeImageData(LoadedPlugin* plugin, PluginImageData& imageData) {
        if (!plugin || !imageData.pixels) return;

        if (plugin->api.freeImageData) {
            plugin->api.freeImageData(
                plugin->decoder,
                &imageData
            );
        }

        imageData = {};
    }

    void unloadPlugins() {
        for (auto& plugin : plugins) {
            if (plugin.api.destroy 
            &&  plugin.decoder
            ) {
                plugin.api.destroy(plugin.decoder);
                plugin.decoder = nullptr;
            }

            if (plugin.handle) {
                FreePluginLibrary(plugin.handle);
                plugin.handle = nullptr;
            }
        }

        plugins.clear();
    }

    size_t getPluginCount() const {
        return plugins.size();
    }

    const std::vector<LoadedPlugin>&
    getPlugins() const {
        return plugins;
    }

private:
    bool loadPlugin(const fs::path& path) {
        PluginHandle handle =
            LoadPluginLibrary(
                path.wstring()
            );

        if (!handle) return false;

        auto getPluginAPI =
            reinterpret_cast<GetPluginAPIFunc>(
                GetPluginSymbol(
                    handle,
                    "GetPluginAPI"
                )
            );

        if (!getPluginAPI) {
            std::cerr
                << "<Plugins> ERROR: Plugin does not export GetPluginAPI: "
                << path.filename().string()
                << "\n";

            FreePluginLibrary(handle);
            return false;
        }

        PluginAPI api = getPluginAPI();

        // TODO: add version priorities to allow for some backwards compatibility
        if (api.apiVersion != PLUGIN_API_VERSION) {
            std::cerr
                << "<Plugins> ERROR: Incompatible plugin version: "
                << path.filename().string()
                << "\n"
                << "  Plugin version: "
                << api.apiVersion
                << "\n"
                << "  Required version: "
                << PLUGIN_API_VERSION
                << "\n";

            FreePluginLibrary(handle);
            return false;
        }

        if (!api.name) {
            std::cerr
                << "<Plugins> ERROR: Plugin has no name: "
                << path.filename().string()
                << "\n";

            FreePluginLibrary(handle);
            return false;
        }

        if (!api.version) {
            std::cerr
                << "<Plugins> ERROR: Plugin has no version: "
                << path.filename().string()
                << "\n";

            FreePluginLibrary(handle);
            return false;
        }

        if (!api.create 
        ||  !api.destroy 
        ||  !api.supportsExtension 
        ||  !api.decodeImage 
        ||  !api.freeImageData
        ) {
            std::cerr
                << "<Plugins> ERROR: Plugin has an incomplete API: "
                << api.name
                << "\n";

            FreePluginLibrary(handle);
            return false;
        }

        PluginDecoder decoder = api.create();

        if (!decoder) {
            FreePluginLibrary(handle);
            return false;
        }

        LoadedPlugin plugin;
        plugin.handle = handle;
        plugin.api = api;
        plugin.decoder = decoder;

        plugins.push_back(plugin);

        std::cout
            << "<Plugins> Plugin loaded: "
            << api.name
            << " V: "
            << api.version
            << " P: "
            << api.priority
            << "\n";

        return true;
    }

    bool isSharedLibrary(const fs::path& path) const {
        std::string extension = path.extension().string();

        std::transform(
            extension.begin(),
            extension.end(),
            extension.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );

#if defined(_WIN32) || defined(_WIN64)
        return extension == ".dll";

#elif defined(__APPLE__)
        return extension == ".dylib";

#else
        return extension == ".so";
#endif
    }

private:
    std::vector<LoadedPlugin> plugins;
    
};
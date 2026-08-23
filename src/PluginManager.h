#pragma once

#include <filesystem>
#include <vector>
#include "PlatformSymbols.h"
#include "IImageDecoder.h"

namespace fs = std::filesystem;

struct LoadedPlugin {
    PluginHandle handle;
    IImageDecoder* decoder;
    DestroyDecoderFunc destroyFunc;
};

class PluginManager {
public:
    ~PluginManager() { unloadPlugins(); }

    // TODO: redo this, it ugly
    void loadPluginsFromFolder(const std::string& folderPath) {
        if (!fs::exists(folderPath)) return;

        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (isSharedLibrary(entry.path())) {
                PluginHandle handle = LoadPluginLibrary(entry.path().wstring());
                if (!handle) continue;

                auto createFunc = (CreateDecoderFunc)getPluginSymbol(handle, "CreateDecoder");
                auto destroyFunc = (DestroyDecoderFunc)getPluginSymbol(handle, "DestroyDecoder");

                if (createFunc && destroyFunc) {
                    IImageDecoder* decoder = createFunc();
                    plugins.push_back({ handle, decoder, destroyFunc });
                    std::cout << "<PluginManager> Plugin loaded successfully: " 
                              << entry.path().filename().string() << "\n";
                } else {
                    freePluginLibrary(handle);
                }
            }
        }
    }

    IImageDecoder* findDecoderForFile(const std::string& filePath) {
        std::string ext = fs::path(filePath).extension().string();
        for (const auto& plugin : plugins) {
            if (plugin.decoder->supportsExtension(ext)) {
                return plugin.decoder;
            }
        }
        return nullptr;
    }

    void unloadPlugins() {
        for (auto& plugin : plugins) {
            plugin.destroyFunc(plugin.decoder);
            freePluginLibrary(plugin.handle);
        }
        plugins.clear();
    }
private:
    std::vector<LoadedPlugin> plugins;

    bool isSharedLibrary(const fs::path& path) {
        auto ext = path.extension().string();
        return (ext == ".dll" || ext == ".so" || ext == ".dylib");
    }
};

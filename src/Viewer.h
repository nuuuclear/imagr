#pragma once

#include <SDL3/SDL.h>
#include <string>

#include "PluginManager.h"

class app;
class PluginManager;

class Viewer {
public:
    explicit Viewer(app* app);
    ~Viewer();

    void present(const std::string& imagePath);
    void draw();

    void loadPlugins(const std::string& dirPath);

    // TODO: image resizing with window
    // TODO: image zooming
    
private:
    app* parentApp = nullptr;
    SDL_Texture* texture = nullptr;

    PluginManager pm;

    SDL_FRect destRect{};
};
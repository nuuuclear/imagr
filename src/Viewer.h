#pragma once

#include <SDL3/SDL.h>

#include <string>

class app;
class PluginManager;

class Viewer {
public:
    Viewer(app* app);
    ~Viewer();

    void present(const std::string& imagePath, PluginManager* pm);
    void draw();

    // TODO: image resizing with window
    // TODO: image zooming
    
private:
    app* parentApp = nullptr;
    SDL_Texture* texture = nullptr;

    SDL_FRect destRect{};
};
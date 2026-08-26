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

    void rebuildRect();
    void handleEvent(const SDL_Event& event);

    void attachEvents(); // todo: make a sub-app class to handle this
    
private:
    app* parentApp = nullptr;
    SDL_Texture* texture = nullptr;

    float fitScale = 1.0f;

    float zoom = 1.0f;

    float imageX = 0.0f;
    float imageY = 0.0f;

    float mouseX = 0.0f;
    float mouseY = 0.0f;

    bool dragging = false;

    float dragStartMouseX = 0.0f;
    float dragStartMouseY = 0.0f;

    float dragStartImageX = 0.0f;
    float dragStartImageY = 0.0f;

    PluginManager pm;

    SDL_FRect destRect{};
};
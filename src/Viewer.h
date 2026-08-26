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

    void update(double deltaTime);
    void draw();

    void loadPlugins(const std::string& dirPath);

    void rebuildRect();
    void handleEvent(const SDL_Event& event);

    void attachEvents(); // todo: make a sub-app class to handle this
private:
    bool loadStaticImage(LoadedPlugin* plugin, const std::string& imagePath);
    bool loadAnimatedImage(LoadedPlugin* plugin, const std::string& imagePath);

    bool createTextureFromFrame(const PluginImageFrame& frame);
    bool updateTextureFromFrame(const PluginImageFrame& frame);

    void clearAnimation();
    void resetView();
private:
    app* parentApp = nullptr;
    SDL_Texture* texture = nullptr;

    PluginManager pm;
    LoadedPlugin* currentAnimationPlugin = nullptr;

    SDL_FRect destRect{};

    float imageX = 0.0f;
    float imageY = 0.0f;

    float mouseX = 0.0f;
    float mouseY = 0.0f;

    // zoom
    float fitScale = 1.0f;

    float zoom = 1.0f;

    static constexpr float minZoom = 0.05f;
    static constexpr float maxZoom = 20.0f;
    static constexpr float zoomFactor = 1.1f;

    // pan
    bool dragging = false;

    float dragStartMouseX = 0.0f;
    float dragStartMouseY = 0.0f;

    float dragStartImageX = 0.0f;
    float dragStartImageY = 0.0f;

    // animation
    PluginImageSequence animation{};

    int currentFrame = 0;
    double frameTime = 0.0;

    bool animated = false;
};
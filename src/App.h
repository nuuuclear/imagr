#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <any>
#include <algorithm>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

struct appConfig {
    std::string title = "title";

    int windowWidth = 256;
    int windowHeight = 240;

    int windowScale = 1;
};

class app {
public:
    using eventCallback = std::function<void()>;
    using ID = uint64_t;

    app();
    ~app();

    bool init(appConfig conf);
    void run(); // starts it running
    void quit();

    void step();

    void reset(appConfig conf);

    SDL_Window* getWindow();
    SDL_Renderer* getRenderer();

    ID addDrawCallback(eventCallback callback) {
        ID currentId = nextId++;
        drawCallbacks.push_back({currentId, callback});
        return currentId;
    }

    void removeDrawCallback(ID id) {
        drawCallbacks.erase(
            std::remove_if(drawCallbacks.begin(), drawCallbacks.end(),
                [id](const CallbackEntry& entry) { return entry.id == id; }),
            drawCallbacks.end()
        );
    }

    template<typename Callable>
    void on(const std::string& eventName, Callable&& callback) {
        auto func = std::function(std::forward<Callable>(callback));
        listeners[eventName].push_back(std::any(func));
    }

    template<typename... Args>
    void emit(const std::string& eventName, Args&&... args) {
        auto it = listeners.find(eventName);
        if (it != listeners.end()) {
            for (const auto& anyCallback : it->second) {
                using FuncType = std::function<void(Args...)>;
                try {
                    auto callback = std::any_cast<FuncType>(anyCallback);
                    callback(std::forward<Args>(args)...);
                } catch (const std::bad_any_cast&) {
                    std::cerr << "Error: Event signature mismatch for " << eventName << "\n";
                }
            }
        }
    }
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Surface* applicationSurface = nullptr;
    TTF_TextEngine* text_engine = nullptr;

    double targetFrameTime;

	uint64_t lastCounter;
	double deltaTime;

    bool running = false;

    TTF_Font* ui_font = nullptr;

    appConfig* config;

    // callbacks
    struct CallbackEntry {
        ID id;
        eventCallback callback;
    };

    ID nextId = 0;
    std::vector<CallbackEntry> drawCallbacks;

    std::unordered_map<std::string, std::vector<std::any>> listeners;
};


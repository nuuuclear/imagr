#pragma once

#include <string>
#include <vector>
#include <functional>

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
    app();
    ~app();

    bool init(appConfig conf);
    void run(); // starts it running
    void quit();

    void step();
    void draw();

    void reset(appConfig conf);

    SDL_Window* getWindow();
    SDL_Renderer* getRenderer();

    void registerUpdateHook(std::function<void(float)> callback) {
		updateCallbacks.push_back(callback);
	}

	void registerDrawHook(std::function<void(float)> callback) {
		drawCallbacks.push_back(callback);
	}
private:
	std::vector<std::function<void(float)>> updateCallbacks;
	std::vector<std::function<void(float)>> drawCallbacks;

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
};


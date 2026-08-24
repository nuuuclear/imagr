#include "App.h"

#include <utility>

app::app() {
}

app::~app() {
    SDL_DestroyWindow(window);

    TTF_CloseFont(ui_font);
    TTF_DestroyRendererTextEngine(text_engine);
}

bool app::init(appConfig conf) {
    config = &conf;
    
    window = SDL_CreateWindow(
        config->title.c_str(), 
        config->windowWidth, 
        config->windowHeight, 
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "App failed create window: %s", SDL_GetError());
        return false;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Video failed to init: %s", SDL_GetError());
    }

    bool ttf = TTF_Init();
    if (!ttf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF failed to init: %s", SDL_GetError());
        return false;
    }

    ui_font = TTF_OpenFont("./resources/monaco.ttf", 16.0f);
    if (!ui_font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font failed to load: %s", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderVSync(renderer, 1);

    lastCounter = SDL_GetPerformanceCounter();
    deltaTime = 0.0;

    targetFrameTime = 1.0 / 60;

    running = true;
    return true;
}

void app::run() {
    while (running) {
        step();
    }
}

void app::quit() {
    running = false;

    SDL_Quit();
}

void app::step() {
    uint64_t currentCounter = SDL_GetPerformanceCounter();

    deltaTime =
        (double)(currentCounter - lastCounter) /
        SDL_GetPerformanceFrequency();

    lastCounter = currentCounter;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            case SDL_EVENT_DROP_BEGIN:

                // show feedback for dragndrop
                
                break;

            case SDL_EVENT_DROP_FILE: {
                if (event.drop.data != NULL) {
                    std::string dropped_file(event.drop.data);

                    emit("fileDrop", std::as_const(dropped_file));
                    break;
                }
            }

            case SDL_EVENT_DROP_COMPLETE:

                break;
            }
    }

    for (const auto& entry: drawCallbacks) {
        entry.callback(); 
    }
}

void app::reset(appConfig conf) {
    SDL_SetWindowSize(window, 
        conf.windowWidth  * conf.windowScale,
        conf.windowHeight * conf.windowScale
    );

}

SDL_Window* app::getWindow() {
    return window;
}

SDL_Renderer* app::getRenderer() {
    return renderer;
}
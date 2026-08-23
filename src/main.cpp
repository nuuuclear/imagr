#include <SDL3/SDL.h>

#include "App.h"
#include "PluginManager.h"
#include "Viewer.h"

#include <string>
#include <memory>

const std::string pluginDir = "./plugins";

std::unique_ptr<Viewer> viewer;

void drawHook(float dt) {
    viewer->draw();
};

int main(int argc, const char** argv) {
    appConfig conf;
    conf.title = "imagr";

    PluginManager pm;
    pm.loadPluginsFromFolder(pluginDir);

    std::string imagePath = argv[1];

    app app;

    if (!app.init(conf)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "App failed to initialize: %s", SDL_GetError());

        return 1;
    }

    viewer = std::make_unique<Viewer>(&app);
    viewer->present(imagePath, &pm);

    app.registerDrawHook(drawHook);

    app.run();
    
    return 0;
}
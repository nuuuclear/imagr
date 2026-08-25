#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include "App.h"
#include "PluginManager.h"
#include "Viewer.h"

#include "Paths.h"

#include <string>
#include <memory>

int main(int argc, const char** argv) {
    appConfig conf;
    conf.title = "imagr";

    std::string imagePath = (argv[1] != nullptr) ? argv[1] : "";

    app app;

    if (!app.init(conf)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "App failed to initialize: %s", SDL_GetError());

        return 1;
    }

    auto viewer = std::make_unique<Viewer>(&app);

    auto pluginsPath = app.getFileSystem().resolve(Paths::Plugins);
    viewer->loadPlugins(pluginsPath.string());
    
    viewer->present(imagePath);

    app.on("fileDrop", [&viewer](const std::string& filePath) {
        viewer->present(filePath);
    });

    app.on("windowResize", [&viewer](void) {
        viewer->rebuildRect();
    });

    app.addDrawCallback([&viewer]() { 
        viewer->draw();
    });
    

    app.run();
    
    return 0;
}
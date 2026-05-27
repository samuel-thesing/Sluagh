#ifndef CMAKE_LIB_APP_MYLIB_MAIN_H
#define CMAKE_LIB_APP_MYLIB_MAIN_H

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "sluagh/core/Application.h"
#include "sluagh/utils/Arguments.h"
#include "sluagh/utils/Logger.h"

SDL_AppResult SDL_AppInit(void **_context, int argc, char **argv) {
	Logger::init();

	auto args = Arguments("sluagh");

#ifdef APP_CONSOLE
	args.addOptionFlag("help", "Prints this help.", "h");
	args.addOptionFlag("version", "Prints the version.", "v");
#endif

	if (!args.parseArgs(argc, argv)) {
		Logger::error("Failed to parse arguments");
		return SDL_APP_FAILURE;
	}

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		Logger::error("Failed to initialize SDL: {}", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	auto app = createApplication(args);
	if (app == nullptr) {
		return SDL_APP_SUCCESS;
	}

	app->start();

	*_context = app;

	Logger::trace("Finished system initializion");
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *_app, SDL_Event *event) {
	auto app = static_cast<Application *>(_app);

	app->onEvent(event);

	return app->shouldQuit() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *_app) {
	auto app = static_cast<Application *>(_app);

	app->onRender();

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *_app, SDL_AppResult result) {
	if (result == SDL_APP_FAILURE) {
		Logger::error("Exited with error");
	}
	if (_app == nullptr)
		return;

	Application::destroy();
}

#endif // CMAKE_LIB_APP_MYLIB_MAIN_H

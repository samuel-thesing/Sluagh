#ifndef APPLICATION_H
#define APPLICATION_H

#include <SDL3/SDL.h>

#include "sluagh/export.h"
#include "sluagh/utils/Arguments.h"

class SLUAGH_EXPORT Application {
public:
	struct Config {
		std::string title;
	};

private:
	SDL_Window *window;

	SDL_GPUDevice *gpu_device;

	SDL_GPUGraphicsPipeline* gpu_pipeline;
	SDL_GPUBuffer* vertex_buffer;
	SDL_GPUBuffer* index_buffer;

	bool quit = false;

private:
	static Application *instance;

	Application(const Config &config);
	~Application();

public:
	static Application *create(Application::Config config);
	static void destroy();

	void onEvent(SDL_Event *event);
	void onRender();

	bool shouldQuit() const { return quit; }
};

extern Application *createApplication(Arguments args);

#endif // APPLICATION_H

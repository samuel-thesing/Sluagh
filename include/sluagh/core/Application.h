#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"

#include <SDL3/SDL.h>

#include "sluagh/export.h"
#include "sluagh/render/Renderer2d.h"
#include "sluagh/utils/Arguments.h"

class SLUAGH_EXPORT Application {
public:
	struct Config {
		std::string title;
	};

private:
	std::vector<Window*> windows;
	uint32_t main_window;

	SDL_GPUDevice *gpu_device;
	Renderer2d* renderer2d;

	bool quit = false;

private:
	static Application *instance;

	Application(const Config &config);
	~Application();

public:
	static Application *create(Application::Config config);
	static void destroy();

	static SDL_GPUDevice* getGPUDevice() {
		return instance->gpu_device;
	};
	static Window* getMainWindow() { return instance->windows[instance->main_window]; }

	void onEvent(SDL_Event *event);
	void onRender();

	bool shouldQuit() const { return quit; }
};

extern Application *createApplication(Arguments args);

#endif // APPLICATION_H

#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"

#include <SDL3/SDL.h>

#include "sluagh/export.h"
#include "sluagh/render/RenderManager.h"
#include "sluagh/render/TextureManager.h"
#include "sluagh/utils/Arguments.h"

class Renderer2d;

class SLUAGH_EXPORT Application {
public:
	struct GraphicConfig {
		bool enable_multisampling;
		SDL_GPUSampleCount multisample_count;
	};

	struct Config {
		GraphicConfig graphic_config;
	};

private:
	Config config;

	std::vector<Window*> windows;
	uint32_t main_window;

	SDL_GPUDevice *gpu_device;
	RenderManager* render_manager;
	TextureManager* texture_manager;

	bool quit = false;

private:
	static Application *instance;

	Application(const Config& config);
	~Application();

public:
	static Application *create(const Config& config);
	void start();
	static void destroy();

	static SDL_GPUDevice* getGPUDevice() { return instance->gpu_device; }
	static TextureManager* getTextureManager() { return instance->texture_manager; }
	static Window* getMainWindow() { return instance->windows[instance->main_window]; }
	static const Config& getConfig() { return instance->config; }
	static SDL_GPUTextureFormat getSwapchainTextureFormat() { return getMainWindow()->getTextureFormat(); }

	RenderManager* getRenderManager() { return instance->render_manager; }

	void addWindow(Window* window);

	void onEvent(SDL_Event *event);
	void onRender();

	bool shouldQuit() const { return quit; }
};

extern Application *createApplication(Arguments args);

#endif // APPLICATION_H

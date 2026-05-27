#include "sluagh/core/Application.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

Application *Application::instance = nullptr;

Application *Application::create(Config config) {
	new Application(config);
	return instance;
}

void Application::destroy() {
	delete instance;
	instance = nullptr;
}

struct PositionColorVertex {
	float x, y, z;
	float r, g, b, a;
};

Application::Application(const Config &config) {
	instance = this;

	gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
	if (gpu_device == nullptr) {
		Logger::critical("Failed to create GPU device: {}", SDL_GetError());
		throw std::runtime_error("Failed to create GPU device");
	}

	main_window = 0;
	windows.push_back(new Window(config.title, {800, 600}));

	renderer2d = new Renderer2d(gpu_device, SDL_GetGPUSwapchainTextureFormat(gpu_device, getMainWindow()->getSDLWindow()));

	Logger::trace("Finished initializing application");

}

Application::~Application() {
	SDL_DestroyGPUDevice(gpu_device);
}

void Application::onEvent(SDL_Event *event) {
	if (event->type == SDL_EVENT_QUIT) {
		quit = true;
		return;
	}

	if (event->type == SDL_EVENT_KEY_UP) {
		if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
			quit = true;
		}
	}
}

void Application::onRender() {
	SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(gpu_device);
	if (cmd_buf == nullptr) {
		Logger::critical("Failed to acquire GPU command buffer");
		throw std::runtime_error("Failed to acquire GPU command buffer");
	}
	for (auto* window : windows) {
		window->startRenderFrame(cmd_buf);
	}

	renderer2d->setCommandBuffer(cmd_buf);

	renderer2d->beginPass();
	renderer2d->setColor(Colors::AQUAMARINE);
	renderer2d->fillRect({-0.1f, -0.1f}, {0.2f, 0.2f});
	renderer2d->endPass();

	renderer2d->beginPass(1);
	renderer2d->setColor(Colors::IVORY);
	renderer2d->fillRect({0.f, 0.f}, {0.5f, 0.5f});
	renderer2d->endPass();

	renderer2d->renderAll();

	SDL_SubmitGPUCommandBuffer(cmd_buf);
}

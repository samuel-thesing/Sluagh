#include "sluagh/core/Application.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

Application *Application::instance = nullptr;

Application *Application::create() {
	new Application();
	return instance;
}
void Application::start() {
	instance->renderer2d = new Renderer2d(
		instance->gpu_device,
		SDL_GetGPUSwapchainTextureFormat(
			instance->gpu_device,
			getMainWindow()->getSDLWindow()
		)
	);
}

void Application::destroy() {
	delete instance;
	instance = nullptr;
}

struct PositionColorVertex {
	float x, y, z;
	float r, g, b, a;
};

Application::Application() {
	instance = this;

	gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
	if (gpu_device == nullptr) {
		Logger::critical("Failed to create GPU device: {}", SDL_GetError());
		throw std::runtime_error("Failed to create GPU device");
	}

	main_window = 0;

	Logger::trace("Finished initializing application");
}

Application::~Application() {
	SDL_DestroyGPUDevice(gpu_device);
}

void Application::addWindow(Window *window) {
	windows.push_back(window);
}

void Application::onEvent(SDL_Event *event) {
	if (event->type == SDL_EVENT_QUIT) {
		quit = true;
		return;
	}

	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
		auto window_id = event->window.windowID;
		uint32_t window_idx = -1u;
		for (uint32_t i = 0; i < windows.size(); i++) {
			if (windows[i]->getWindowId() == window_id) {
				window_idx = i;
				break;
			}
		}
		if (window_idx == -1u) {
			Logger::error("Received window close-request for window not managed by application");
			return;
		}

		if (main_window == window_idx && windows.size() != 1) {
			Logger::error("Close the main window last or switch the main window before closing");
			main_window = 0;
		}
		delete windows[window_idx];
		windows.erase(windows.begin() + window_idx);
		if (window_idx < main_window) {
			main_window--;
		}
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

	if (windows.empty()) {
		return;
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

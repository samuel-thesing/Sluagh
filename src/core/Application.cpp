#include "sluagh/core/Application.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Color.h"
#include "sluagh/utils/Logger.h"

#include "sluagh/render/Renderer2d.h"

Application *Application::instance = nullptr;

Application *Application::create(const Config& config) {
	new Application(config);
	return instance;
}
void Application::start() {
	texture_manager->loadFromFile("./rsc/textures/dog.png");
}

void Application::destroy() {
	delete instance;
	instance = nullptr;
}

Application::Application(const Config& config) : config(config) {
	instance = this;

	SDL_SetHint(SDL_HINT_RENDER_VULKAN_DEBUG, "1");

	SDL_PropertiesID gpu_device_props = SDL_CreateProperties();
	SDL_SetBooleanProperty(gpu_device_props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);

	SDL_GPUVulkanOptions vulkan_options = {
		.vulkan_api_version = static_cast<uint32_t>((1 << 22u) | (4 << 12u) | 350)
	};

	SDL_SetPointerProperty(gpu_device_props, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_OPTIONS_POINTER, &vulkan_options);

	gpu_device = SDL_CreateGPUDeviceWithProperties(gpu_device_props);
	if (gpu_device == nullptr) {
		Logger::critical("Failed to create GPU device: {}", SDL_GetError());
		throw std::runtime_error("Failed to create GPU device");
	}

	main_window = 0;

	render_manager = new RenderManager();
	texture_manager = new TextureManager();

	Logger::trace("Finished initializing application");
}

Application::~Application() {
	delete render_manager;
	delete texture_manager;

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
	auto* renderer2d = render_manager->getRenderer<Renderer2d>();

	renderer2d->setCommandBuffer(cmd_buf);

	renderer2d->beginPass(1);
	renderer2d->setColor(Colors::RED);
	renderer2d->fillRect({200.f, 200.f}, {150.f, 125.f});
	renderer2d->endPass();

	renderer2d->beginPass();
	renderer2d->setColor(Colors::AQUAMARINE);
	renderer2d->fillRect({300.f, 300.f}, {100.f, 50.f});
	renderer2d->setColor(Colors::GREEN);
	renderer2d->drawBezier({0.f, 0.f}, {100.f, 0.f}, {0.f, 100.f}, {100.f, 100.f}, 20.f);
	renderer2d->endPass();

	renderer2d->beginPass();
	renderer2d->setColor(Colors::GREEN);
	renderer2d->drawTexture(texture_manager->getTexture(1)->getTexture(), {0.f, 200.f}, {200.f, 200.f});
	renderer2d->endPass();

	render_manager->flushPasses();

	SDL_SubmitGPUCommandBuffer(cmd_buf);
}

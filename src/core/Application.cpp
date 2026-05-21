#include "sluagh/core/Application.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

Application *Application::instance = nullptr;

Application *Application::create(Config config) {
	instance = new Application(config);
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
	window = SDL_CreateWindow(config.title.c_str(), 800, 600, 0);
	if (window == nullptr) {
		Logger::critical("Failed to create window");
		throw std::runtime_error("Failed to create window");
	}

	gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
	if (gpu_device == nullptr) {
		Logger::critical("Failed to create GPU device: {}", SDL_GetError());
		throw std::runtime_error("Failed to create GPU device");
	}

	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
		Logger::critical("Failed to claim window for GPU device");
		throw std::runtime_error("Failed to claim window for GPU device");
	}

	renderer2d = new Renderer2d(gpu_device, SDL_GetGPUSwapchainTextureFormat(gpu_device, window));

	Logger::trace("Finished initializing application");

}

Application::~Application() {
	SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
	SDL_DestroyGPUDevice(gpu_device);
	SDL_DestroyWindow(window);
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

	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(
			cmd_buf, window, &swapchain_texture, nullptr, nullptr)) {
		Logger::critical("Failed to wait for swapchain texture");
		throw std::runtime_error("Failed to wait for swapchain texture");
	}

	if (swapchain_texture != nullptr) {
		SDL_GPUColorTargetInfo color_target = {0};
		color_target.mip_level = 0;
		color_target.texture = swapchain_texture;
		color_target.clear_color = SDL_FColor{0.f, 0.f, 0.f, 1.0f};
		color_target.load_op = SDL_GPU_LOADOP_CLEAR;
		color_target.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass *render_pass =
			SDL_BeginGPURenderPass(cmd_buf, &color_target, 1, nullptr);
		SDL_EndGPURenderPass(render_pass);

		renderer2d->setCommandBuffer(cmd_buf);

		renderer2d->beginPass();

		renderer2d->setColor(Colors::BLUE);
		renderer2d->fillRect({0.f, 0.f}, {0.5f, 0.5f});

		renderer2d->endPassWindow(swapchain_texture);

		renderer2d->renderAll();
	}
	SDL_SubmitGPUCommandBuffer(cmd_buf);
}

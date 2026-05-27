#include "sluagh/core/Window.h"

#include "sluagh/core/Application.h"
#include "sluagh/render/Renderer.h"
#include "sluagh/utils/Logger.h"

Window::Window(const std::string& title, Vec2i size)
	: title(title), size(size) {
	sdl_window = SDL_CreateWindow(title.c_str(), size.x(), size.y(), 0);
	if (sdl_window == nullptr) {
		Logger::critical("Failed to create window '{}'", title);
		throw std::runtime_error("Failed to create window");
	}

	if (!SDL_ClaimWindowForGPUDevice(Application::getGPUDevice(), sdl_window)) {
		Logger::critical("Failed to claim window '{}' for GPU device", title);
		throw std::runtime_error("Failed to claim window for GPU device");
	}

	render_target = new RenderTarget(RenderTargetType::Window, size);
}

Window::~Window() {
	SDL_ReleaseWindowFromGPUDevice(Application::getGPUDevice(), sdl_window);
	SDL_DestroyWindow(sdl_window);
}

void Window::setTitle(const std::string &new_title) {
	title = new_title;
	SDL_SetWindowTitle(sdl_window, title.c_str());
}

void Window::resize(Vec2i new_size) {
	size = new_size;
	SDL_SetWindowSize(sdl_window, new_size.x(), new_size.y());
}
void Window::startRenderFrame(SDL_GPUCommandBuffer* command_buffer) {
	SDL_GPUTexture* swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(
			command_buffer,
			sdl_window,
			&swapchain_texture,
			nullptr, nullptr
		)) {
		Logger::critical("Failed to acquire swapchain texture for GPU command buffer '{}'", title);
		throw std::runtime_error("Failed to acquire swapchain texture for GPU command buffer");
	}
	render_target->setTexture(swapchain_texture);

	SDL_GPUColorTargetInfo color_target = {
		.texture = swapchain_texture,
		.mip_level = 0,
		.clear_color = SDL_FColor{0.f, 0.f, 0.f, 1.0f},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};

	SDL_GPUDepthStencilTargetInfo depth_target = {
		.texture = render_target->getDepthBuffer(),
		.clear_depth = 1.0f,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};

	SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
		command_buffer, &color_target, 1,
		&depth_target
	);
	SDL_EndGPURenderPass(render_pass);
}
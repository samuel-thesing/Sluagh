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

	gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL, true, nullptr);
	if (gpu_device == nullptr) {
		Logger::critical("Failed to create GPU device: {}", SDL_GetError());
		throw std::runtime_error("Failed to create GPU device");
	}

	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
		Logger::critical("Failed to claim window for GPU device");
		throw std::runtime_error("Failed to claim window for GPU device");
	}

	Logger::trace("Finished initializing application");


	SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
	SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer);
}

Application::~Application() {
	SDL_ReleaseGPUGraphicsPipeline(gpu_device, gpu_pipeline);
	SDL_ReleaseGPUBuffer(gpu_device, vertex_buffer);

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
		color_target.clear_color = SDL_FColor{0.2f, 0.6f, 0.8f, 1.0f};
		color_target.load_op = SDL_GPU_LOADOP_CLEAR;
		color_target.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass *render_pass =
			SDL_BeginGPURenderPass(cmd_buf, &color_target, 1, nullptr);

		SDL_BindGPUGraphicsPipeline(render_pass, gpu_pipeline);
		SDL_GPUBufferBinding vertex_binding = {
			.buffer = vertex_buffer,
			.offset = 0
		};
		SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
		SDL_GPUBufferBinding index_binding = {
			.buffer = index_buffer,
			.offset = 0
		};
		SDL_BindGPUIndexBuffer(render_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
		SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);

		SDL_EndGPURenderPass(render_pass);
	}
	SDL_SubmitGPUCommandBuffer(cmd_buf);
}

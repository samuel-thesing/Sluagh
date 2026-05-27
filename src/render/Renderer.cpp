#include "sluagh/render/Renderer.h"

#include "sluagh/core/Application.h"

uint32_t RenderTarget::next_id = 1;
RenderTarget::RenderTarget(RenderTargetType type, Vec2i size,
	bool clear_depth)
	: id(next_id++), type(type), size(size), texture(nullptr), clear_depth(clear_depth) {
	SDL_GPUTextureCreateInfo depth_texture_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
		.width = static_cast<uint32_t>(size.x()),
		.height = static_cast<uint32_t>(size.y()),
		.layer_count_or_depth = 1,
		.num_levels = 1
	};
	depth_buffer = SDL_CreateGPUTexture(Application::getGPUDevice(), &depth_texture_info);

	projection_matrix = Mat4f::ortho(0.f, static_cast<float>(size.x()), static_cast<float>(size.y()), 0.f, 100.f, 0.f);
}
RenderTarget::~RenderTarget() {
	SDL_ReleaseGPUTexture(Application::getGPUDevice(), depth_buffer);
}

void RenderTarget::setTexture(SDL_GPUTexture* new_texture) {
	texture = new_texture;
}

RenderPass::RenderPass(RenderTarget *target) : target{target} {}

void Renderer::addRenderPass(RenderPass *pass) {
	render_passes.push_back(pass);
}

void Renderer::setCommandBuffer(SDL_GPUCommandBuffer *current_command_buffer) {
	command_buffer = current_command_buffer;
}

void Renderer::renderAll() { // todo
	for (auto* pass : render_passes) {
		pass->render();
	}

	for (auto *pass : render_passes) {
		delete pass;
	}
	render_passes.clear();
}

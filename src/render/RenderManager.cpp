#include "sluagh/render/RenderManager.h"

#include "sluagh/core/Application.h"
#include "sluagh/render/Renderer.h"
#include "sluagh/utils/Logger.h"

uint32_t RenderTarget::next_id = 1;
RenderTarget::RenderTarget(RenderTargetType type, Vec2i size,
		SDL_GPUTextureFormat format, bool clear_depth)
	: id(next_id++), type(type), size(size), texture(nullptr), clear_depth(clear_depth) {
	const auto& config = Application::getConfig().graphic_config;
	auto sample_count = SDL_GPU_SAMPLECOUNT_1;
	if (config.enable_multisampling) {
		sample_count = config.multisample_count;
		if (!SDL_GPUTextureSupportsSampleCount(Application::getGPUDevice(), format, sample_count)) {
			Logger::critical("Sample count {} not supported", 1 << sample_count);
			throw std::runtime_error("Sample count not supported");
		}
	}

	SDL_GPUTextureCreateInfo depth_texture_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
		.width = static_cast<uint32_t>(size.x()),
		.height = static_cast<uint32_t>(size.y()),
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = sample_count
	};

	depth_buffer = SDL_CreateGPUTexture(Application::getGPUDevice(), &depth_texture_info);
	if (depth_buffer == nullptr) {
		Logger::error("Failed to create depth buffer");
		throw std::runtime_error("Failed to create depth buffer");
	}

	if (config.enable_multisampling) {
		if (format == SDL_GPU_TEXTUREFORMAT_INVALID) {
			format = Application::getSwapchainTextureFormat();
		}

		SDL_GPUTextureCreateInfo msaa_texture_info = {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = format,
			.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
			.width = static_cast<uint32_t>(size.x()),
			.height = static_cast<uint32_t>(size.y()),
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = config.multisample_count
		};
		multisample_texture = SDL_CreateGPUTexture(Application::getGPUDevice(), &msaa_texture_info);
		if (multisample_texture == nullptr) {
			Logger::error("Failed to create multisample texture");
			throw std::runtime_error("Failed to create multisample texture");
		}
	}

	projection_matrix = Mat4f::ortho(0.f, static_cast<float>(size.x()), static_cast<float>(size.y()), 0.f, 100.f, 0.f);
}
RenderTarget::~RenderTarget() {
	SDL_ReleaseGPUTexture(Application::getGPUDevice(), depth_buffer);
	if (multisample_texture != nullptr) {
		SDL_ReleaseGPUTexture(Application::getGPUDevice(), multisample_texture);
	}
}

void RenderTarget::setTexture(SDL_GPUTexture* new_texture) {
	texture = new_texture;
}

RenderPass::RenderPass(RenderTarget *target) : target{target} {}

RenderManager::~RenderManager() {
	for (auto& [_, renderer] : renderers) {
		delete renderer;
	}
}

void RenderManager::addRenderPass(RenderPass *render_pass) {
	render_passes.push_back(render_pass);
}

void RenderManager::flushPasses() {
	for (auto* pass : render_passes) {
		pass->render();
	}

	for (auto *pass : render_passes) {
		delete pass;
	}
	render_passes.clear();
}

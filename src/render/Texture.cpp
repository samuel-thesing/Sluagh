#include "sluagh/render/Texture.h"

#include "sluagh/core/Application.h"

void Texture::uploadTexture(void *data) {
	SDL_GPUTextureCreateInfo create_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = size.x(),
		.height = size.y(),
		.layer_count_or_depth = 1,
		.num_levels = 1
	};
	texture = SDL_CreateGPUTexture(Application::getGPUDevice(), &create_info);

	uint32_t full_size = size.x() * size.y() * 4;

	SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = full_size
	};
	auto* transfer_buffer = SDL_CreateGPUTransferBuffer(
		Application::getGPUDevice(), &transfer_buffer_info
	);
	auto* transfer_data = static_cast<uint8_t *>(SDL_MapGPUTransferBuffer(
		Application::getGPUDevice(), transfer_buffer, true)
	);
	SDL_memcpy(transfer_data, data, full_size);
	SDL_UnmapGPUTransferBuffer(Application::getGPUDevice(), transfer_buffer);

	auto* cmd_buffer = SDL_AcquireGPUCommandBuffer(Application::getGPUDevice());
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);

	SDL_GPUTextureTransferInfo transfer_info = {
		.transfer_buffer = transfer_buffer,
		.offset = 0
	};
	SDL_GPUTextureRegion target_loc = {
		.texture = texture,
		.w = size.x(),
		.h = size.y(),
		.d = 1
	};
	SDL_UploadToGPUTexture(
		copy_pass,
		&transfer_info,
		&target_loc,
		false
	);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(cmd_buffer);

	SDL_ReleaseGPUTransferBuffer(
		Application::getGPUDevice(), transfer_buffer
	);
}

Texture::Texture() : texture(nullptr) {}

Texture::~Texture() {
	if (texture != nullptr) {
		SDL_ReleaseGPUTexture(Application::getGPUDevice(), texture);
		texture = nullptr;
		size.x() = 0;
		size.y() = 0;
	}
}
void Texture::loadFromFile(const std::string &path) {
	auto* surface = SDL_LoadPNG(path.c_str());
	if (surface == nullptr) {
		Logger::error("Failed to load image '{}'", path);
		throw std::runtime_error("Failed to load image.");
	}

	SDL_PixelFormat surface_format = SDL_PIXELFORMAT_ABGR8888;
	if (surface->format != surface_format) {
		SDL_Surface* converted = SDL_ConvertSurface(surface, surface_format);
		SDL_DestroySurface(surface);
		surface = converted;
	}

	size.x() = static_cast<uint32_t>(surface->w);
	size.y() = static_cast<uint32_t>(surface->h);

	uploadTexture(surface->pixels);
}
void Texture::createFromData(void *data, Vec2u texture_size) {
	size = texture_size;
	uploadTexture(data);
}
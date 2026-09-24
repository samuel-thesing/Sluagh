#include "sluagh/render/RenderContext.h"

VertexBuffer RenderContext::createVertexBuffer(uint32_t stride, uint32_t capacity, const void *data) {
	SDL_GPUBufferCreateInfo buffer_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = capacity * stride
	};
	auto* sdl_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
	auto vb = VertexBuffer{Buffer(sdl_buffer, capacity * stride), stride};
	return vb;
}

IndexBuffer RenderContext::createIndexBuffer(IndexType index_type, uint32_t capacity, const void *data) {
	const auto stride = static_cast<uint32_t>(index_type);
	SDL_GPUBufferCreateInfo buffer_info = {
		.usage = SDL_GPU_BUFFERUSAGE_INDEX,
		.size = capacity * stride
	};
	auto* sdl_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
	auto ib = IndexBuffer{Buffer(sdl_buffer, capacity * stride), index_type};
	return ib;
}

Texture RenderContext::createTexture2d(
	Vec2u size, SDL_GPUTextureFormat format) {
	SDL_GPUTextureCreateInfo info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = format,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = size.x(),
		.height = size.y(),
		.layer_count_or_depth = 1,
		.num_levels = 1
	};
	auto* sdl_texture = SDL_CreateGPUTexture(device, &info);
	auto texture = Texture(sdl_texture, size, format);
	return texture;
}

void RenderContext::destroy(Buffer &buffer) {
	if (buffer.buffer != nullptr) {
		SDL_ReleaseGPUBuffer(device, buffer.buffer);
		buffer.buffer = nullptr;
	}
}
void RenderContext::destroy(Texture &texture) {
	if (texture.texture != nullptr) {
		SDL_ReleaseGPUTexture(device, texture.texture);
		texture.texture = nullptr;
	}
}
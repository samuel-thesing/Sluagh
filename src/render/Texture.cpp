#include "sluagh/render/Texture.h"

#include "sluagh/core/Application.h"

Texture::Texture(SDL_GPUTexture *texture, Vec2u size, SDL_GPUTextureFormat format)
	: texture{texture}, size{size}, format{format} {}

Texture::Texture() : texture(nullptr) {}

Texture::~Texture() {
	if (texture != nullptr) {
		SDL_ReleaseGPUTexture(Application::getGPUDevice(), texture);
		texture = nullptr;
		size.x() = 0;
		size.y() = 0;
	}
}

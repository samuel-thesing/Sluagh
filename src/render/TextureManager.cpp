#include "sluagh/render/TextureManager.h"

#include "sluagh/core/Application.h"

TextureManager::TextureManager() {
	auto white_texture = new Texture();
	uint32_t data = 0;
	white_texture->createFromData(&data, Vec2u(1, 1));
	textures.push_back(white_texture);

	transfer_capacity = 800 * 600 * 4;
	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = transfer_capacity
	};
	transfer_buffer = SDL_CreateGPUTransferBuffer(
		Application::getGPUDevice(), &transfer_buffer_create_info
	);
}
TextureManager::~TextureManager() {
	SDL_ReleaseGPUTransferBuffer(Application::getGPUDevice(), transfer_buffer);

	for (auto* texture : textures) {
		if (texture != nullptr) {
			delete texture;
		}
	}
	textures.clear();
}

uint32_t TextureManager::loadFromFile(const std::string &path) {
	auto* texture = new Texture();
	texture->loadFromFile(path);
	textures.push_back(texture);
	return static_cast<uint32_t>(textures.size()) - 1;
}

uint32_t TextureManager::createFromData(void *data, Vec2u size) {
	auto* texture = new Texture();
	texture->createFromData(data, size);
	textures.push_back(texture);
	return static_cast<uint32_t>(textures.size()) - 1;
}

Texture * TextureManager::getTexture(uint32_t id) {
	if (id > textures.size()) {
		Logger::error("Trying to access texture with invalid id {}", id);
		return nullptr;
	}

	return textures[id];
}
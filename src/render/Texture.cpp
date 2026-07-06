#include "sluagh/render/Texture.h"

#include "sluagh/core/Application.h"

Texture::Texture() : texture(nullptr) {}

Texture::~Texture() {
	if (texture != nullptr) {
		SDL_ReleaseGPUTexture(Application::getGPUDevice(), texture);
		texture = nullptr;
		size.x() = 0;
		size.y() = 0;
	}
}
bool Texture::loadFromFile(const std::string &path) {
	SDL_Surface* surface = SDL_LoadPNG(path.c_str());
	if (surface == nullptr) {
		Logger::error("Failed to load image '{}'", path);
		throw std::runtime_error("Failed to load image.");
	}


}
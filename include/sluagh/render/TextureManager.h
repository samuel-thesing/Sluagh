#ifndef SLUAGH_TEXTUREMANAGER_H
#define SLUAGH_TEXTUREMANAGER_H

#include "sluagh/render/Texture.h"

#include <vector>

class TextureManager {
private:
	std::vector<Texture*> textures;
	std::vector<uint32_t> requested;
	SDL_GPUTransferBuffer* transfer_buffer;
	uint32_t transfer_capacity;

public:
	TextureManager();
	~TextureManager();

	/**
	 * Loads image to surface with 4 channels. Does not upload to gpu
	 * @return texture id
	 */
	uint32_t loadFromFile(const std::string& path);

	/**
	 * Loads image to surface with 4 channels. Does not upload to gpu
	 * @return texture id
	 */
	uint32_t createFromData(void* data, Vec2u size);

	Texture* getTexture(uint32_t id);
};

#endif // SLUAGH_TEXTUREMANAGER_H

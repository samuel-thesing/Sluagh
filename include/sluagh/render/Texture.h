#ifndef SLUAGH_TEXTURE_H
#define SLUAGH_TEXTURE_H

#include "sluagh/utils/Vector.h"

#include <SDL3/SDL_gpu.h>

class Texture {
protected:
	Vec2u size;
	SDL_GPUTextureFormat format;
	SDL_GPUTexture* texture;

public:
	Texture();
	virtual ~Texture();

	bool loadFromFile(const std::string& path);

	Vec2u getSize() const { return size; }
	SDL_GPUTextureFormat getFormat() const { return format; }
	SDL_GPUTexture* getTexture() const { return texture; }
};

#endif // SLUAGH_TEXTURE_H

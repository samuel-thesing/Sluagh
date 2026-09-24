#ifndef SLUAGH_TEXTURE_H
#define SLUAGH_TEXTURE_H

#include "sluagh/utils/Vector.h"

#include <SDL3/SDL_gpu.h>

class Texture {
	friend class RenderContext;
protected:
	SDL_GPUTexture* texture;
	Vec2u size;
	SDL_GPUTextureFormat format;

private:
	Texture(SDL_GPUTexture* texture, Vec2u size, SDL_GPUTextureFormat format);

public:
	Texture();
	virtual ~Texture();

	Vec2u getSize() const { return size; }
	SDL_GPUTextureFormat getFormat() const { return format; }
	SDL_GPUTexture* getTexture() const { return texture; }
};

#endif // SLUAGH_TEXTURE_H

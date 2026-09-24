#ifndef SLUAGH_RENDERGRAPHTEXTURE_H
#define SLUAGH_RENDERGRAPHTEXTURE_H

#include "Texture.h"
#include "sluagh/utils/Vector.h"

struct RenderGraphTexture {
public:
	struct Desc {
		Vec2u size;
		SDL_GPUTextureFormat format;

		bool operator==(const Desc&) const = default;
	};
private:
	Texture* texture;

public:
	void create(const Desc &desc, void *allocator);
	void destroy(const Desc &desc, void *allocator);

	static std::string toString(const Desc &desc);
};

template<>
struct std::hash<RenderGraphTexture::Desc> {
	uint32_t operator()(const RenderGraphTexture::Desc &desc) const;
};

#endif // SLUAGH_RENDERGRAPHTEXTURE_H

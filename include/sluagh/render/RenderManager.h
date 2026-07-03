#ifndef SLUAGH_RENDERMANAGER_H
#define SLUAGH_RENDERMANAGER_H

#include <SDL3/SDL.h>

#include <vector>

#include "sluagh/utils/Matrix.h"

enum class RenderTargetType {
	Undefined,
	Window,
	Texture
};

class RenderTarget {
private:
	static uint32_t next_id;

private:
	uint32_t id;
	RenderTargetType type;
	Vec2i size;
	SDL_GPUTexture* texture;
	SDL_GPUTexture* depth_buffer;
	SDL_GPUTexture* multisample_texture;
	bool clear_depth = true;
	Mat4f projection_matrix;

public:
	RenderTarget(RenderTargetType type, Vec2i size,
		SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_INVALID,
		bool clear_depth = true);
	~RenderTarget();

	RenderTargetType getType() const { return type; }
	Vec2i getSize() const { return size; }
	const Mat4f& getProjectionMatrix() const { return projection_matrix; }
	SDL_GPUTexture* getTexture() const { return texture; }
	SDL_GPUTexture* getDepthBuffer() const { return depth_buffer; }
	bool isClearDepth() const { return clear_depth; }
	bool isMultisample() const { return multisample_texture != nullptr; }
	SDL_GPUTexture* getMultisampleTexture() const { return multisample_texture; }

	void setTexture(SDL_GPUTexture* new_texture);

	uint32_t getId() const { return id; }
};

class RenderPass {
private:
	static uint32_t next_id;

protected:
	uint32_t id;
	RenderTarget* target;

public:
	RenderPass(RenderTarget* target);
	virtual ~RenderPass() = default;

	virtual void render() = 0;

	RenderTarget* getRenderTarget() { return target; }
};

class RenderManager {
private:
	std::vector<RenderPass*> render_passes; // ordered by z then by when they ended

	// integrate effects as passes. how to "sort" them?

	// types of effects:
	// apply to current state vs after/before specific z
	// forbid apply to current state and instead force to use separate pass
	// => bind to z-layers or specific passes -> how to id passes?

	// examples:
	// z-0 blur z-1 => sort by z

	// list renderers?
	// default renderer?

public:
	RenderManager() = default;
	~RenderManager() = default;
};

#endif // SLUAGH_RENDERMANAGER_H

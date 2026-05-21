#ifndef SLUAGH_RENDERER_H
#define SLUAGH_RENDERER_H
#include "SDL3/SDL_gpu.h"
#include "sluagh/export.h"
#include "sluagh/utils/Vector.h"

#include <vector>

enum class RenderTargetType {
	Undefined,
	Window,
	Texture
};

struct RenderTarget {
	RenderTargetType type;
	Vec2i size;
	SDL_GPUTexture* texture;
};

class RenderPass {
protected:
	int z_index = 0;
	RenderTarget target;

public:
	RenderPass() = default;
	virtual ~RenderPass() = default;

	virtual void render() = 0;

	int getZIndex() const { return z_index; }

	void setRenderTarget(RenderTargetType type, Vec2i size, SDL_GPUTexture *texture);
	const RenderTarget& getRenderTarget() const { return target; }
};

class SLUAGH_EXPORT Renderer {
private:
	std::vector<RenderPass*> render_passes;

protected:
	SDL_GPUCommandBuffer* command_buffer;

protected:
	void insertRenderPass(RenderPass* pass);

public:
	Renderer() = default;
	virtual ~Renderer() = default;

	void setCommandBuffer(SDL_GPUCommandBuffer* current_command_buffer);

	void renderAll();
};

#endif // SLUAGH_RENDERER_H

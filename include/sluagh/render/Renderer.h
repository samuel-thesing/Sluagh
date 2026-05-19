#ifndef SLUAGH_RENDERER_H
#define SLUAGH_RENDERER_H
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

	void setRenderTarget(RenderTargetType type, Vec2i size);
};

class Renderer {
private:
	std::vector<RenderPass*> render_passes;

protected:
	void insertRenderPass(RenderPass* pass);

public:
	Renderer() = default;
	virtual ~Renderer() = default;
};

#endif // SLUAGH_RENDERER_H

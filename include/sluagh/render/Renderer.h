#ifndef SLUAGH_RENDERER_H
#define SLUAGH_RENDERER_H
#include "SDL3/SDL_gpu.h"
#include "sluagh/export.h"
#include "sluagh/utils/Matrix.h"
#include "sluagh/utils/Vector.h"

#include <vector>

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
	bool clear_depth = true;
	Mat4f projection_matrix;

public:
	RenderTarget(RenderTargetType type, Vec2i size, bool clear_depth = true);
	~RenderTarget();

	RenderTargetType getType() const { return type; }
	Vec2i getSize() const { return size; }
	const Mat4f& getProjectionMatrix() const { return projection_matrix; }
	SDL_GPUTexture* getTexture() const { return texture; }
	SDL_GPUTexture* getDepthBuffer() const { return depth_buffer; }
	bool isClearDepth() const { return clear_depth; }

	void setTexture(SDL_GPUTexture* new_texture);

	uint32_t getId() const { return id; }
};

class RenderPass {
protected:
	RenderTarget* target;

public:
	RenderPass(RenderTarget* target);
	virtual ~RenderPass() = default;

	virtual void render() = 0;

	RenderTarget* getRenderTarget() { return target; }
};

class SLUAGH_EXPORT Renderer {
private:
	std::vector<RenderPass*> render_passes;

protected:
	SDL_GPUCommandBuffer* command_buffer;

protected:
	void addRenderPass(RenderPass* pass);

public:
	Renderer() = default;
	virtual ~Renderer() = default;

	void setCommandBuffer(SDL_GPUCommandBuffer* current_command_buffer);

	void renderAll();
};

#endif // SLUAGH_RENDERER_H

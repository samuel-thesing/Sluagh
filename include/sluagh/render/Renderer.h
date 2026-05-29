#ifndef SLUAGH_RENDERER_H
#define SLUAGH_RENDERER_H
#include "sluagh/render/RenderManager.h"
#include "SDL3/SDL_gpu.h"
#include "sluagh/export.h"
#include "sluagh/utils/Matrix.h"
#include "sluagh/utils/Vector.h"

#include <vector>

class SLUAGH_EXPORT Renderer {
protected:
	SDL_GPUCommandBuffer* command_buffer;
	RendererManager* manager;

protected:
	void addRenderPass(RenderPass* pass);

public:
	Renderer() = default;
	virtual ~Renderer() = default;

	void setCommandBuffer(SDL_GPUCommandBuffer* current_command_buffer);

	void renderAll();
};

#endif // SLUAGH_RENDERER_H

#ifndef SLUAGH_RENDERER2D_H
#define SLUAGH_RENDERER2D_H
#include "Renderer.h"
#include "SDL3/SDL_gpu.h"

#include <vector>

#include "sluagh/utils/Color.h"
#include "sluagh/utils/Vector.h"

#include <stack>
#include <unordered_map>

class Renderer2d;
struct RenderCommand2d {
	unsigned position_count;
	unsigned index_count;
	Vec2f* positions;
	uint16_t* indices;
	Color color;
	// texture

	RenderCommand2d(uint32_t position_count, uint32_t index_count);
};

class RenderPass2d : public RenderPass {
private:
	Renderer2d* renderer;
	std::vector<RenderCommand2d> render_commands;

public:
	RenderPass2d(Renderer2d* renderer, int z);
	~RenderPass2d() override;

	void addCommand(RenderCommand2d command);

	void render() override;
};

class Renderer2d : public Renderer {
private:
	SDL_GPUDevice* gpu_device;

	SDL_GPUGraphicsPipeline* pipeline;
	SDL_GPUBuffer* vertex_buffer;
	SDL_GPUBuffer* index_buffer;

	std::vector<RenderPass2d*> render_passes;
	std::vector<RenderPass2d*> render_pass_stack;

	Color color;

	const uint16_t MAX_VERTICES_PER_BATCH = 20000;
	const uint16_t MAX_INDICES_PER_BATCH = 30000;

private:
	friend class RenderPass2d;
	void renderPass(RenderPass2d* pass);

	void flush(uint32_t count_vertices, uint32_t count_indices);

public:
	Renderer2d(SDL_GPUDevice* gpu_device, SDL_GPUTextureFormat target_format);
	~Renderer2d() override;

	void beginPass(int z = 0);
	void endPassWindow(/*TODO Multi-window*/);
	void endPassTexture(/*TODO texture class*/);

	void setColor(Color color);

	void drawLine(Vec2f start, Vec2f end, float thickness);
	void fillRect(Vec2f pos, Vec2f size);
};

#endif // SLUAGH_RENDERER2D_H

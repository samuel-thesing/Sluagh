#ifndef SLUAGH_RENDERER2D_H
#define SLUAGH_RENDERER2D_H
#include "Renderer.h"
#include "SDL3/SDL_gpu.h"

#include <vector>

#include "sluagh/utils/Color.h"
#include "sluagh/utils/Matrix.h"
#include "sluagh/utils/Vector.h"

#include <stack>
#include <unordered_map>

enum class BorderAlignment {
	INSIDE = 0,
	CENTER = 1,
	OUTSIDE = 2
};

class Renderer2d;
struct RenderCommand2d {
	unsigned position_count;
	uint16_t index_count;
	Vec2f* positions;
	uint16_t* indices;
	Color color;
	// texture

	RenderCommand2d(uint32_t position_count, uint16_t index_count);
	RenderCommand2d(RenderCommand2d&& o) noexcept;
	~RenderCommand2d();
};

class RenderPass2d : public RenderPass {
private:
	Renderer2d* renderer;
	std::vector<RenderCommand2d> render_commands;

	int z_index = 0;

public:
	RenderPass2d(RenderTarget* target, Renderer2d* renderer, int z);
	~RenderPass2d() override = default;

	void addCommand(RenderCommand2d&& command);
	const std::vector<RenderCommand2d>& getRenderCommands() const { return render_commands; };

	int getZIndex() const { return z_index; }

	void render() override;
};

class SLUAGH_EXPORT Renderer2d : public Renderer {
private:
	SDL_GPUDevice* gpu_device;

	SDL_GPUGraphicsPipeline* pipeline;
	SDL_GPUBuffer* vertex_buffer;
	SDL_GPUBuffer* index_buffer;
	SDL_GPUTransferBuffer* transfer_buffer;

	std::vector<RenderPass2d*> render_pass_stack;

	Color color;

	const uint16_t MAX_VERTICES_PER_BATCH = 20000;
	const uint16_t MAX_INDICES_PER_BATCH = 30000;

private:
	friend class RenderPass2d;
	void renderPass(RenderPass2d* pass);

	void flush(RenderTarget* target, int z_index,
		const std::vector<RenderCommand2d>& commands, uint32_t offset,
		uint32_t command_count, uint32_t count_vertices, uint32_t count_indices);

public:
	Renderer2d(SDL_GPUDevice* gpu_device, SDL_GPUTextureFormat target_format);
	~Renderer2d() override;

	void beginPass(int z = 0);
	void beginPass(RenderTarget* target, int z = 0);
	void endPass();

	void setColor(Color color);

	void drawLine(Vec2f start, Vec2f end, float thickness = 2.f);
	void drawRect(Vec2f pos, Vec2f size, float thickness = 2.f,
		BorderAlignment border_alignment = BorderAlignment::CENTER);
	void fillRect(Vec2f pos, Vec2f size);

	void drawCircle(Vec2f pos, float radius, float thickness = 2.f,
		BorderAlignment border_alignment = BorderAlignment::CENTER);
	void fillCircle(Vec2f pos, float radius);
	void drawArc(Vec2f pos, float radius, float start_angle, float end_angle,
		float thickness = 2.f, BorderAlignment border_alignment = BorderAlignment::CENTER);
	void fillArc(Vec2f pos, float radius, float start_angle, float end_angle);

	void fillTriangle(Vec2f a, Vec2f b, Vec2f c);
	void drawBezier(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, float thickness = 2.f);
};

#endif // SLUAGH_RENDERER2D_H

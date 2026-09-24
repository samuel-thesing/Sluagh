#ifndef SLUAGH_RENDERCONTEXT_H
#define SLUAGH_RENDERCONTEXT_H
#include "Buffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "VertexBuffer.h"

class RenderContext {
private:
	SDL_GPUDevice* device;

public:
	RenderContext();
	~RenderContext();

	VertexBuffer createVertexBuffer(uint32_t stride, uint32_t capacity, const void *data = nullptr);
	IndexBuffer createIndexBuffer(IndexType index_type, uint32_t capacity, const void *data = nullptr);

	Texture createTexture2d(Vec2u size, SDL_GPUTextureFormat format);

	void destroy(Buffer& buffer);
	void destroy(Texture& texture);
};

#endif // SLUAGH_RENDERCONTEXT_H

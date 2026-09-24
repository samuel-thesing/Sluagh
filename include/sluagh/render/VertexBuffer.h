#ifndef SLUAGH_VERTEXBUFFER_H
#define SLUAGH_VERTEXBUFFER_H
#include "Buffer.h"

class VertexBuffer final : public Buffer {
	friend class RenderContext;
private:
	uint32_t stride;

private:
	VertexBuffer(Buffer buffer, uint32_t stride);

public:
	VertexBuffer() = default;

	uint32_t getStride() const { return stride; }
};

#endif // SLUAGH_VERTEXBUFFER_H

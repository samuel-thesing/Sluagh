#ifndef SLUAGH_BUFFER_H
#define SLUAGH_BUFFER_H
#include <SDL3/SDL_gpu.h>

class Buffer {
	friend class RenderContext;
private:
	SDL_GPUBuffer* buffer;
	uint32_t size;

private:
	Buffer(SDL_GPUBuffer* buffer, uint32_t size);

public:
	Buffer() = default;
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) noexcept;
	virtual ~Buffer();

	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) noexcept;

	uint32_t getSize() const { return size; }
};

#endif // SLUAGH_BUFFER_H

#ifndef SLUAGH_INDEXBUFFER_H
#define SLUAGH_INDEXBUFFER_H
#include "Buffer.h"

enum class IndexType {
	NONE = 0,
	UINT8 = 1,
	UINT16 = 2,
	UINT32 = 4
};

class IndexBuffer final : public Buffer {
	friend class RenderContext;
private:
	IndexType index_type{IndexType::NONE};

protected:
	IndexBuffer(Buffer buffer, IndexType index_type);

public:
	IndexBuffer() = default;

	IndexType getIndexType() const { return index_type; }
};

#endif // SLUAGH_INDEXBUFFER_H

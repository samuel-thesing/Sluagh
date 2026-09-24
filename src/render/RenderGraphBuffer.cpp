#include "sluagh/render/RenderGraphBuffer.h"

#include "sluagh/render/ResourceManager.h"
#include "sluagh/utils/Hash.h"

#include <format>

void RenderGraphBuffer::create(const Desc &desc, void *allocator) {
	buffer = static_cast<ResourceManager*>(allocator)->acquireBuffer(desc);
}

void RenderGraphBuffer::destroy(const Desc &desc, void *allocator) {
	static_cast<ResourceManager*>(allocator)->releaseBuffer(desc, std::move(buffer));
}

std::string RenderGraphBuffer::toString(const Desc &desc) {
	return std::format("buffer {} bytes", desc.size);
}

uint32_t std::hash<RenderGraphBuffer::Desc>::operator()(const RenderGraphBuffer::Desc &desc) const {
	uint32_t h = 0;
	hashCombine(h, desc.size);
	return h;
}
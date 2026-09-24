#ifndef SLUAGH_RESOURCEALLOCATOR_H
#define SLUAGH_RESOURCEALLOCATOR_H

#include "Buffer.h"
#include "RenderGraphBuffer.h"
#include "RenderGraphTexture.h"
#include "Texture.h"

#include <unordered_map>
#include <vector>



/**
 * Manages pools of resources/transient resources for the frame graph.
 */
class ResourceManager {
private:
	RenderContext& render_context;

	std::vector<std::unique_ptr<Texture>> textures;
	std::vector<std::unique_ptr<Buffer>> buffers;

	template <typename T>
	struct ResourceEntry {
		T resource;
		float life;
	};
	template <typename T>
	using ResourcePool = std::vector<ResourceEntry<T>>;

	std::unordered_map<RenderGraphTexture::Desc, ResourcePool<Texture*>, std::hash<RenderGraphTexture::Desc>, std::equal_to<>> texture_pools;
	std::unordered_map<RenderGraphBuffer::Desc, ResourcePool<Buffer*>> buffer_pools;

public:
	ResourceManager(RenderContext &render_context);
	~ResourceManager();

	Texture *acquireTexture(const RenderGraphTexture::Desc &desc);
	void releaseTexture(const RenderGraphTexture::Desc &desc, Texture *texture);
	Buffer *acquireBuffer(const RenderGraphBuffer::Desc &desc);
	void releaseBuffer(const RenderGraphBuffer::Desc &desc, Buffer *buffer);
};

#endif // SLUAGH_RESOURCEALLOCATOR_H

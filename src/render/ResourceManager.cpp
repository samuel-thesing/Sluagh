#include "sluagh/render/ResourceManager.h"

#include "sluagh/render/RenderContext.h"
#include "sluagh/utils/Hash.h"

ResourceManager::ResourceManager(RenderContext &render_context)
	: render_context(render_context) {}

ResourceManager::~ResourceManager() {

}

Texture* ResourceManager::acquireTexture(const RenderGraphTexture::Desc &desc) {
	const auto it = texture_pools.find(desc);

	if (it == texture_pools.end() || it->second.empty()) {
		auto texture = render_context.createTexture2d(desc.size, desc.format);
		textures.push_back(std::make_unique<Texture>(std::move(texture)));
		auto *ptr = textures.back().get();
		return ptr;
	} else {
		auto& pool = it->second;
		auto *texture = pool.back().resource;
		pool.pop_back();
		return texture;
	}
}

void ResourceManager::releaseTexture(const RenderGraphTexture::Desc &desc, Texture *texture) {
	auto it = texture_pools.find(desc);
	if (it == texture_pools.end()) {
		texture_pools[desc] = {};
		texture_pools[desc].push_back({texture, 0.f});
	} else {
		it->second.push_back({texture, 0.f});
	}
}

Buffer * ResourceManager::acquireBuffer(const RenderGraphBuffer::Desc &desc) {
	const auto it = buffer_pools.find(desc);

	if (it == buffer_pools.end() || it->second.empty()) {
		auto buffer = render_context.createVertexBuffer(desc.size);
		buffers.push_back(std::make_unique<Buffer>(std::move(buffer)));
		auto *ptr = buffers.back().get();
		return ptr;
	} else {
		auto& pool = it->second;
		auto *buffer = pool.back().resource;
		pool.pop_back();
		return buffer;
	}
}

void ResourceManager::releaseBuffer(const RenderGraphBuffer::Desc &desc, Buffer *buffer) {
	auto it = buffer_pools.find(desc);
	if (it == buffer_pools.end()) {
		buffer_pools[desc] = {};
		buffer_pools[desc].push_back({buffer, 0.f});
	} else {
		it->second.push_back({buffer, 0.f});
	}
}
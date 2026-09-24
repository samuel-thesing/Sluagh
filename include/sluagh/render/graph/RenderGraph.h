#ifndef SLUAGH_RENDERGRAPH_H
#define SLUAGH_RENDERGRAPH_H

#include "GraphTypeTraits.h"
#include "RenderGraphResource.h"
#include "PassNode.h"
#include "ResourceEntry.h"
#include "ResourceNode.h"

class RenderGraph {
	friend class RenderGraphBuilder;
	friend class RenderGraphPassResources;

private:
	std::vector<PassNode> pass_nodes;
	std::vector<ResourceNode> resource_nodes;
	std::vector<ResourceEntry> resource_registry;

private:
	PassNode& createPassNode(const std::string& name, std::unique_ptr<RenderGraphPassBase> &&base);

	template<has_Descriptor T>
	RenderGraphResource create(ResourceEntry::Type type,
		const std::string& name, const T::Desc &descriptor, T &&resource);

	ResourceNode& createResourceNode(const std::string& name, uint32_t resource_id,
		uint32_t version = ResourceEntry::initial_version);

	RenderGraphResource clone(RenderGraphResource id);

	const ResourceNode& getResourceNode(RenderGraphResource id) const;
	ResourceNode& getResourceNode(RenderGraphResource id);

	const ResourceEntry& getResourceEntry(RenderGraphResource id) const;
	ResourceEntry& getResourceEntry(RenderGraphResource id);

	const ResourceEntry& getResourceEntry(const ResourceNode& resource_node) const;
	ResourceEntry& getResourceEntry(const ResourceNode& resource_node);


public:
	RenderGraph() = default;

	struct NoData {};

	template<typename Data = NoData, typename Setup, typename Execute>
	const Data& addCallbackPass(const std::string& name, Setup &&setup, Execute &&execute);

	template<has_Descriptor T>
	const T::Desc& getDescriptor(RenderGraphResource id) const;

	template<has_Descriptor T>
	RenderGraphResource import(const std::string& name, const T::Desc &descriptor, T &&resource);

	void compile();
	void execute(void *context = nullptr, void *allocator = nullptr);

	bool isValid(RenderGraphResource id) const;
};

#include "RenderGraph.inl"

#endif // SLUAGH_RENDERGRAPH_H

#ifndef SLUAGH_RENDERGRAPH_INL
#define SLUAGH_RENDERGRAPH_INL

#include <functional>

#include "RenderGraph.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphPass.h"

template<typename Data, typename Setup, typename Execute>
const Data& RenderGraph::addCallbackPass(const std::string &name, Setup &&setup, Execute &&execute) {
	auto *pass = new RenderGraphPass<Data, Execute>(std::forward<Execute>(execute));
	auto &pass_node = createPassNode(name, std::unique_ptr<RenderGraphPass<Data, Execute>>(pass));
	RenderGraphBuilder builder{*this, pass_node};
	std::invoke(setup, builder, pass->data);
	return pass->data;
}

template<has_Descriptor T>
const T::Desc& RenderGraph::getDescriptor(RenderGraphResource id) const {
	return getResourceEntry(id).getDescriptor<T>();
}

template<has_Descriptor T>
RenderGraphResource RenderGraph::import(const std::string& name, const typename T::Desc &descriptor, T &&resource) {
	return create<T>(ResourceEntry::Type::Imported, name, descriptor, std::forward<T>(resource));
}

template<has_Descriptor T>
RenderGraphResource RenderGraph::create(ResourceEntry::Type type, const std::string& name,
	const typename T::Desc &descriptor, T &&resource) {
	const auto resource_id = static_cast<uint32_t>(resource_registry.size());
	resource_registry.emplace_back(ResourceEntry{
		type, resource_id, descriptor, std::forward<T>(resource)
	});
	return createResourceNode(name, resource_id).getId();
}


#endif // SLUAGH_RENDERGRAPH_INL

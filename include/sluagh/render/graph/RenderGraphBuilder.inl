#ifndef SLUAGH_RENDERGRAPHBUILDER_INL
#define SLUAGH_RENDERGRAPHBUILDER_INL

#include "RenderGraphBuilder.h"
#include "ResourceEntry.h"
#include "RenderGraph.h"
#include "PassNode.h"

template<has_Descriptor T>
RenderGraphResource RenderGraphBuilder::create(const std::string& name, const typename T::Desc &descriptor) {
	const auto id = frame_graph.create(ResourceEntry::Type::Transient, name, descriptor, T{});
	return pass_node.creates.emplace_back(id);
}

#endif // SLUAGH_RENDERGRAPHBUILDER_INL

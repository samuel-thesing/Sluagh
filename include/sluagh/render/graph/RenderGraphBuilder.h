#ifndef SLUAGH_RENDERGRAPHBUILDER_H
#define SLUAGH_RENDERGRAPHBUILDER_H

#include "GraphTypeTraits.h"
#include "RenderGraphResource.h"
#include "PassNode.h"

class RenderGraph;

class RenderGraphBuilder final {
private:
	RenderGraph &frame_graph;
	PassNode &pass_node;

private:
	friend class RenderGraph;

	RenderGraphBuilder(RenderGraph &frame_graph, PassNode &pass_node);

public:
	template<has_Descriptor T>
	RenderGraphResource create(const std::string& name, const T::Desc &descriptor);
	RenderGraphResource read(RenderGraphResource id);
	RenderGraphResource write(RenderGraphResource id);

	RenderGraphBuilder& setSideEffect();
};

#include "RenderGraphBuilder.inl"


#endif // SLUAGH_RENDERGRAPHBUILDER_H

#ifndef SLUAGH_RENDERGRAPHPASSRESOURCES_H
#define SLUAGH_RENDERGRAPHPASSRESOURCES_H

#include "RenderGraphResource.h"
#include "GraphTypeTraits.h"

class RenderGraph;
class PassNode;

class RenderGraphPassResources {
	friend class RenderGraph;

private:
	RenderGraph &frame_graph;

private:
	RenderGraphPassResources(RenderGraph &frame_graph)
		: frame_graph(frame_graph) {}

public:
	template<has_Descriptor T>
	T& get(RenderGraphResource id);

	template<has_Descriptor T>
	const T::Desc& getDescriptor(RenderGraphResource id) const;
};

#include "RenderGraphPassResources.inl"

#endif // SLUAGH_RENDERGRAPHPASSRESOURCES_H

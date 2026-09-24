#ifndef SLUAGH_RENDERGRAPHPASSRESOURCES_INL
#define SLUAGH_RENDERGRAPHPASSRESOURCES_INL

#include "RenderGraph.h"
#include "RenderGraphPassResources.h"

template<has_Descriptor T>
T& RenderGraphPassResources::get(RenderGraphResource id) {
	return frame_graph.getResourceEntry(id).get<T>();
}

template<has_Descriptor T>
const T::Desc& RenderGraphPassResources::getDescriptor(RenderGraphResource id) const {
	return frame_graph.getDescriptor<T>(id);
}


#endif // SLUAGH_RENDERGRAPHPASSRESOURCES_INL

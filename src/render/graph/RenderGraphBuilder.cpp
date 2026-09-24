#include "sluagh/render/graph/RenderGraphBuilder.h"

#include "sluagh/render/graph/PassNode.h"

RenderGraphBuilder::RenderGraphBuilder(RenderGraph &frame_graph, PassNode &pass_node)
	: frame_graph{frame_graph}, pass_node{pass_node} {}

RenderGraphResource RenderGraphBuilder::read(RenderGraphResource id) {
	return pass_node.read(id);
}

RenderGraphResource RenderGraphBuilder::write(RenderGraphResource id) {
	if (frame_graph.getResourceEntry(id).isImported()) {
		setSideEffect();
	}

	if (pass_node.doesCreate(id)) {
		return pass_node.write(id);
	}

	pass_node.read(id);
	return pass_node.write(frame_graph.clone(id));
}

RenderGraphBuilder & RenderGraphBuilder::setSideEffect() {
	pass_node.side_effect = true;
	return *this;
}
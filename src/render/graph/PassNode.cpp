#include "sluagh/render/graph/PassNode.h"

PassNode::PassNode(const std::string& name, uint32_t node_id, std::unique_ptr<RenderGraphPassBase> &&executor)
	: GraphNode{name, node_id}, executor{std::move(executor)} {}

bool hasId(const std::vector<RenderGraphResource> &v, RenderGraphResource id) {
	return std::ranges::find(v, id) != v.cend();
}

RenderGraphResource PassNode::read(RenderGraphResource id) {
	return hasId(reads, id) ? id : reads.emplace_back(id);
}
RenderGraphResource PassNode::write(RenderGraphResource id) {
	return hasId(writes, id) ? id : writes.emplace_back(id);
}

bool PassNode::doesCreate(RenderGraphResource id) {
	return hasId(creates, id);
}

bool PassNode::doesRead(RenderGraphResource id) {
	return hasId(reads, id);
}

bool PassNode::doesWrite(RenderGraphResource id) {
	return hasId(writes, id);
}
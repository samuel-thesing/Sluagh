#ifndef SLUAGH_PASSNODE_H
#define SLUAGH_PASSNODE_H

#include <memory>
#include <vector>

#include "RenderGraphPass.h"
#include "RenderGraphResource.h"
#include "GraphNode.h"

class PassNode final : public GraphNode {
	friend class RenderGraph;
	friend class RenderGraphBuilder;

private:
	std::unique_ptr<RenderGraphPassBase> executor;

	std::vector<RenderGraphResource> creates;
	std::vector<RenderGraphResource> reads;
	std::vector<RenderGraphResource> writes;

	bool side_effect{false};

private:
	RenderGraphResource read(RenderGraphResource id);
	RenderGraphResource write(RenderGraphResource id);

	PassNode(const std::string& name, uint32_t node_id, std::unique_ptr<RenderGraphPassBase> &&executor);

public:
	bool doesCreate(RenderGraphResource id);
	bool doesRead(RenderGraphResource id);
	bool doesWrite(RenderGraphResource id);

	bool hasSideEffect() const { return side_effect; };
	bool canExecute() const {
		return getRefCount() > 0 || side_effect;
	}
};

#endif // SLUAGH_PASSNODE_H

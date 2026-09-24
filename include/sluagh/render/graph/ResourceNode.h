#ifndef SLUAGH_RESOURCENODE_H
#define SLUAGH_RESOURCENODE_H

#include "GraphNode.h"

class ResourceNode final : public GraphNode {
	friend class RenderGraph;

private:
	const uint32_t resource_id;
	const uint32_t version;

	PassNode *producer;

private:
	ResourceNode(const std::string& name, uint32_t node_id, uint32_t resource_id, uint32_t version)
		: GraphNode{name, node_id}, resource_id{resource_id}, version{version} {}

public:
	ResourceNode(const ResourceNode&) = delete;
	ResourceNode(ResourceNode&&) = default;

	uint32_t getResourceId() const { return resource_id; }
	uint32_t getVersion() const { return version; }
};

#endif // SLUAGH_RESOURCENODE_H

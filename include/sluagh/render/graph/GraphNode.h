#ifndef SLUAGH_GRAPHNODE_H
#define SLUAGH_GRAPHNODE_H

#include <cstdint>
#include <string>

class GraphNode {
	friend class RenderGraph;

private:
	std::string name;

	uint32_t id;
	int32_t ref_count{0};

protected:
	GraphNode(const std::string& name, uint32_t id)
		: name{name}, id{id} {}

public:
	virtual ~GraphNode() = default;

	std::string getName() const { return name; }
	uint32_t getId() const { return id; }
	int32_t getRefCount() const { return ref_count; }

};

#endif // SLUAGH_GRAPHNODE_H

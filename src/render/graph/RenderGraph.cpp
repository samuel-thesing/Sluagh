#include "sluagh/render/graph/RenderGraph.h"

#include <stack>

#include "sluagh/render/graph/RenderGraphPassResources.h"

bool RenderGraph::isValid(RenderGraphResource id) const {
    const auto &node = getResourceNode(id);
    return node.getVersion() == getResourceEntry(id).getVersion();
}

void RenderGraph::compile() {
    for (auto &pass : pass_nodes) {
        pass.ref_count = static_cast<int32_t>(pass.writes.size());
        for (const auto id : pass.reads) {
            auto &consumed = resource_nodes[id];
            consumed.ref_count++;
        }
        for (const auto id : pass.writes) {
            auto &written = resource_nodes[id];
            written.producer = &pass;
        }
    }

    std::stack<ResourceNode*> unreferenced_resources;
    for (auto &node : resource_nodes) {
        if (node.getRefCount() == 0) unreferenced_resources.push(&node);
    }
    while (!unreferenced_resources.empty()) {
        auto *unreferenced_resource = unreferenced_resources.top();
        unreferenced_resources.pop();
        PassNode *producer{unreferenced_resource->producer};
        if (producer == nullptr || producer->hasSideEffect()) continue;

        if (--producer->ref_count == 0) {
            for (const auto id : producer->reads) {
                auto &node = resource_nodes[id];
                if (--node.ref_count == 0) unreferenced_resources.push(&node);
            }
        }
    }

    for (auto &pass : pass_nodes) {
        if (pass.canExecute()) continue;

        for (const auto id : pass.creates) {
            getResourceEntry(id).producer = &pass;
        }
        for (const auto id : pass.writes) {
            getResourceEntry(id).last = &pass;
        }
        for (const auto id : pass.reads) {
            getResourceEntry(id).last = &pass;
        }
    }
}

void RenderGraph::execute(void *context, void *allocator) {
    for (const auto &pass : pass_nodes) {
        if (!pass.canExecute()) continue;

        for (const auto id : pass.creates) {
            getResourceEntry(id).create(allocator);
        }

        for (const auto id : pass.reads) {
            getResourceEntry(id).preRead(context);
        }

        for (const auto id : pass.writes) {
            getResourceEntry(id).preWrite(context);
        }

        RenderGraphPassResources resources{*this};
        std::invoke(*(pass.executor), resources, context);

        for (auto &entry : resource_registry) {
            if (entry.last == &pass && entry.isTransient()) {
                entry.destroy(allocator);
            }
        }
    }
}

PassNode& RenderGraph::createPassNode(const std::string& name, std::unique_ptr<RenderGraphPassBase> &&base) {
    const auto id = static_cast<uint32_t>(pass_nodes.size());
    return pass_nodes.emplace_back(PassNode{name, id, std::move(base)});
}

ResourceNode& RenderGraph::createResourceNode(const std::string& name, uint32_t resource_id, uint32_t version) {
    const auto id = static_cast<uint32_t>(resource_nodes.size());
    return resource_nodes.emplace_back(ResourceNode{name, id, resource_id, version});
}

RenderGraphResource RenderGraph::clone(RenderGraphResource id) {
    const auto &node = getResourceNode(id);
    auto &entry = getResourceEntry(id);
    entry.version++;

    const auto& clone = createResourceNode(node.getName(), node.getResourceId(), entry.getVersion());
    return clone.getId();
}

const ResourceNode& RenderGraph::getResourceNode(RenderGraphResource id) const {
    return resource_nodes[id];
}

ResourceNode& RenderGraph::getResourceNode(RenderGraphResource id) {
    return const_cast<ResourceNode&>(
        const_cast<const RenderGraph*>(this)->getResourceNode(id)
    );
}

const ResourceEntry& RenderGraph::getResourceEntry(RenderGraphResource id) const {
    return getResourceEntry(getResourceNode(id));
}

ResourceEntry& RenderGraph::getResourceEntry(RenderGraphResource id) {
    return const_cast<ResourceEntry&>(
        const_cast<const RenderGraph*>(this)->getResourceEntry(id)
    );
}

const ResourceEntry& RenderGraph::getResourceEntry(const ResourceNode& node) const {
    return resource_registry[node.getResourceId()];
}

ResourceEntry& RenderGraph::getResourceEntry(const ResourceNode& node) {
    return const_cast<ResourceEntry&>(
        const_cast<const RenderGraph*>(this)->getResourceEntry(node)
    );
}
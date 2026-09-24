#ifndef SLUAGH_RENDERGRAPHPASS_H
#define SLUAGH_RENDERGRAPHPASS_H

class RenderGraphPassResources;

class RenderGraphPassBase {
public:
	RenderGraphPassBase() = default;
	RenderGraphPassBase(const RenderGraphPassBase&) = delete;
	RenderGraphPassBase(RenderGraphPassBase&&) = delete;
	virtual ~RenderGraphPassBase() = default;

	virtual void operator()(RenderGraphPassResources &resources, void *context) = 0;
};

template<typename Data, typename Execute>
class RenderGraphPass final : public RenderGraphPassBase {
	friend class RenderGraph;

private:
	Execute execute_fn;
	Data data{};

public:
	explicit RenderGraphPass(Execute&& execute_fn)
	: execute_fn{execute_fn} {}

	void operator()(RenderGraphPassResources &resources, void *context) override {
		execute_fn(data, resources, context);
	}
};

#endif // SLUAGH_RENDERGRAPHPASS_H

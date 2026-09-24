#ifndef SLUAGH_RENDERGRAPHBUFFER_H
#define SLUAGH_RENDERGRAPHBUFFER_H
#include "Buffer.h"


#include <string>



class RenderGraphBuffer {
public:
	struct Desc {
		uint32_t size;

		bool operator==(const Desc&) const = default;
	};

private:
    Buffer *buffer{nullptr};

public:
	void create(const Desc& desc, void *allocator);
	void destroy(const Desc& desc, void *allocator);

	static std::string toString(const Desc& desc);
    
};

template<>
struct std::hash<RenderGraphBuffer::Desc> {
	uint32_t operator()(const RenderGraphBuffer::Desc &desc) const;
};



#endif //SLUAGH_RENDERGRAPHBUFFER_H

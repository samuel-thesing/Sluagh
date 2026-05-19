#include "sluagh/render/Renderer.h"

void RenderPass::setRenderTarget(RenderTargetType type, Vec2i size) {
	target.type = type;
	target.size = size;
}

void Renderer::insertRenderPass(RenderPass *pass) {
	auto it = render_passes.begin();
	for (; it != render_passes.end(); ++it) {
		if ((*it)->getZIndex() < pass->getZIndex()) {
			render_passes.insert(it, pass);
			break;
		}
	}
	if (it == render_passes.cend()) {
		render_passes.push_back(pass);
	}
}

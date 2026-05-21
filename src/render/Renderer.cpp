#include "sluagh/render/Renderer.h"

void RenderPass::setRenderTarget(RenderTargetType type, Vec2i size, SDL_GPUTexture *texture) {
	target.type = type;
	target.size = size;
	target.texture = texture;
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

void Renderer::setCommandBuffer(SDL_GPUCommandBuffer *current_command_buffer) {
	command_buffer = current_command_buffer;
}

void Renderer::renderAll() {
	for (auto* pass : render_passes) {
		pass->render();
	}

	for (auto *pass : render_passes) {
		delete pass;
	}
	render_passes.clear();
}

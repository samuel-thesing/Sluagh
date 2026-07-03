#include "sluagh/render/Renderer.h"

#include "sluagh/core/Application.h"

void Renderer::addRenderPass(RenderPass *pass) {
	render_passes.push_back(pass);
}

void Renderer::setCommandBuffer(SDL_GPUCommandBuffer *current_command_buffer) {
	command_buffer = current_command_buffer;
}

void Renderer::renderAll() { // todo
	for (auto* pass : render_passes) {
		pass->render();
	}

	for (auto *pass : render_passes) {
		delete pass;
	}
	render_passes.clear();
}

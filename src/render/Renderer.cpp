#include "sluagh/render/Renderer.h"

#include "sluagh/core/Application.h"

void Renderer::setRenderManager(RenderManager *manager) {
	render_manager = manager;
}

void Renderer::setCommandBuffer(SDL_GPUCommandBuffer *current_command_buffer) {
	command_buffer = current_command_buffer;
}

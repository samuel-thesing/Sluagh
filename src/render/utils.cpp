#include "sluagh/render/utils.h"

#include "sluagh/utils/Logger.h"

#include <SDL3/SDL_gpu.h>

SDL_GPUShader *loadShader(SDL_GPUDevice *gpu_device, const char *shader_path,
	uint32_t sampler_count, uint32_t uniform_buffer_count,
	uint32_t storage_buffer_count, uint32_t storage_texture_count) {
	SDL_GPUShaderStage stage;
	if (SDL_strstr(shader_path, ".vert")) {
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	} else if (SDL_strstr(shader_path, ".frag")) {
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	} else {
		Logger::error("Can't extract shader stage from filename '{}'", shader_path);
		return nullptr;
	}

	std::string full_path = std::format("{}.spv", shader_path);
	size_t code_length;
	void* code = SDL_LoadFile(full_path.c_str(), &code_length);
	if (code == nullptr) {
		Logger::error("Failed to read shader-file '{}'", full_path);
		return nullptr;
	}

	SDL_GPUShaderCreateInfo shader_info = {
		.code_size = code_length,
		.code = static_cast<uint8_t*>(code),
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV,
		.stage = stage,
		.num_samplers = sampler_count,
		.num_storage_textures = storage_texture_count,
		.num_storage_buffers = storage_buffer_count,
		.num_uniform_buffers = uniform_buffer_count,
	};
	SDL_GPUShader* shader = SDL_CreateGPUShader(gpu_device, &shader_info);
	SDL_free(code);
	if (shader == nullptr) {
		Logger::error("Failed to create shader '{}'", shader_path);
		return nullptr;
	}

	return shader;
}
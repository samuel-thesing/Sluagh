#ifndef SLUAGH_UTILS_H
#define SLUAGH_UTILS_H

#include <SDL3/SDL.h>

SDL_GPUShader *loadShader(SDL_GPUDevice *gpu_device, const char *shader_path,
	uint32_t sampler_count, uint32_t uniform_buffer_count,
	uint32_t storage_buffer_count, uint32_t storage_texture_count);

#endif // SLUAGH_UTILS_H

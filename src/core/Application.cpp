#include "sluagh/core/Application.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

Application *Application::instance = nullptr;

Application *Application::create(Config config) {
	instance = new Application(config);
	return instance;
}

void Application::destroy() {
	delete instance;
	instance = nullptr;
}

struct PositionColorVertex {
	float x, y, z;
	float r, g, b, a;
};

Application::Application(const Config &config) {
	window = SDL_CreateWindow(config.title.c_str(), 800, 600, 0);
	if (window == nullptr) {
		Logger::critical("Failed to create window");
		throw std::runtime_error("Failed to create window");
	}

	gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL, true, nullptr);
	if (gpu_device == nullptr) {
		Logger::critical("Failed to create GPU device: {}", SDL_GetError());
		throw std::runtime_error("Failed to create GPU device");
	}

	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
		Logger::critical("Failed to claim window for GPU device");
		throw std::runtime_error("Failed to claim window for GPU device");
	}

	Logger::trace("Finished initializing application");

	SDL_GPUShader *vertex_shader = loadShader(gpu_device,
		"./rsc/shader/basic.vert", 0, 0,
		0, 0);
	SDL_GPUShader *fragment_shader = loadShader(gpu_device,
		"./rsc/shader/basic.frag", 0, 0,
		0, 0);

	SDL_GPUColorTargetDescription color_target = {
		.format = SDL_GetGPUSwapchainTextureFormat(gpu_device, window),
		.blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.enable_blend = true,
		}
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.slot = 0,
		.pitch = sizeof(PositionColorVertex),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		.instance_step_rate = 0
	};

	SDL_GPUVertexAttribute vertex_attribs[] = {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0
		},
		{
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
			.offset = sizeof(float) * 3
		}
	};

	SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.vertex_input_state ={
				.vertex_buffer_descriptions = &vertex_buffer_description,
				.num_vertex_buffers = 1,
				.vertex_attributes = vertex_attribs,
				.num_vertex_attributes = 2,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = &color_target,
			.num_color_targets = 1
		}
	};

	gpu_pipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipeline_info);
	if (gpu_pipeline == nullptr) {
		Logger::critical("Failed to create GPU pipeline");
		throw std::runtime_error("Failed to create GPU pipeline");
	}

	SDL_ReleaseGPUShader(gpu_device, vertex_shader);
	SDL_ReleaseGPUShader(gpu_device, fragment_shader);

	SDL_GPUBufferCreateInfo vertex_buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = sizeof(PositionColorVertex) * 4
	};
	vertex_buffer = SDL_CreateGPUBuffer(gpu_device, &vertex_buffer_create_info);

	SDL_GPUBufferCreateInfo index_buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_INDEX,
		.size = sizeof(uint16_t) * 6
	};
	index_buffer = SDL_CreateGPUBuffer(gpu_device, &index_buffer_create_info);

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = sizeof(PositionColorVertex) * 4 + sizeof(uint16_t) * 6
	};
	SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(
		gpu_device, &transfer_buffer_create_info);
	auto* transfer_data = static_cast<PositionColorVertex*>(SDL_MapGPUTransferBuffer(gpu_device, transfer_buffer, false));

	transfer_data[0] = {0.5, 0.5, 0, 1, 1, 1, 1};
	transfer_data[1] = {0.5, -0.5, 0, 0, 1, 0, 1};
	transfer_data[2] = {-0.5, -0.5, 0, 1, 0, 0, 1};
	transfer_data[3] = {-0.5, 0.5, 0, 0, 0, 1, 1};

	auto* index_data = reinterpret_cast<uint16_t*>(&transfer_data[4]);
	index_data[0] = 0;
	index_data[1] = 1;
	index_data[2] = 3;
	index_data[3] = 1;
	index_data[4] = 2;
	index_data[5] = 3;

	SDL_UnmapGPUTransferBuffer(gpu_device, transfer_buffer);

	SDL_GPUCommandBuffer* upload_cmd_buf = SDL_AcquireGPUCommandBuffer(gpu_device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buf);

	SDL_GPUTransferBufferLocation vertex_transfer_loc = {
		.transfer_buffer = transfer_buffer,
		.offset = 0
	};
	SDL_GPUBufferRegion vertex_buffer_region = {
		.buffer = vertex_buffer,
		.offset = 0,
		.size = sizeof(PositionColorVertex) * 4
	};
	SDL_UploadToGPUBuffer(copy_pass, &vertex_transfer_loc, &vertex_buffer_region, false);

	SDL_GPUTransferBufferLocation index_transfer_loc = {
		.transfer_buffer = transfer_buffer,
		.offset = sizeof(PositionColorVertex) * 4
	};
	SDL_GPUBufferRegion index_buffer_region = {
		.buffer = index_buffer,
		.offset = 0,
		.size = sizeof(uint16_t) * 6
	};
	SDL_UploadToGPUBuffer(copy_pass, &index_transfer_loc, &index_buffer_region, false);

	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
	SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer);
}

Application::~Application() {
	SDL_ReleaseGPUGraphicsPipeline(gpu_device, gpu_pipeline);
	SDL_ReleaseGPUBuffer(gpu_device, vertex_buffer);

	SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
	SDL_DestroyGPUDevice(gpu_device);
	SDL_DestroyWindow(window);
}

void Application::onEvent(SDL_Event *event) {
	if (event->type == SDL_EVENT_QUIT) {
		quit = true;
		return;
	}

	if (event->type == SDL_EVENT_KEY_UP) {
		if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
			quit = true;
		}
	}
}

void Application::onRender() {
	SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(gpu_device);
	if (cmd_buf == nullptr) {
		Logger::critical("Failed to acquire GPU command buffer");
		throw std::runtime_error("Failed to acquire GPU command buffer");
	}

	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(
			cmd_buf, window, &swapchain_texture, nullptr, nullptr)) {
		Logger::critical("Failed to wait for swapchain texture");
		throw std::runtime_error("Failed to wait for swapchain texture");
	}

	if (swapchain_texture != nullptr) {
		SDL_GPUColorTargetInfo color_target = {0};
		color_target.mip_level = 0;
		color_target.texture = swapchain_texture;
		color_target.clear_color = SDL_FColor{0.2f, 0.6f, 0.8f, 1.0f};
		color_target.load_op = SDL_GPU_LOADOP_CLEAR;
		color_target.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass *render_pass =
			SDL_BeginGPURenderPass(cmd_buf, &color_target, 1, nullptr);

		SDL_BindGPUGraphicsPipeline(render_pass, gpu_pipeline);
		SDL_GPUBufferBinding vertex_binding = {
			.buffer = vertex_buffer,
			.offset = 0
		};
		SDL_BindGPUVertexBuffers(render_pass, 0, &vertex_binding, 1);
		SDL_GPUBufferBinding index_binding = {
			.buffer = index_buffer,
			.offset = 0
		};
		SDL_BindGPUIndexBuffer(render_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
		SDL_DrawGPUIndexedPrimitives(render_pass, 6, 1, 0, 0, 0);

		SDL_EndGPURenderPass(render_pass);
	}
	SDL_SubmitGPUCommandBuffer(cmd_buf);
}

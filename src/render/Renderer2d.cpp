#include "sluagh/render/Renderer2d.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

struct PositionColorVertex {
	Vec2f pos;
	Vec4f color;
};

RenderCommand2d::RenderCommand2d(uint32_t position_count, uint32_t index_count)
	: position_count(position_count), index_count(index_count), color(Colors::MAGENTA) {
	positions = new Vec2f[position_count];
	indices = new uint16_t[index_count];
}

void RenderPass2d::addCommand(RenderCommand2d command) {
	render_commands.emplace_back(std::move(command));
}

void RenderPass2d::render() {
	renderer->renderPass(this);
}

void Renderer2d::renderPass(RenderPass2d *pass) {
	// actual render logic:
	// convert RenderCommands to buffers and such
	// transfer buffers
	// render
}

void Renderer2d::flush(RenderPassuint32_t count_vertices, uint32_t count_indices) {
	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = static_cast<uint32_t>(sizeof(PositionColorVertex) * count_vertices
			+ sizeof(uint16_t) * count_indices)
	};
	SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(
		gpu_device, &transfer_buffer_create_info);
	auto* transfer_data = static_cast<PositionColorVertex*>(SDL_MapGPUTransferBuffer(gpu_device, transfer_buffer, false));
	auto* index_data = reinterpret_cast<uint16_t*>(&transfer_data[count_vertices]);

	for (const auto& command : render_)

	transfer_data[0] = {0.5, 0.5, 0, 1, 1, 1, 1};
	transfer_data[1] = {0.5, -0.5, 0, 0, 1, 0, 1};
	transfer_data[2] = {-0.5, -0.5, 0, 1, 0, 0, 1};
	transfer_data[3] = {-0.5, 0.5, 0, 0, 0, 1, 1};

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
}

Renderer2d::Renderer2d(SDL_GPUDevice* gpu_device, SDL_GPUTextureFormat target_format)
	: gpu_device(gpu_device) {
	SDL_GPUShader *vertex_shader = loadShader(gpu_device,
		"./rsc/shader/basic.vert", 0, 0,
		0, 0);
	SDL_GPUShader *fragment_shader = loadShader(gpu_device,
		"./rsc/shader/basic.frag", 0, 0,
		0, 0);

	SDL_GPUColorTargetDescription color_target = {
		.format = target_format,
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
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = 0
		},
		{
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
			.offset = sizeof(float) * 2
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

	pipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipeline_info);
	if (pipeline == nullptr) {
		Logger::critical("Failed to create GPU pipeline");
		throw std::runtime_error("Failed to create GPU pipeline");
	}

	SDL_ReleaseGPUShader(gpu_device, vertex_shader);
	SDL_ReleaseGPUShader(gpu_device, fragment_shader);

	SDL_GPUBufferCreateInfo vertex_buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = static_cast<uint32_t>(sizeof(PositionColorVertex) * MAX_VERTICES_PER_BATCH)
	};
	vertex_buffer = SDL_CreateGPUBuffer(gpu_device, &vertex_buffer_create_info);

	SDL_GPUBufferCreateInfo index_buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_INDEX,
		.size = static_cast<uint32_t>(sizeof(uint16_t) * MAX_INDICES_PER_BATCH)
	};
	index_buffer = SDL_CreateGPUBuffer(gpu_device, &index_buffer_create_info);
}

Renderer2d::~Renderer2d() {}

void Renderer2d::beginPass(int z) {
	render_pass_stack.push_back(new RenderPass2d(this, z));
}
void Renderer2d::endPassWindow() {
	auto* pass = render_pass_stack.back();
	render_pass_stack.pop_back();

	// TODO fetch window size
	pass->setRenderTarget(RenderTargetType::Window, Vec2i(800, 600));
	insertRenderPass(pass);
}

void Renderer2d::setColor(Color new_color) {
	color = new_color;
}

void Renderer2d::drawLine(Vec2f start, Vec2f end, float thickness) {
	auto command = RenderCommand2d(4, 6);
	command.color = color;

	auto dir = (end - start).normalized();
	auto normal = dir.cross();

	command.positions[0] = start + normal * thickness / 2.f;
	command.positions[1] = end + normal * thickness / 2.f;
	command.positions[2] = end - normal * thickness / 2.f;
	command.positions[3] = start - normal * thickness / 2.f;

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 3;
	command.indices[3] = 1;
	command.indices[4] = 2;
	command.indices[5] = 3;

	render_pass_stack.back()->addCommand(command);
}

void Renderer2d::fillRect(Vec2f pos, Vec2f size) {
	auto command = RenderCommand2d(4, 6);
	command.color = color;

	command.positions[0] = pos;
	command.positions[1] = pos + Vec2f{size.x(), 0.f};
	command.positions[2] = pos + size;
	command.positions[3] = pos + Vec2f{0.f, size.y()};

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 3;
	command.indices[3] = 1;
	command.indices[4] = 2;
	command.indices[5] = 3;
	render_passes.back()->addCommand(command);
}

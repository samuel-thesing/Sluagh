#include "sluagh/render/Renderer2d.h"

#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

struct PositionColorVertex {
	Vec2f pos;
	Vec4f color;
};

RenderCommand2d::RenderCommand2d(uint32_t position_count, uint16_t index_count)
	: position_count(position_count), index_count(index_count), color(Colors::MAGENTA) {
	positions = new Vec2f[position_count];
	indices = new uint16_t[index_count];
}
RenderCommand2d::RenderCommand2d(RenderCommand2d &&o) noexcept {
	positions = o.positions;
	indices = o.indices;
	o.positions = nullptr;
	o.indices = nullptr;

	color = o.color;
	position_count = o.position_count;
	index_count = o.index_count;
}

RenderCommand2d::~RenderCommand2d() {
	delete[] positions;
	delete[] indices;
}

RenderPass2d::RenderPass2d(Renderer2d *renderer, int z)
	: renderer{renderer} {
	z_index = z;
}

void RenderPass2d::addCommand(RenderCommand2d&& command) {
	render_commands.push_back(std::move(command));
}

void RenderPass2d::render() {
	renderer->renderPass(this);
}

void Renderer2d::renderPass(RenderPass2d *pass) {
	const auto& target = pass->getRenderTarget();
	const auto& commands = pass->getRenderCommands();
	uint32_t count_vertices = 0;
	uint32_t count_indices = 0;
	uint32_t last_flush_index = 0;
	for (uint32_t i = 0; i < commands.size(); i++) {
		if (count_vertices + commands[i].position_count >= MAX_VERTICES_PER_BATCH
			|| count_indices + commands[i].index_count >= MAX_INDICES_PER_BATCH) {

			flush(
				target.texture,
				commands,
				last_flush_index,
				i - last_flush_index,
				count_vertices - commands[i].position_count,
				count_indices - commands[i].index_count
			);
			last_flush_index = i;
			count_vertices = 0;
			count_indices = 0;
		}

		count_vertices += commands[i].position_count;
		count_indices += commands[i].index_count;
	}

	if (count_vertices != 0 || count_indices != 0) {
		flush(
			target.texture,
			commands,
			last_flush_index,
			static_cast<uint32_t>(commands.size() - last_flush_index),
			count_vertices,
			count_indices
		);
	}
}

void Renderer2d::flush(SDL_GPUTexture* target_texture,
		const std::vector<RenderCommand2d>& commands,
		uint32_t offset, uint32_t command_count,
		uint32_t count_vertices, uint32_t count_indices) {
	auto* transfer_data = static_cast<PositionColorVertex*>(SDL_MapGPUTransferBuffer(gpu_device, transfer_buffer, true));
	auto* index_data = reinterpret_cast<uint16_t*>(&transfer_data[count_vertices]);

	uint32_t vertex_i = 0;
	uint32_t index_i = 0;
	uint16_t index_offset = 0;
	for (uint32_t i = offset; i < offset + command_count; i++) {
		const auto& command = commands[i];
		for (uint32_t j = 0; j < command.position_count; j++, vertex_i++) {
			transfer_data[vertex_i].pos = command.positions[j];
			transfer_data[vertex_i].color = command.color.toVec4();
		}

		for (uint32_t j = 0; j < command.index_count; j++, index_i++) {
			index_data[index_i] = index_offset + command.indices[j];
		}
		index_offset += command.index_count;
	}

	SDL_UnmapGPUTransferBuffer(gpu_device, transfer_buffer);

	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

	SDL_GPUTransferBufferLocation vertex_transfer_loc = {
		.transfer_buffer = transfer_buffer,
		.offset = 0
	};
	SDL_GPUBufferRegion vertex_buffer_region = {
		.buffer = vertex_buffer,
		.offset = 0,
		.size = static_cast<uint32_t>(sizeof(PositionColorVertex) * count_vertices)
	};
	SDL_UploadToGPUBuffer(copy_pass, &vertex_transfer_loc, &vertex_buffer_region, false);

	SDL_GPUTransferBufferLocation index_transfer_loc = {
		.transfer_buffer = transfer_buffer,
		.offset = static_cast<uint32_t>(sizeof(PositionColorVertex) * count_vertices)
	};
	SDL_GPUBufferRegion index_buffer_region = {
		.buffer = index_buffer,
		.offset = 0,
		.size = static_cast<uint32_t>(sizeof(uint16_t) * count_indices)
	};
	SDL_UploadToGPUBuffer(copy_pass, &index_transfer_loc, &index_buffer_region, true);

	SDL_EndGPUCopyPass(copy_pass);

	// render pass for
	SDL_GPUColorTargetInfo color_target = {0};
	color_target.mip_level = 0;
	color_target.texture = target_texture;
	color_target.clear_color = SDL_FColor{0.f, 0.f, 0.f, 1.f};
	color_target.load_op = SDL_GPU_LOADOP_LOAD;
	color_target.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass *render_pass =
			SDL_BeginGPURenderPass(command_buffer, &color_target, 1, nullptr);

	SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
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
	SDL_DrawGPUIndexedPrimitives(render_pass, count_indices, 1, 0, 0, 0);

	SDL_EndGPURenderPass(render_pass);
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

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = static_cast<uint32_t>(sizeof(PositionColorVertex) * MAX_VERTICES_PER_BATCH
			+ sizeof(uint16_t) * MAX_INDICES_PER_BATCH)
	};
	transfer_buffer = SDL_CreateGPUTransferBuffer(
		gpu_device, &transfer_buffer_create_info);
}

Renderer2d::~Renderer2d() {
	SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer);
	SDL_ReleaseGPUBuffer(gpu_device, vertex_buffer);
	SDL_ReleaseGPUBuffer(gpu_device, index_buffer);
	SDL_ReleaseGPUGraphicsPipeline(gpu_device, pipeline);
}

void Renderer2d::beginPass(int z) {
	render_pass_stack.push_back(new RenderPass2d(this, z));
}
void Renderer2d::endPassWindow(SDL_GPUTexture* window_texture) {
	auto* pass = render_pass_stack.back();
	render_pass_stack.pop_back();

	// TODO fetch window size
	pass->setRenderTarget(RenderTargetType::Window, Vec2i(800, 600), window_texture);
	insertRenderPass(pass);
}
void Renderer2d::endPassTexture() {

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

	render_pass_stack.back()->addCommand(std::move(command));
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
	render_pass_stack.back()->addCommand(std::move(command));
}

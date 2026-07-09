#include "sluagh/render/Renderer2d.h"

#include "sluagh/core/Application.h"
#include "sluagh/render/utils.h"
#include "sluagh/utils/Logger.h"

struct PositionColorVertex {
	Vec4f pos;
	Vec4f color;
	Vec2f texture_coord;
	float texture;
};

RenderCommand2d::RenderCommand2d(uint32_t position_count, uint16_t index_count)
	: position_count(position_count), index_count(index_count), color(Colors::MAGENTA),
		texture(nullptr) {
	positions = new Vec2f[position_count];
	indices = new uint16_t[index_count];
	texture_coords = nullptr;
}

RenderCommand2d::RenderCommand2d(uint32_t position_count, uint16_t index_count, SDL_GPUTexture* texture)
	: position_count(position_count), index_count(index_count), color(Colors::MAGENTA), texture(texture) {
	positions = new Vec2f[position_count];
	indices = new uint16_t[index_count];
	texture_coords = new Vec2f[position_count];
}

RenderCommand2d::RenderCommand2d(RenderCommand2d &&o) noexcept {
	positions = o.positions;
	indices = o.indices;
	texture_coords = o.texture_coords;
	o.positions = nullptr;
	o.indices = nullptr;
	o.texture_coords = nullptr;

	color = o.color;
	position_count = o.position_count;
	index_count = o.index_count;
	texture = o.texture;
}

RenderCommand2d::~RenderCommand2d() {
	delete[] positions;
	delete[] indices;
	if (texture_coords != nullptr) {
		delete[] texture_coords;
	}
}

RenderPass2d::RenderPass2d(RenderTarget* target, Renderer2d *renderer, int z)
	: RenderPass(target), renderer{renderer} {
	z_index = z;
}

void RenderPass2d::addCommand(RenderCommand2d&& command) {
	render_commands.push_back(std::move(command));
}

void RenderPass2d::render() {
	renderer->renderPass(this);
}

void Renderer2d::renderPass(RenderPass2d *pass) {
	auto* target = pass->getRenderTarget();
	const auto& commands = pass->getRenderCommands();
	uint32_t count_vertices = 0;
	uint32_t count_indices = 0;
	uint32_t count_textures = 0;
	uint32_t last_flush_index = 0;
	auto textures = std::vector<SDL_GPUTexture*>(sampler_count, nullptr);
	for (uint32_t i = 0; i < commands.size(); i++) {
		count_vertices += commands[i].position_count;
		count_indices += commands[i].index_count;

		bool texture_found = true; // no texture => "always found"
		if (commands[i].texture != nullptr) {
			texture_found = false;
			for (uint32_t j = 0; j < count_textures; j++) {
				if (textures[j] == commands[i].texture) {
					texture_found = true;
					break;
				}
			}
		}

		if (count_vertices + commands[i].position_count >= MAX_VERTICES_PER_BATCH
			|| count_indices + commands[i].index_count >= MAX_INDICES_PER_BATCH
			|| (!texture_found && count_textures >= sampler_count)) {

			flush(
				target,
				pass->getZIndex(),
				commands,
				last_flush_index,
				i - last_flush_index,
				count_vertices,
				count_indices,
				textures,
				count_textures
			);
			last_flush_index = i;
			count_vertices = 0;
			count_indices = 0;
			count_textures = 0;
		}

		count_vertices += commands[i].position_count;
		count_indices += commands[i].index_count;
		if (!texture_found) {
			textures[count_textures] = commands[i].texture;
			count_textures++;
		}
	}

	if (count_vertices != 0 || count_indices != 0) {
		flush(
			target,
			pass->getZIndex(),
			commands,
			last_flush_index,
			static_cast<uint32_t>(commands.size() - last_flush_index),
			count_vertices,
			count_indices,
			textures,
			count_textures
		);
	}
}

void Renderer2d::flush(RenderTarget* target, int z_index,
		const std::vector<RenderCommand2d>& commands,
		uint32_t offset, uint32_t command_count,
		uint32_t count_vertices, uint32_t count_indices,
		const std::vector<SDL_GPUTexture*>& textures, uint32_t count_textures) {
	auto* transfer_data = static_cast<PositionColorVertex*>(
		SDL_MapGPUTransferBuffer(Application::getGPUDevice(), transfer_buffer, true)
	);
	auto* index_data = reinterpret_cast<uint16_t*>(&transfer_data[count_vertices]);

	const auto& proj = target->getProjectionMatrix();

	uint32_t vertex_i = 0;
	uint32_t index_i = 0;
	uint16_t vertex_offset = 0;
	for (uint32_t i = offset; i < offset + command_count; i++) {
		const auto& command = commands[i];
		float texture = -1.f;
		if (command.texture != nullptr) {
			for (uint32_t j = 0; j < count_textures; j++) {
				if (textures[j] == command.texture) {
					texture = static_cast<float>(j);
					break;
				}
			}
		}
		for (uint32_t j = 0; j < command.position_count; j++, vertex_i++) {

			transfer_data[vertex_i].pos = proj * Vec4f(
				command.positions[j].x(), command.positions[j].y(),
				z_index, 1.f
			);
			transfer_data[vertex_i].color = command.color.toVec4();
			if (command.texture != nullptr) {
				transfer_data[vertex_i].texture_coord = command.texture_coords[j];
			}
			transfer_data[vertex_i].texture = texture;
		}

		for (uint32_t j = 0; j < command.index_count; j++, index_i++) {
			index_data[index_i] = vertex_offset + command.indices[j];
		}
		vertex_offset += command.position_count;
	}

	SDL_UnmapGPUTransferBuffer(Application::getGPUDevice(), transfer_buffer);

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

	SDL_GPUColorTargetInfo color_target = {
		.texture = target->getTexture(),
		.mip_level = 0,
		.clear_color = SDL_FColor{0.f, 0.f, 0.f, 1.f},
		.load_op = SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
	};

	if (target->isMultisample()) {
		color_target.store_op = SDL_GPU_STOREOP_RESOLVE;
		color_target.texture = target->getMultisampleTexture();
		color_target.resolve_texture = target->getTexture();
		color_target.mip_level = 0;
	}

	SDL_GPUDepthStencilTargetInfo depth_target = {
		.texture = target->getDepthBuffer(),
		.load_op = SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
	};

	SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
		command_buffer, &color_target, 1, &depth_target
	);

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

	std::vector<SDL_GPUTextureSamplerBinding> sampler_bindings = {};
	sampler_bindings.reserve(sampler_count);
	for (uint32_t i = 0; i < count_textures; i++) {
		sampler_bindings.push_back({
			.texture = textures[i],
			.sampler = samplers[i]
		});
	}
	if (count_textures != sampler_count) {// not all texture slots used
		auto* white_texture = Application::getTextureManager()->getTexture(0)->getTexture();
		for (uint32_t i = count_textures; i < sampler_count; i++) {
			sampler_bindings.push_back({
				.texture = white_texture,
				.sampler = samplers[i]
			});
		}
	}
	SDL_BindGPUFragmentSamplers(render_pass, 0, sampler_bindings.data(), sampler_count);

	SDL_DrawGPUIndexedPrimitives(render_pass, count_indices, 1, 0, 0, 0);

	SDL_EndGPURenderPass(render_pass);
}

Renderer2d::Renderer2d() {
	auto* gpu_device = Application::getGPUDevice();

	sampler_count = 16;
	samplers = std::vector<SDL_GPUSampler*>(sampler_count, nullptr);
	for (uint32_t i = 0; i < sampler_count; i++) {
		SDL_GPUSamplerCreateInfo sampler_info = {
			.min_filter = SDL_GPU_FILTER_NEAREST,
			.mag_filter = SDL_GPU_FILTER_LINEAR,
			.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
			.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
			.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
			.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
		};
		samplers[i] = SDL_CreateGPUSampler(
			Application::getGPUDevice(), &sampler_info
		);
	}

	SDL_GPUShader *vertex_shader = loadShader(gpu_device,
		"./rsc/shader/basic.vert", 0, 0,
		0, 0);
	SDL_GPUShader *fragment_shader = loadShader(gpu_device,
		"./rsc/shader/basic.frag", sampler_count, 0,
		0, 0);

	SDL_GPUColorTargetDescription color_target = {
		.format = Application::getSwapchainTextureFormat(),
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
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
			.offset = 0
		},
		{
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
			.offset = sizeof(float) * 4
		},
		{
			.location = 2,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = sizeof(float) * 8
		},
		{
			.location = 3,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
			.offset = sizeof(float) * 10
		}
	};

	SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.vertex_input_state ={
			.vertex_buffer_descriptions = &vertex_buffer_description,
			.num_vertex_buffers = 1,
			.vertex_attributes = vertex_attribs,
			.num_vertex_attributes = 4,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.depth_stencil_state = {
			.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
			.enable_depth_test = true,
			.enable_depth_write = true
		},
		.target_info = {
			.color_target_descriptions = &color_target,
			.num_color_targets = 1,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			.has_depth_stencil_target = true
		}
	};

	const auto& config = Application::getConfig().graphic_config;
	if (config.enable_multisampling) {
		pipeline_info.multisample_state = {
			.sample_count = config.multisample_count,
			.sample_mask = 0,
			.enable_mask = false,
			.enable_alpha_to_coverage = false
		};
	}

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
	auto* gpu_device = Application::getGPUDevice();

	SDL_ReleaseGPUTransferBuffer(gpu_device, transfer_buffer);
	SDL_ReleaseGPUBuffer(gpu_device, vertex_buffer);
	SDL_ReleaseGPUBuffer(gpu_device, index_buffer);

	for (auto* sampler : samplers) {
		SDL_ReleaseGPUSampler(gpu_device, sampler);
	}

	SDL_ReleaseGPUGraphicsPipeline(gpu_device, pipeline);
}
void Renderer2d::beginPass(int z) {
	render_pass_stack.push_back(new RenderPass2d(
		Application::getMainWindow()->getRenderTarget(), this, z
	));
}

void Renderer2d::beginPass(RenderTarget* target, int z) {
	render_pass_stack.push_back(new RenderPass2d(target, this,  z));
}
void Renderer2d::endPass() {
	auto* pass = render_pass_stack.back();
	render_manager->addRenderPass(pass);
	render_pass_stack.pop_back();
}

void Renderer2d::setColor(Color new_color) {
	color = new_color;
}

void Renderer2d::drawLine(Vec2f start, Vec2f end, float thickness) {
	auto command = RenderCommand2d(4, 6);
	command.color = color;

	auto dir = (end - start).normalized();
	auto normal = dir.cross();

	auto tl = start - normal * thickness / 2.f;
	auto tr = end - normal * thickness / 2.f;
	auto br = end + normal * thickness / 2.f;
	auto bl = start + normal * thickness / 2.f;

	command.positions[0] = tl;
	command.positions[1] = br;
	command.positions[2] = tr;
	command.positions[3] = bl;

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 2;
	command.indices[3] = 0;
	command.indices[4] = 3;
	command.indices[5] = 1;

	render_pass_stack.back()->addCommand(std::move(command));
}
void Renderer2d::drawRect(Vec2f pos, Vec2f size, float thickness,
		BorderAlignment border_alignment) {
	auto command = RenderCommand2d(8, 24);

	auto inner_tl = pos;
	auto inner_br = pos + size;
	if (border_alignment == BorderAlignment::INSIDE) {
		inner_tl += thickness;
		inner_br -= thickness;
	} else if (border_alignment == BorderAlignment::CENTER) {
		inner_tl += thickness / 2.f;
		inner_br -= thickness / 2.f;
	}
	auto outer_tl = inner_tl - thickness;
	auto outer_br = inner_br + thickness;

	command.positions[0] = outer_tl;
	command.positions[1] = inner_tl;
	command.positions[2] = Vec2f{outer_br.x(), outer_tl.y()};
	command.positions[3] = Vec2f{inner_br.x(), inner_tl.y()};
	command.positions[4] = outer_br;
	command.positions[5] = inner_br;
	command.positions[6] = Vec2f{outer_tl.x(), outer_br.y()};
	command.positions[7] = Vec2f{inner_tl.x(), inner_br.y()};

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 2;
	command.indices[3] = 1;
	command.indices[4] = 3;
	command.indices[5] = 2;
	command.indices[6] = 2;
	command.indices[7] = 3;
	command.indices[8] = 4;
	command.indices[9] = 3;
	command.indices[10] = 5;
	command.indices[11] = 4;
	command.indices[12] = 4;
	command.indices[13] = 5;
	command.indices[14] = 6;
	command.indices[15] = 5;
	command.indices[16] = 7;
	command.indices[17] = 6;
	command.indices[18] = 6;
	command.indices[19] = 7;
	command.indices[20] = 0;
	command.indices[21] = 7;
	command.indices[22] = 1;
	command.indices[23] = 0;

	render_pass_stack.back()->addCommand(std::move(command));
}

void Renderer2d::fillRect(Vec2f pos, Vec2f size) {
	auto command = RenderCommand2d(4, 6);
	command.color = color;

	auto tl = pos;
	auto br = pos + size;

	command.positions[0] = Vec2f(tl.x(), tl.y());
	command.positions[1] = Vec2f(br.x(), br.y());
	command.positions[2] = Vec2f(br.x(), tl.y());
	command.positions[3] = Vec2f(tl.x(), br.y());

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 2;
	command.indices[3] = 0;
	command.indices[4] = 3;
	command.indices[5] = 1;
	render_pass_stack.back()->addCommand(std::move(command));
}
void Renderer2d::drawTexture(SDL_GPUTexture *texture, Vec2f pos, Vec2f size) {
	auto command = RenderCommand2d(4, 6, texture);
	command.color = Colors::WHITE;

	auto tl = pos;
	auto br = pos + size;

	command.positions[0] = Vec2f(tl.x(), tl.y());
	command.positions[1] = Vec2f(br.x(), br.y());
	command.positions[2] = Vec2f(br.x(), tl.y());
	command.positions[3] = Vec2f(tl.x(), br.y());

	command.texture_coords[0] = Vec2f(0.f, 0.f);
	command.texture_coords[1] = Vec2f(1.f, 1.f);
	command.texture_coords[2] = Vec2f(1.f, 0.f);
	command.texture_coords[3] = Vec2f(0.f, 1.f);

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 2;
	command.indices[3] = 0;
	command.indices[4] = 3;
	command.indices[5] = 1;
	render_pass_stack.back()->addCommand(std::move(command));
}

static uint16_t circle_segment_count(float radius) {
	auto d = std::acos(1.f - 0.3f / radius);
	return static_cast<uint16_t>(std::ceil(std::numbers::pi / d));
}

void Renderer2d::drawCircle(Vec2f pos, float radius, float thickness, BorderAlignment border_alignment) {
	auto segment_count = circle_segment_count(radius);

	if (border_alignment == BorderAlignment::INSIDE) {
		radius -= thickness;
	} else if (border_alignment == BorderAlignment::CENTER) {
		radius -= thickness / 2.f;
	}
	if (radius < 0.f) radius = 0.f;

	auto command = RenderCommand2d(2 * segment_count, 6 * segment_count);
	command.color = color;

	constexpr auto two_pi = std::numbers::pi * 2.f;
	for (uint16_t i = 0; i <= segment_count; i++) {
		auto angle = i * two_pi / segment_count;
		auto angle_vec = Vec2f{std::cos(angle), std::sin(angle)};
		command.positions[2 * i + 0] = pos + radius * angle_vec;
		command.positions[2 * i + 0] = pos + (radius + thickness) * angle_vec;

		command.indices[6 * i + 0] = 2 * i;
		command.indices[6 * i + 1] = 2 * i + 1;
		command.indices[6 * i + 2] = 2 * ((i + 1) % segment_count);
		command.indices[6 * i + 3] = 2 * i + 1;
		command.indices[6 * i + 4] = 2 * ((i + 1) % segment_count);
		command.indices[6 * i + 5] = 2 * ((i + 1) % segment_count) + 1;
	}

	render_pass_stack.back()->addCommand(std::move(command));
}

void Renderer2d::fillCircle(Vec2f pos, float radius) {
	auto segment_count = circle_segment_count(radius);

	auto command = RenderCommand2d(segment_count + 1, 3 * segment_count);
	command.color = color;

	constexpr auto two_pi = std::numbers::pi * 2.f;
	command.positions[0] = pos;
	for (uint16_t i = 0; i <= segment_count; i++) {
		auto angle = i * two_pi / segment_count;
		command.positions[1 + i] = pos + radius * Vec2f{std::cos(angle), std::sin(angle)};

		command.indices[3 * i] = 0;
		command.indices[3 * i + 1] = 1 + i;
		command.indices[3 * i + 2] = 1 + (i + 1) % segment_count;
	}

	render_pass_stack.back()->addCommand(std::move(command));
}

void Renderer2d::drawArc(Vec2f pos, float radius, float start_angle,
	float end_angle, float thickness, BorderAlignment border_alignment) {
	constexpr auto two_pi = std::numbers::pi * 2.f;
	auto angle_diff = end_angle - start_angle;

	if (start_angle >= end_angle) {
		Logger::warn("fillArc was called with a start_angle equal or greater than the end_angle.");
		return;
	}
	if (start_angle <= -two_pi || start_angle >= two_pi) {
		Logger::warn("fillArc expects a start_angle in the inclusive range of [-2*PI, 2*PI].");
		return;
	}
	if (angle_diff > two_pi) {
		Logger::warn("fillArc expects an angle_diff of less than 2*PI.");
		return;
	}

	auto segment_count = static_cast<uint16_t>(
		std::ceil(circle_segment_count(radius) / (angle_diff / two_pi))
	);

	if (border_alignment == BorderAlignment::INSIDE) {
		radius -= thickness;
	} else if (border_alignment == BorderAlignment::CENTER) {
		radius -= thickness / 2.f;
	}
	if (radius < 0.f) radius = 0.f;

	auto command = RenderCommand2d(2 * segment_count, 6 * segment_count);
	command.color = color;

	for (uint16_t i = 0; i <= segment_count; i++) {
		auto angle = start_angle + i * angle_diff / segment_count;
		auto angle_vec = Vec2f{std::cos(angle), std::sin(angle)};
		command.positions[2 * i + 0] = pos + radius * angle_vec;
		command.positions[2 * i + 0] = pos + (radius + thickness) * angle_vec;
	}

	for (uint16_t i = 0; i < segment_count; i++) {
		command.indices[6 * i + 0] = 2 * i;
		command.indices[6 * i + 1] = 2 * i + 1;
		command.indices[6 * i + 2] = 2 * (i + 1);
		command.indices[6 * i + 3] = 2 * i + 1;
		command.indices[6 * i + 4] = 2 * (i + 1);
		command.indices[6 * i + 5] = 2 * (i + 1) + 1;
	}

	render_pass_stack.back()->addCommand(std::move(command));
}

void Renderer2d::fillArc(Vec2f pos, float radius, float start_angle, float end_angle) {
	constexpr auto two_pi = std::numbers::pi * 2.f;
	auto angle_diff = end_angle - start_angle;

	if (start_angle >= end_angle) {
		Logger::warn("fillArc was called with a start_angle equal or greater than the end_angle.");
		return;
	}
	if (start_angle <= -two_pi || start_angle >= two_pi) {
		Logger::warn("fillArc expects a start_angle in the inclusive range of [-2*PI, 2*PI].");
		return;
	}
	if (angle_diff > two_pi) {
		Logger::warn("fillArc expects an angle_diff of less than 2*PI.");
		return;
	}

	auto segment_count = static_cast<uint16_t>(
		std::ceil(circle_segment_count(radius) / (angle_diff / two_pi))
	);

	auto command = RenderCommand2d(segment_count + 1, 3 * segment_count);
	command.color = color;

	command.positions[0] = pos;
	for (uint16_t i = 0; i <= segment_count + 1; i++) {
		auto angle = start_angle + i * angle_diff / segment_count;
		command.positions[1 + i] = pos + radius * Vec2f{std::cos(angle), std::sin(-angle)};
	}

	for (uint16_t i = 0; i < segment_count; i++) {
		command.indices[3 * i] = 0;
		command.indices[3 * i + 1] = 1 + i;
		command.indices[3 * i + 2] = 1 + i + 1;
	}

	render_pass_stack.back()->addCommand(std::move(command));
}

void Renderer2d::fillTriangle(Vec2f a, Vec2f b, Vec2f c) {
	auto command = RenderCommand2d(3, 3);
	command.color = color;

	auto m = Mat3f{
		a.x(), a.y(), 1.f,
		b.x(), b.y(), 1.f,
		c.x(), c.y(), 1.f
	};
	if (m.det() > 0) {
		auto tmp = b;
		b = c;
		c = tmp;
	}

	command.positions[0] = a;
	command.positions[1] = b;
	command.positions[2] = c;

	command.indices[0] = 0;
	command.indices[1] = 1;
	command.indices[2] = 2;
	render_pass_stack.back()->addCommand(std::move(command));
}

static void recursive_bezier(std::vector<Vec2f>& points, Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, uint8_t level = 0) {
	if (level > 16) return;

	constexpr auto distance_tolerance = 0.25f;
	constexpr auto angle_tolerance = 0.05f;

	auto p12 = (p1 + p2) / 2.f;
	auto p23 = (p2 + p3) / 2.f;
	auto p34 = (p3 + p4) / 2.f;
	auto p123 = (p12 + p23) / 2.f;
	auto p234 = (p23 + p34) / 2.f;
	auto p1234 = (p123 + p234) / 2.f;

	Vec2f d = p4 - p1;
	float d2 = std::fabs((p2.x() - p4.x()) * d.y() - (p2.y() - p4.y()) * d.x());
	float d3 = std::fabs((p3.x() - p4.x()) * d.y() - (p3.y() - p4.y()) * d.x());

	if ((d2 + d3) * (d2 + d3) < distance_tolerance * (d.x() * d.x() + d.y() * d.y())) {
		auto a23 = atan2(p3.y() - p2.y(), p3.x() - p2.x());
		auto da1 = std::fabs(a23 - atan2(p2.y() - p1.y(), p2.x() - p1.x()));
		auto da2 = std::fabs(atan2(p4.y() - p3.y(), p4.x() - p3.x()) - a23);
		if (da1 >= std::numbers::pi) da1 = 2 * std::numbers::pi - da1;
		if (da2 >= std::numbers::pi) da2 = 2 * std::numbers::pi - da2;

		if (da1 + da2 < angle_tolerance) {
			points.push_back(p23);
			return;
		}
	}

	recursive_bezier(points, p1, p12, p123, p1234, level + 1);
	recursive_bezier(points, p1234, p234, p34, p4, level + 1);
}

void Renderer2d::drawBezier(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f p4, float thickness) {
	auto points = std::vector<Vec2f>();
	points.push_back(p1);
	recursive_bezier(points, p1, p2, p3, p4);
	points.push_back(p4);

	std::vector<Vec2f> tangents;
	std::vector<Vec2f> normals;
	tangents.reserve(points.size());
	normals.reserve(points.size());
	for (uint16_t i = 0; i < points.size() - 1; i++) {
		auto tangent = (points[i+1] - points[i]).normalized();
		tangents.push_back(tangent);
		normals.push_back(tangent.cross());
	}
	auto end_tangent = (p4 - p3).normalized();
	tangents.push_back(end_tangent);
	normals.push_back(end_tangent.cross());

	auto command = RenderCommand2d(
		static_cast<uint32_t>(2 * points.size()),
		static_cast<uint16_t>((points.size() - 1) * 6)
	);
	command.color = color;

	for (uint16_t i = 0; i < points.size(); i++) {
		command.positions[2 * i + 0] = points[i] - normals[i] * thickness / 2.f;
		command.positions[2 * i + 1] = points[i] + normals[i] * thickness / 2.f;
	}

	for (uint16_t i = 0; i < points.size()-1; ++i) {
		auto t1 = tangents[i];
		auto t2 = tangents[i+1];
		auto n1 = normals[i];
		auto n2 = normals[i+1];

		auto cos_theta = std::clamp(t1.dot(t2), -1.f, 1.f);
		auto theta = std::acos(cos_theta);
		if (std::abs(theta) < 1e-3f) continue;

		Vec2f miter_dir = (n1 + n2).normalized();
		float denom = miter_dir.dot(n2);
		if (std::abs(denom) < 1e-6f) continue;

		float miter_len = thickness / 2.f / denom;
		command.positions[2 * i] = points[i] - miter_dir * miter_len;
		command.positions[2 * i + 1] = points[i] + miter_dir * miter_len;

		uint16_t tl_idx = 2 * i;
		uint16_t tr_idx = 2 * (i + 1);
		uint16_t bl_idx = 2 * i + 1;
		uint16_t br_idx = 2 * (i + 1) + 1;
		command.indices[6 * i + 0] = tl_idx;
		command.indices[6 * i + 1] = br_idx;
		command.indices[6 * i + 2] = tr_idx;
		command.indices[6 * i + 3] = tl_idx;
		command.indices[6 * i + 4] = bl_idx;
		command.indices[6 * i + 5] = br_idx;
	}

	render_pass_stack.back()->addCommand(std::move(command));
}


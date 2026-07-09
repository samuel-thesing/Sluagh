#version 460 core

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in float a_texture;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_tex_coord;
layout(location = 2) out float v_texture;

void main() {
    gl_Position = a_position;
    v_color = a_color;
    v_tex_coord = a_tex_coord;
    v_texture = a_texture;
}


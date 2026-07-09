#version 460 core
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_tex_coord;
layout(location = 2) in float v_texture;

layout(set = 2, binding = 0) uniform sampler2D texture0;
layout(set = 2, binding = 1) uniform sampler2D texture1;
layout(set = 2, binding = 2) uniform sampler2D texture2;
layout(set = 2, binding = 3) uniform sampler2D texture3;
layout(set = 2, binding = 4) uniform sampler2D texture4;
layout(set = 2, binding = 5) uniform sampler2D texture5;
layout(set = 2, binding = 6) uniform sampler2D texture6;
layout(set = 2, binding = 7) uniform sampler2D texture7;
layout(set = 2, binding = 8) uniform sampler2D texture8;
layout(set = 2, binding = 9) uniform sampler2D texture9;
layout(set = 2, binding = 10) uniform sampler2D texture10;
layout(set = 2, binding = 11) uniform sampler2D texture11;
layout(set = 2, binding = 12) uniform sampler2D texture12;
layout(set = 2, binding = 13) uniform sampler2D texture13;
layout(set = 2, binding = 14) uniform sampler2D texture14;
layout(set = 2, binding = 15) uniform sampler2D texture15;

layout(location = 0) out vec4 FragColor;

void main() {
    if (v_texture < -0.5f) {
        FragColor = v_color;
    } else {
        vec4 tex_color = vec4(0.0);
        switch (uint(v_texture)) {
            case 0: tex_color = texture(texture0, v_tex_coord); break;
            case 1: tex_color = texture(texture1, v_tex_coord); break;
            case 2: tex_color = texture(texture2, v_tex_coord); break;
            case 3: tex_color = texture(texture3, v_tex_coord); break;
            case 4: tex_color = texture(texture4, v_tex_coord); break;
            case 5: tex_color = texture(texture5, v_tex_coord); break;
            case 6: tex_color = texture(texture6, v_tex_coord); break;
            case 7: tex_color = texture(texture7, v_tex_coord); break;
            case 8: tex_color = texture(texture8, v_tex_coord); break;
            case 9: tex_color = texture(texture9, v_tex_coord); break;
            case 10: tex_color = texture(texture10, v_tex_coord); break;
            case 11: tex_color = texture(texture11, v_tex_coord); break;
            case 12: tex_color = texture(texture12, v_tex_coord); break;
            case 13: tex_color = texture(texture13, v_tex_coord); break;
            case 14: tex_color = texture(texture14, v_tex_coord); break;
            case 15: tex_color = texture(texture15, v_tex_coord); break;
        }
        FragColor = tex_color * v_color;
    }
}
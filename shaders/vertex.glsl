#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 text;
layout (location = 2) in vec3 normal;

out VS_OUT {
	vec2 text_uv;
} vs_out;

uniform mat4 trans;

void main()
{
	gl_Position = trans * vec4(position, 1.0f);
	vs_out.text_uv = text;
}

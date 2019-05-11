#version 330 core

in vec2 uv;

out vec4 color;

uniform sampler2D diffuse;

void main()
{
	color = vec4(texture(diffuse, uv).xyz, 1.0);
}

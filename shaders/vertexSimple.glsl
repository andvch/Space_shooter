#version 330 core
layout (location = 0) in vec3 position;

out vec2 coords;

uniform mat4 trans;

void main()
{
	gl_Position = trans * vec4(position, 1.0);
	coords = position.xy;
}

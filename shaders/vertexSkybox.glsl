#version 330 core
layout (location = 0) in vec3 position;

out vec3 coords;

uniform mat4 trans;

void main()
{
	coords = position;
	vec4 pos = trans * vec4(position, 1.0);
	gl_Position = pos.xyww;
}

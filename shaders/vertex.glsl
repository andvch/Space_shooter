#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 text;
layout (location = 2) in vec3 normal;

out vec2 UV;

uniform mat4 trans;
uniform vec3 lightPosition;

void main()
{
    gl_Position = trans * vec4(position, 1.0f);
    UV = text;
}
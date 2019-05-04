#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 text;
layout (location = 2) in vec3 normal;

out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;
out vec2 UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPosition;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
	UV = text;
    Normal_cameraspace = normal;
    LightDirection_cameraspace = lightPosition - position;
}
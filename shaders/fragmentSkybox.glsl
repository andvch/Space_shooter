#version 330 core

in vec3 coords;

out vec4 color;

uniform samplerCube skybox;
uniform float mixCoef;
uniform vec4 skyColor;

void main()
{
	color = mix(texture(skybox, coords), skyColor, mixCoef);
}

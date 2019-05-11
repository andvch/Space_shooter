#version 330 core

in vec2 coords;

out vec4 color;

uniform vec4 circlColor;

void main()
{
	if (length(coords) < 1.0f)
		color = circlColor;
	else
		discard;
}

#version 330 core

out vec4 color;

in vec2 UV;

uniform sampler2D diffuse;

void main()
{
    color = vec4(texture(diffuse, UV).xyz, 1.0);
}

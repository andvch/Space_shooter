#version 330 core

out vec4 color;

in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;
in vec2 UV;

uniform sampler2D diffuse;

void main()
{
    vec3 n = normalize(Normal_cameraspace);  // Normal of the computed fragment, in camera space
    vec3 l = normalize(LightDirection_cameraspace); // Direction of the light (from the fragment to the light)
    float cosTheta = clamp(dot(n,l), 0, 1);         // Cosine of the angle between the normal and the light direction, 

    color = vec4((0.1 + 1.3*cosTheta) * texture(diffuse, UV).xyz, 1.0);
}

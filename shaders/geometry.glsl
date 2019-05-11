#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
	vec2 text_uv;
} gs_in[];

out vec2 uv;

uniform float decay;

void main()
{
	vec4 m = (gl_in[0].gl_Position +
		gl_in[1].gl_Position + gl_in[2].gl_Position) / 3;

	gl_Position = gl_in[0].gl_Position + (m - gl_in[0].gl_Position) * decay;
	uv = gs_in[0].text_uv;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position + (m - gl_in[1].gl_Position) * decay;
	uv = gs_in[1].text_uv;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position + (m - gl_in[2].gl_Position) * decay;
	uv = gs_in[2].text_uv;
	EmitVertex();

	EndPrimitive();
}

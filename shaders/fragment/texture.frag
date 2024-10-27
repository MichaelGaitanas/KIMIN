#version 330 core

in vec2 uv;
out vec4 frag_col;

uniform sampler2D sample_tex;

void main()
{
	//Sample the texture bound to texture unit 0 at the specified (u,v) coordinates and return
	//the color of the corresponding texel (pixel in the texture).
	frag_col = texture(sample_tex, uv);
}
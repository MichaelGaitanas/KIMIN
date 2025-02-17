#version 450 core

in vec2 uv;
out vec4 frag_col;

//1) This refers to the texture bound to texture unit 0.
//2) The sampler2D uses binding mechanism instead of being passed directly via glUniform*(...).
uniform sampler2D sample_tex;

void main()
{
	//Sample the texture bound to texture unit 0 at the specified (u,v) coordinates and return
	//the color of the corresponding texel (pixel in the texture).
	frag_col = texture(sample_tex, uv);
}
#version 450 core

in vec3 uv;

out vec4 frag_col;

uniform samplerCube skybox;

void main()
{    
    frag_col = texture(skybox, uv);
}
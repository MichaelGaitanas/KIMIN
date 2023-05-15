#version 330 core

in vec3 tex_coords;

out vec4 frag_col;

uniform samplerCube skybox;

void main()
{    
    frag_col = texture(skybox, tex_coords);
}
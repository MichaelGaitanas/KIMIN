#version 330 core

out vec4 frag_col;

uniform vec3 model_col;

void main()
{
    frag_col = vec4(model_col, 1.0f); //same color for all fragments
}
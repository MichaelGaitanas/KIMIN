#version 450 core

out vec4 frag_col;



uniform vec3 mesh_col; //Mesh color.

void main()
{
    frag_col = vec4(mesh_col, 1.0); //Same color for all fragments.
}
#version 450 core

layout (location = 0) in vec3 pos;

uniform mat4 dir_light_pv; //Precomputed projection*view matrix.
uniform mat4 model;

void main()
{
    //The following operation, transforms all the scene's vertices (pos) to the directional light's (orthographic) view.
    gl_Position = dir_light_pv*model*vec4(pos, 1.0f);
}
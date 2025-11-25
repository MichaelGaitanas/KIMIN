#version 450 core

layout (location = 0) in vec3 pos;

uniform mat4 light_pv; //Precomputed directional light's projection*view matrix.
uniform mat4 model;

void main()
{
    gl_Position = light_pv*model*vec4(pos, 1.0f); //Transform all the scene's vertices (pos) to the directional light's (orthographic) view.
}
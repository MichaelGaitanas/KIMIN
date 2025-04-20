#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec4 frag_pos_light;
out vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 light_pv; //Precomputed directional light's projection*view matrix.

void main()
{
    frag_pos_light = light_pv*model*vec4(pos, 1.0f); //Fragment's position with respect to the light.
    normal = mat3(transpose(inverse(model)))*norm; //Avoiding non uniform scaling issues.
    gl_Position = projection*view*model*vec4(pos, 1.0f); //Final vertex position.
}
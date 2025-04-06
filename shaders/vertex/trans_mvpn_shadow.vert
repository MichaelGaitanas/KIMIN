#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec3 frag_pos_world;
out vec4 frag_pos_light;
out vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 dir_light_pv; //Light's projection*view matrix.

void main()
{
    frag_pos_world = vec3(model*vec4(pos,1.0f)); //Fragment's position in world coordinates.
    frag_pos_light = dir_light_pv*model*vec4(pos, 1.0f);
    normal = mat3(transpose(inverse(model)))*norm; //Avoiding non uniform scaling issues.
    gl_Position = projection*view*model*vec4(pos, 1.0f); //Final vertex position.
}
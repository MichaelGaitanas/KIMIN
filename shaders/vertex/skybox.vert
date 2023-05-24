#version 330 core

layout (location = 0) in vec3 pos;

out vec3 tex_coords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec4 frag_pos = projection*view*vec4(pos, 1.0f); //we don't deal with a model matrix, because it is always the identity one.
    tex_coords = vec3(pos.x, pos.y, -pos.z); //reversing the sign of pos.z due to the left handed coordinate system.
    gl_Position = vec4(frag_pos.x, frag_pos.y, frag_pos.w, frag_pos.w); //frag_pos.z = frag_pos.w means that depth = 1.0f always.
}
#version 330 core

layout (location = 0) in vec3 pos;

out vec3 tex_coords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec4 frag_pos = projection*view*vec4(pos, 1.0f);
    tex_coords = vec3(pos.x, pos.y, -pos.z);
    gl_Position = vec4(frag_pos.x, frag_pos.y, frag_pos.w, frag_pos.w);
}
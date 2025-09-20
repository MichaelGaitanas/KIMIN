#version 450 core

layout(location = 0) in vec3 pos;

out vec3 uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 frag_pos = projection*view*model*vec4(pos, 1.0f);
    uv = pos;
    gl_Position = frag_pos.xyww;
}
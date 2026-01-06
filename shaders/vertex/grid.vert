#version 450 core

layout(location = 0) in vec2 pos;

out vec2 v_ndc;

void main()
{
    v_ndc = pos;
    gl_Position = vec4(pos.x,pos.y,0.0, 1.0);
}
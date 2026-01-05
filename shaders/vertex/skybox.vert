#version 450 core

layout(location = 0) in vec3 pos;

out vec3 uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    uv = pos;
    uv.x = -uv.x;
    
    vec4 frag_pos = projection*view*model*vec4(pos, 1.0);
    gl_Position = frag_pos.xyww; //The .xyww forces : z_c = w_c. Hence z_ndc = z_c/w_c = 1.0 => so all the skybox's fragments have depth 1.0.
}
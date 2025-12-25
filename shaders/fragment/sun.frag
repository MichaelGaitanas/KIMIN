#version 450 core

layout(location = 0) out vec4 frag_col;

in vec2 coord;

uniform vec3 sun_color;

void main()
{
    if (length(coord) > 1.0) discard;

    frag_col = vec4(sun_color, 1.0);
}
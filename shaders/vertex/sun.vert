#version 450 core
layout(location = 0) in vec2 in_pos;

uniform mat4  projection;
uniform mat4  view;
uniform vec3  light_dir_world;
uniform float sun_angular_radius_deg;
uniform float sun_distance;
uniform float sun_scale;  // 1.0 for disc, >1 for rays

out vec2 q;

void main() {
    mat3 R = mat3(view);
    vec3 d = normalize(R * light_dir_world);

    vec3 up_ref = (abs(d.z) > 0.99) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 right  = normalize(cross(d, up_ref));
    vec3 upv    = normalize(cross(right, d));

    float ang = radians(sun_angular_radius_deg);
    float s   = tan(ang) * sun_distance * sun_scale;

    vec3 pos_view = d * sun_distance + right * (in_pos.x * s) + upv * (in_pos.y * s);
    gl_Position   = projection * vec4(pos_view, 1.0);

    q = in_pos;
}
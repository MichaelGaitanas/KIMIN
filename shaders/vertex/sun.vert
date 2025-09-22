#version 450 core
layout(location = 0) in vec2 in_pos;   // quad coords in [-1, 1]^2

uniform mat4  projection;
uniform mat4  view;
uniform vec3  light_dir_world;         // points from origin TOWARD the Sun
uniform float sun_angular_radius_deg;  // apparent half-angle (deg)
uniform float sun_distance;            // large-ish number in view units
uniform float sun_scale;               // 1.0 for disc, >1.0 for halo

out vec2 q;

void main()
{
    // rotate-only view (ignore translation)
    mat3 R = mat3(view);
    vec3 d = normalize(R * light_dir_world);   // Sun direction in view space

    // Orthonormal basis for billboard plane
    vec3 up_ref = (abs(d.z) > 0.99) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 right  = normalize(cross(d, up_ref));
    vec3 upv    = normalize(cross(right, d));

    // Size from angular radius (scaled for halo)
    float ang = radians(sun_angular_radius_deg);
    float s   = tan(ang) * sun_distance * sun_scale;   // half-size in view space

    // Place the quad center at distance along d, offset in its plane
    vec3 pos_view = d * sun_distance + right * (in_pos.x * s) + upv * (in_pos.y * s);
    gl_Position   = projection * vec4(pos_view, 1.0);

    q = in_pos; // pass to fragment for radial masks
}
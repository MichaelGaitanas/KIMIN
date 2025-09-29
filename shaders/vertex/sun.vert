#version 450 core

layout(location = 0) in vec2 pos; //Quad coords in [-1, 1]^2.

uniform mat4  projection;
uniform mat4  view;
uniform vec3  light_dir_world;         //Points from origin towards the Sun.
uniform float sun_angular_radius_deg;  //Apparent half angle [deg].
uniform float sun_distance;            //Large-ish number in view units.
uniform float sun_scale;               // = 1 for disc, > 1 for halo.

out vec2 q;

void main()
{
    //Rotate-only view (ignore translation).
    mat3 R = mat3(view);
    vec3 d = normalize(R*light_dir_world); //Sun direction in view space.

    //Orthonormal basis for billboard plane
    vec3 up_ref = (abs(d.z) > 0.99) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 right  = normalize(cross(d, up_ref));
    vec3 upv    = normalize(cross(right, d));

    //Size from angular radius (scaled for halo).
    float ang = radians(sun_angular_radius_deg);
    float s = tan(ang)*sun_distance*sun_scale;   //Half-size in view space.

    //Place the quad center at distance along d, offset in its plane.
    vec3 pos_view = d*sun_distance + right*(pos.x*s) + upv*(pos.y*s);
    gl_Position = projection*vec4(pos_view, 1.0);

    q = pos; //Pass to fragment for radial masks.
}
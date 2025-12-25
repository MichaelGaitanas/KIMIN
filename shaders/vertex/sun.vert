#version 450 core

layout(location = 0) in vec2 pos; //Quad coords in [-1,1]^2.

uniform mat4 projection;
uniform mat4 view;
uniform vec3 light_dir; //Points from origin towards the Sun.
uniform float apparent_angular_radius;  //Apparent half angle [rad].
uniform float quad_distance; //Quad's distance in view units.

out vec2 coord;

void main()
{
    //Rotate-only view (ignore translation).
    mat3 R = mat3(view);
    vec3 d = normalize(R*light_dir); //Sun direction in view space.

    //Orthonormal basis for billboard plane
    vec3 up_ref = (abs(d.z) > 0.99) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 right  = normalize(cross(d, up_ref));
    vec3 upv    = normalize(cross(right, d));

    float s = tan(apparent_angular_radius)*quad_distance;

    //Place the quad center at distance along d, offset in its plane.
    vec3 pos_view = d*quad_distance + right*(pos.x*s) + upv*(pos.y*s);
    gl_Position = projection*vec4(pos_view, 1.0);

    coord = pos; //Pass to fragment for radial masks.
}
#version 450 core

layout(location = 0) in vec2 pos; //Quad coords in [-1,1]^2.

out vec2 coord;

const float qscale = 500.0;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 light_dir; //Points from world origin towards the Sun.
uniform float apparent_angular_radius; //Sun's apparent half angle [rad].
uniform float quad_distance; //Quad's distance in view units.

//Remember, the Sun is a 2D quad in its geometry.
//In this vertex shader I am doing the billboard rotation effect used in the early 3D games.
//See the statues at : https://www.youtube.com/watch?v=VQwazYorcyE&t=2359s : 39:18-39:24
void main()
{
    mat3 R = mat3(view); //Rotate only view (ignore translation).
    vec3 light_dir_norm = normalize(light_dir);

    vec3 ref_world = vec3(0.0,0.0,1.0);
    if (abs(dot(light_dir_norm, ref_world)) > 0.99)
        ref_world = vec3(0.0,1.0,0.0);

    //Build a stable orthonormal basis in world frame.
    vec3 right_world = normalize(cross(ref_world, light_dir_norm));
    vec3 up_world    = normalize(cross(light_dir_norm, right_world));

    //Move basis in view frame.
    vec3 d = normalize(R*light_dir_norm);
    vec3 right_view = R*right_world;
    vec3 up_view = R*up_world;

    float s = quad_distance*tan(apparent_angular_radius);
    vec3 pos_view = d*quad_distance + (pos.x*s*qscale)*right_view + (pos.y*s*qscale)*up_view;

    coord = pos*qscale; //Scale the [-1,1] quad to give space to the fragment shader next.
    gl_Position = projection*vec4(pos_view, 1.0);
}
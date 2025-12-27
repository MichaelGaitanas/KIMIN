#version 450 core

layout(location = 0) in vec2 pos; //Quad coords in [-1,1]^2.

uniform mat4 projection;
uniform mat4 view;
uniform vec3 light_dir; //Points from world origin towards the Sun.
uniform float apparent_angular_radius; //Apparent half angle [rad].
uniform float quad_distance; //Quad's distance in view units.

out vec2 coord;

void main()
{
    //Rotate-only view (ignore translation).
    mat3 R = mat3(view);
    // Sun direction in world space (given)
    vec3 dW = normalize(light_dir);

    // Choose a fixed world reference "north"
    vec3 refW = vec3(0.0, 0.0, 1.0);
    if (abs(dot(dW, refW)) > 0.99) refW = vec3(0.0, 1.0, 0.0);

    // Build a stable orthonormal basis in WORLD space
    vec3 rightW = normalize(cross(refW, dW));
    vec3 upW    = normalize(cross(dW, rightW));

    // Move basis into VIEW space
    vec3 d  = normalize(R * dW);
    vec3 right = R * rightW;
    vec3 upv   = R * upW;

    float s = tan(apparent_angular_radius) * quad_distance;
    vec3 pos_view = d * quad_distance + right * (pos.x * s) + upv * (pos.y * s);
    gl_Position = projection * vec4(pos_view, 1.0);

    coord = pos;
}
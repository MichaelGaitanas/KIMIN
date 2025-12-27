#version 450 core

layout(location = 0) out vec4 frag_col;

in vec2 coord;

//Noise functions (procedural turbulence for surface).
float random(in vec3 st)
{
    return fract(sin(dot(st, vec3(12.9898, 78.233, 23.112))) * 12943.145);
}

float noise(in vec3 _pos)
{
    vec3 i_pos = floor(_pos);
    vec3 f_pos = fract(_pos);

    float aa = random(i_pos);
    float ab = random(i_pos + vec3(1., 0., 0.));
    float ac = random(i_pos + vec3(0., 1., 0.));
    float ad = random(i_pos + vec3(1., 1., 0.));
    float ae = random(i_pos + vec3(0., 0., 1.));
    float af = random(i_pos + vec3(1., 0., 1.));
    float ag = random(i_pos + vec3(0., 1., 1.));
    float ah = random(i_pos + vec3(1., 1., 1.));

    float ba = random(i_pos + 1.);
    float bb = random(i_pos + 1. + vec3(1., 0., 0.));
    float bc = random(i_pos + 1. + vec3(0., 1., 0.));
    float bd = random(i_pos + 1. + vec3(1., 1., 0.));
    float be = random(i_pos + 1. + vec3(0., 0., 1.));
    float bf = random(i_pos + 1. + vec3(1., 0., 1.));
    float bg = random(i_pos + 1. + vec3(0., 1., 1.));
    float bh = random(i_pos + 1. + vec3(1., 1., 1.));

    vec3 t = smoothstep(0., 1., f_pos);
    float t_time = smoothstep(0., 1., 0.2);

    return mix(
        mix(
            mix(mix(aa, ab, t.x), mix(ac, ad, t.x), t.y),
            mix(mix(ae, af, t.x), mix(ag, ah, t.x), t.y),
            t.z
        ),
        mix(
            mix(mix(ba, bb, t.x), mix(bc, bd, t.x), t.y),
            mix(mix(be, bf, t.x), mix(bg, bh, t.x), t.y),
            t.z
        ),
        t_time
    );
}

#define NUM_OCTAVES 6
float fBm(in vec3 _pos, in float sz) {
    float v = 0.0;
    float a = 0.2;
    _pos *= sz;
    vec3 angle = vec3(0.001, 0.0001, 0.0004);
    
    mat3 rotx = mat3(1, 0, 0, 0, cos(angle.x), -sin(angle.x), 0, sin(angle.x), cos(angle.x));
    mat3 roty = mat3(cos(angle.y), 0, sin(angle.y), 0, 1, 0, -sin(angle.y), 0, cos(angle.y));
    mat3 rotz = mat3(cos(angle.z), -sin(angle.z), 0, sin(angle.z), cos(angle.z), 0, 0, 0, 1);
    
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(_pos);
        _pos = rotx * roty * rotz * _pos * 2.0;
        a *= 0.8;
    }
    return v;
}

void main()
{
    float r = length(coord);
    
    if (r > 1.0)
        discard;

    // Project to virtual sphere for 3D noise.
    float theta = atan(coord.y, coord.x);
    float sin_phi = r;
    float cos_phi = sqrt(1.0 - sin_phi * sin_phi);
    vec3 st = vec3(sin_phi * cos(theta), sin_phi * sin(theta), cos_phi);

    // Procedural surface noise.
    vec3 q = vec3(0.);
    q.x = fBm(st, 5.);
    q.y = fBm(st + vec3(1.2, 3.2, 1.52), 5.);
    q.z = fBm(st + vec3(0.02, 0.12, 0.152), 5.);
    float n = fBm(st + q + vec3(1.82, 1.32, 1.09), 5.);

    vec3 color = mix(vec3(1., 0.4, 0.), vec3(1., 1., 1.), n);

    color *= 1.6; // Boost for brightness.

    frag_col = vec4(color, 1.0); // Opaque disk.
}
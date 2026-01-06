#version 450 core

out vec4 frag_col;

in vec2 coord;

const int n_octaves = 6;
const float qscale = 500.0;

//Pseudo RNG in [0,1).
float random(vec3 st)
{
    return fract(12943.145*sin(dot(st, vec3(12.9898, 78.233, 23.112))));
}

//Noise function for procedural turbulence on the Sun's surface.
float noise(vec3 pos)
{
    vec3 ipos = floor(pos);
    vec3 fpos = fract(pos);

    float aa = random(ipos);
    float ab = random(ipos + vec3(1.0,0.0,0.0));
    float ac = random(ipos + vec3(0.0,1.0,0.0));
    float ad = random(ipos + vec3(1.0,1.0,0.0));
    float ae = random(ipos + vec3(0.0,0.0,1.0));
    float af = random(ipos + vec3(1.0,0.0,1.0));
    float ag = random(ipos + vec3(0.0,1.0,1.0));
    float ah = random(ipos + vec3(1.0,1.0,1.0));

    float ba = random(ipos + 1.0);
    float bb = random(ipos + 1.0 + vec3(1.0, 0.0, 0.0));
    float bc = random(ipos + 1.0 + vec3(0.0, 1.0, 0.0));
    float bd = random(ipos + 1.0 + vec3(1.0, 1.0, 0.0));
    float be = random(ipos + 1.0 + vec3(0.0, 0.0, 1.0));
    float bf = random(ipos + 1.0 + vec3(1.0, 0.0, 1.0));
    float bg = random(ipos + 1.0 + vec3(0.0, 1.0, 1.0));
    float bh = random(ipos + 1.0 + vec3(1.0, 1.0, 1.0));

    vec3 t = smoothstep(0.0, 1.0, fpos);

    return mix(
        mix(mix(mix(aa, ab, t.x), mix(ac, ad, t.x), t.y),
            mix(mix(ae, af, t.x), mix(ag, ah, t.x), t.y),
            t.z),
        mix(mix(mix(ba, bb, t.x), mix(bc, bd, t.x), t.y),
            mix(mix(be, bf, t.x), mix(bg, bh, t.x), t.y),
            t.z),
        smoothstep(0.0, 1.0, 0.2)
    );
    //I made a mixer. VSOOUUUUUNNNNN!
}

float fractional_brownian(vec3 pos, float sz)
{
    vec3 angle = vec3(0.001, 0.0001, 0.0004);
    
    mat3 rotx = mat3(1.0,     0.0,           0.0, 
                     0.0, cos(angle.x), -sin(angle.x),
                     0.0, sin(angle.x),  cos(angle.x));

    mat3 roty = mat3(cos(angle.y), 0.0, sin(angle.y),
                         0.0,      1.0,     0.0,
                    -sin(angle.y), 0.0, cos(angle.y));

    mat3 rotz = mat3(cos(angle.z), -sin(angle.z), 0.0,
                     sin(angle.z),  cos(angle.z), 0.0,
                         0.0,           0.0,      1.0);

    float v = 0.0;
    float a = 0.2;
    pos *= sz;
    for (int i = 0; i < n_octaves; ++i)
    {
        v += a*noise(pos);
        pos = rotx*roty*rotz*pos*2.0;
        a *= 0.8;
    }

    return v;
}

void main()
{
    float r = length(coord);

    //1 : Outside Sun's corona :
    if (r > qscale)
        discard;

    //2 : Sun's corona region :
    if (r > 1.0)
    {
        float a = 1.1, b = 0.33, c = 2.0; //See mathematica notebook for these. Maybe I will add rays in the future.
        float intensity = pow(a, -pow((pow(r,b) - 1.0), c) );
        frag_col = vec4(intensity*vec3(1.0,0.65,0.1), intensity);
        return;
    }

    //3 : Sun's physical disk region :

    //Project 2D coordinates to virtual 3D sphere. Here is where we "transform" the 2D quad to a sphere.
    float theta = atan(coord.y, coord.x);
    float sin_phi = r;
    float cos_phi = sqrt(max(0.0, 1.0 - sin_phi*sin_phi));
    vec3 st = vec3(sin_phi*cos(theta), sin_phi*sin(theta), cos_phi);
    //Now apply actual procedural surface texture.
    vec3 q = vec3(0.0);
    q.x = fractional_brownian(st, 5.0);
    q.y = fractional_brownian(st + vec3(1.2, 3.2, 1.52), 5.0);
    q.z = fractional_brownian(st + vec3(0.02, 0.12, 0.152), 5.0);
    float n = fractional_brownian(st + q + vec3(1.82, 1.32, 1.09), 5.0);
    vec3 color = 1.5*mix(vec3(1.0, 0.4, 0.0), vec3(1.0, 1.0, 1.0), n);

    frag_col = vec4(color, 1.0);
}
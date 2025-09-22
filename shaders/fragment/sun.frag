#version 450 core
layout(location = 0) out vec4 frag_col;

in vec2 q;

// Mode: 0 = core disc, 1 = halo ring
uniform int   u_mode;

// Your 7 GUI knobs
uniform vec3  sun_color;
uniform float sun_disc_intensity;
uniform float sun_disc_edge_soft;  // also controls halo breadth/strength
uniform float sun_limb_strength;
uniform float sun_limb_power;

// From vertex (so we know this pass' scale)
uniform float sun_scale;

void main()
{
    float r = length(q);

    if (u_mode == 0) {
        // ----- Core disc -----
        if (r > 1.0) discard;

        // Limb darkening
        float mu   = sqrt(max(0.0, 1.0 - r*r));
        float limb = mix(1.0, pow(mu, sun_limb_power), clamp(sun_limb_strength, 0.0, 1.0));

        // Soft edge near the rim
        float soft = smoothstep(1.0 - sun_disc_edge_soft, 1.0, r);

        vec3 col = sun_color * sun_disc_intensity * limb * (1.0 - soft);
        frag_col = vec4(col, 1.0);
        return;
    }
}
#version 450 core
layout(location = 0) out vec4 frag_col;

in vec2 q;

// u_mode: 0 = disc, 1 = halo (optional), 2 = rays
uniform int   u_mode;
uniform vec3  sun_color;

// ----- Disc uniforms (you already have these) -----
uniform float sun_disc_intensity;
uniform float sun_disc_edge_soft;
uniform float sun_limb_strength;
uniform float sun_limb_power;

// ----- Rays uniforms -----
uniform int   sun_rays_count;       // N rays (even # recommended)
uniform float sun_rays_intensity;   // additive brightness
uniform float sun_rays_width_frac;  // 0..1 of half-sector width
uniform float sun_rays_sharpness;   // >1
uniform float sun_rays_falloff;     // radial exponent
uniform float sun_rays_rotation;    // degrees

// Shared with vertex so we know the scale of this pass
uniform float sun_scale;            // 1.0 for disc, >1.0 for rays/halo

void main()
{
    float r = length(q);

    if (u_mode == 0) {
        // ---- Core disc (unchanged) ----
        if (r > 1.0) discard;

        float mu   = sqrt(max(0.0, 1.0 - r*r));
        float limb = mix(1.0, pow(mu, sun_limb_power), clamp(sun_limb_strength, 0.0, 1.0));
        float soft = smoothstep(1.0 - sun_disc_edge_soft, 1.0, r);

        vec3 col = sun_color * sun_disc_intensity * limb * (1.0 - soft);
        frag_col = vec4(col, 1.0);
        return;
    }
    else if (u_mode == 2) {
        // ---- Starburst rays (additive ring) ----
        // Only draw outside the disc, inside the scaled quad edge
        float inner = 1.0 / max(sun_scale, 1.0001); // disc radius in this pass
        if (r < inner || r > 1.0) discard;

        // Angular distance to nearest ray axis
        float theta  = atan(q.y, q.x) - radians(sun_rays_rotation);
        float sector = 6.28318530718 / float(max(sun_rays_count, 1));
        // Wrap theta to [-sector/2, +sector/2] around nearest axis
        float a = abs(mod(theta + 0.5*sector, sector) - 0.5*sector);

        // Width of one ray (radians): fraction of half-sector
        float width = clamp(sun_rays_width_frac, 0.001, 0.999) * (0.5 * sector);

        // Angular profile: 1 on axis -> 0 at width
        float angProfile = clamp(1.0 - a / width, 0.0, 1.0);
        angProfile = pow(angProfile, sun_rays_sharpness);

        // Radial profile: 1 at inner (disc edge), -> 0 at outer edge
        float t = (r - inner) / (1.0 - inner);
        float radProfile = pow(1.0 - t, sun_rays_falloff);

        float I = sun_rays_intensity * angProfile * radProfile;

        frag_col = vec4(sun_color * I, 1.0); // additive target
        return;
    }

    discard; // for u_mode == 1 (halo) if you’re not using it
}
#version 450 core
in vec2 v_ndc;
layout(location=0) out vec4 frag_col;

uniform mat4 uProj;         // projection
uniform mat4 uView;         // view
uniform float uFadeEnd;     // fade-out radius on plane (e.g., 2*cam.dist)

// look
const float LINE_PX    = 1.0;       // exact pixel thickness (tune 1.5–2.0)
const float TARGET_PX  = 224.0;      // bigger => fewer lines (try 224/256)

// ===== helpers (no inverse()) =====
vec3 cameraWorldPos() {
    mat3 R = mat3(uView);
    vec3 t = vec3(uView[3]);
    return -transpose(R) * t;
}
vec3 viewToWorld(vec3 v) { return transpose(mat3(uView)) * v; }
vec3 ndcToViewDir(vec2 ndc) {
    float invFx = 1.0 / uProj[0][0];
    float invFy = 1.0 / uProj[1][1];
    return normalize(vec3(ndc.x * invFx, ndc.y * invFy, -1.0));
}
float nice12(float x){
    float lx=log(max(x,1e-12)), e=floor(lx/log(10.0));
    float m=x/pow(10.0,e);
    float n=(m<1.5)?1.0: (m<6.0)?2.0: 10.0;
    return n*pow(10.0,e);
}
float next12(float a){
    float lx=log(max(a,1e-12)), e=floor(lx/log(10.0));
    float m=a/pow(10.0,e);
    float n=(m<1.5)?2.0:10.0;
    if(n>=10.0-1e-6){ e+=1.0; n=1.0; }
    return n*pow(10.0,e);
}

void main() {
    // ---- world-space hit on z=0 plane
    vec3 camPos = cameraWorldPos();
    vec3 dirWS  = normalize(viewToWorld(ndcToViewDir(v_ndc)));
    float dz    = dirWS.z;
    if (abs(dz) < 1e-6) discard;
    float t = -camPos.z / dz;
    if (t <= 0.0) discard;
    vec3 P = camPos + t * dirWS;

    // correct depth
    vec4 clip = uProj * uView * vec4(P, 1.0);
    gl_FragDepth = (clip.z / clip.w) * 0.5 + 0.5;

    // ---- view-angle aware LOD (constant per draw) using center ray
    vec3 dirC   = normalize(viewToWorld(ndcToViewDir(vec2(0.0))));
    float tC    = abs(camPos.z) / max(abs(dirC.z), 1e-6);

    // NDC pixel size is constant for full-screen quad
    float ndcPxX = fwidth(v_ndc.x);
    float ndcPxY = fwidth(v_ndc.y);

    // world-units per pixel at center
    float invFx = 1.0 / uProj[0][0];
    float invFy = 1.0 / uProj[1][1];
    float wpp_center = max(tC * invFx * ndcPxX, tC * invFy * ndcPxY);

    // two adjacent pleasant spacings and smooth crossfade
    float raw    = wpp_center * TARGET_PX;
    float cellA  = nice12(raw);
    float cellB  = next12(cellA);
    float logRaw = log(raw), logA = log(cellA), logB = log(cellB);
    float k      = clamp((logRaw - logA) / max(logB - logA, 1e-12), 0.0, 1.0);
    k            = smoothstep(0.75, 1.0, k);

    // ===== per-axis coverage with per-axis gradients (keeps px thickness) =====
    // For a vertical line family (x = const):
    // distance field dX = distance to nearest vertical line in world units.
    // ∂dX/∂P.x ≈ ±1 piecewise -> |∇dX| in screen space ≈ |∇P.x|.
    float dXA = abs(fract(P.x / cellA) - 0.5) * cellA;
    float dYA = abs(fract(P.y / cellA) - 0.5) * cellA;
    float dXB = abs(fract(P.x / cellB) - 0.5) * cellB;
    float dYB = abs(fract(P.y / cellB) - 0.5) * cellB;

    // screen-space gradient magnitudes per axis
    float gradPx = length(vec2(dFdx(P.x), dFdy(P.x))) + 1e-12;
    float gradPy = length(vec2(dFdx(P.y), dFdy(P.y))) + 1e-12;

    // exact pixel widths
    float halfX = 0.5 * LINE_PX * gradPx;
    float halfY = 0.5 * LINE_PX * gradPy;
    float aaX   = gradPx;   // ~1 px AA
    float aaY   = gradPy;

    // coverage for spacing A (vertical and horizontal families)
    float covXA = 1.0 - smoothstep(halfX, halfX + aaX, dXA);
    float covYA = 1.0 - smoothstep(halfY, halfY + aaY, dYA);
    // combine families without over-brightening
    float covA  = covXA + covYA - covXA * covYA; // “or” in coverage space

    // coverage for spacing B
    float covXB = 1.0 - smoothstep(halfX, halfX + aaX, dXB);
    float covYB = 1.0 - smoothstep(halfY, halfY + aaY, dYB);
    float covB  = covXB + covYB - covXB * covYB;

    // crossfade between LODs
    float cov = mix(covA, covB, k);

    // radial fade 0 → uFadeEnd
    float fade = 1.0 - smoothstep(0.0, max(uFadeEnd, 1e-6), length(P.xy));

    float alpha = cov * fade;
    if (alpha <= 0.001) discard;

    frag_col = vec4(vec3(0.30), alpha);
}
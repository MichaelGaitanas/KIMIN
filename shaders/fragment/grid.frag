#version 450 core
in vec2 v_ndc;
layout(location=0) out vec4 frag_col;

uniform mat4 uProj;          // same uniforms you already set
uniform mat4 uView;

uniform vec3  uColor;        // grid color
uniform float uCell;         // world-units per cell
uniform float uPx;           // line thickness in SCREEN pixels
uniform float uFadeStart;    // fade starts at radius
uniform float uFadeEnd;      // fade ends at radius

// --- derive camera world pos and view->world rotation WITHOUT inverse() ---
vec3 cameraWorldPos()
{
    // For a rigid view matrix V = [ R  t ; 0 1 ] (world->view),
    // camera position C = -R^T * t
    mat3 R = mat3(uView);
    vec3 t = vec3(uView[3]);
    return -transpose(R) * t;
}

vec3 viewToWorld(vec3 v)
{
    // inverse rotation = transpose(R)
    return transpose(mat3(uView)) * v;
}

// --- build view-space ray dir without inverse(uProj) ---
vec3 ndcToViewDir(vec2 ndc)
{
    // For standard perspective: dirVS ~ (ndc.x / P00, ndc.y / P11, -1)
    float invFx = 1.0 / uProj[0][0];
    float invFy = 1.0 / uProj[1][1];
    return normalize(vec3(ndc.x * invFx, ndc.y * invFy, -1.0));
}

void main()
{
    vec3 camPos = cameraWorldPos();

    // ray in world
    vec3 dirVS = ndcToViewDir(v_ndc);
    vec3 dirWS = normalize(viewToWorld(dirVS));

    // intersect with plane z = 0 (z-up)
    float dz = dirWS.z;
    if (abs(dz) < 1e-6) discard;
    float t = -camPos.z / dz;
    if (t <= 0.0) discard;

    vec3 P = camPos + t * dirWS;   // world hit point

    // correct depth so grid is occluded properly
    vec4 clip = uProj * uView * vec4(P, 1.0);
    float ndcZ = clip.z / clip.w;
    gl_FragDepth = ndcZ * 0.5 + 0.5;

    // grid distances in world units (XY plane)
    float c = max(uCell, 1e-6);
    float gx = abs(fract(P.x / c) - 0.5) * c;
    float gy = abs(fract(P.y / c) - 0.5) * c;

    // world-units per pixel around this fragment
    float wpx = length(vec2(dFdx(P.x), dFdy(P.x)));
    float wpy = length(vec2(dFdx(P.y), dFdy(P.y)));

    // half thickness in world units per axis (pixel-true thickness)
    float halfX = max(1e-6, 0.5 * uPx * wpx);
    float halfY = max(1e-6, 0.5 * uPx * wpy);

    // AA widths
    float aax = fwidth(gx);
    float aay = fwidth(gy);

    float lineX = 1.0 - smoothstep(halfX, halfX + aax, gx);
    float lineY = 1.0 - smoothstep(halfY, halfY + aay, gy);
    float line  = max(lineX, lineY);

    // distance fade (on plane)
    float fs = min(uFadeStart, uFadeEnd);
    float fe = max(uFadeStart, uFadeEnd);
    float r   = length(P.xy);
    float fade = 1.0 - smoothstep(fs, fe, r);

    float alpha = line * fade;
    if (alpha <= 0.001) discard;

    frag_col = vec4(uColor, alpha);
}
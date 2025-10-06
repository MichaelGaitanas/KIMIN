#version 450 core
in vec2 v_ndc;
layout(location=0) out vec4 frag_col;

uniform mat4 uProj;        // camera projection
uniform mat4 uView;        // camera view

uniform vec3 uColor;  // grid line color
uniform float uCell;        // world units per cell
uniform float uPx;        // line thickness in pixels
uniform float uFadeStart;   // world radius where fade begins
uniform float uFadeEnd;  // world radius where fully faded

// Reconstruct world-space camera origin and a view ray from ndc
vec3 getCamPos(){
    mat4 invView = inverse(uView);
    return vec3(invView[3]); // translation column
}
vec3 ndcToWorldDir(vec2 ndc){
    // view-space point on the far plane at ndc (x,y,1)
    vec4 vFar  = inverse(uProj) * vec4(ndc, 1.0, 1.0);
    vec3 dirVS = normalize(vFar.xyz / vFar.w);
    // to world
    mat3 invR = mat3(inverse(uView)); // rotation only
    return normalize(invR * dirVS);
}

void main(){
    vec3 camPos = getCamPos();
    vec3 dir    = ndcToWorldDir(v_ndc);

    // Intersect ray (camPos + t*dir) with z=0 plane
    float denom = dir.z;
    // If ray parallel or points away and camera on plane → discard
    if (abs(denom) < 1e-6) discard;

    float t = -camPos.z / denom;
    if (t <= 0.0) discard; // behind the camera

    vec3 P = camPos + t*dir;    // world hit point on the XY plane

    // Compute correct depth so grid is occluded by scene
    vec4 clip = uProj * uView * vec4(P,1.0);
    float ndcZ = clip.z / clip.w;
    gl_FragDepth = ndcZ * 0.5 + 0.5; // map [-1,1] -> [0,1]

    // --- world-space distances to nearest vertical/horizontal lines
    float gx = abs(fract(P.x / uCell) - 0.5) * uCell;
    float gy = abs(fract(P.y / uCell) - 0.5) * uCell;

    // per-axis "world units per pixel"
    float wpx = length(vec2(dFdx(P.x), dFdy(P.x)));
    float wpy = length(vec2(dFdx(P.y), dFdy(P.y)));

    // desired half thickness in world units, per axis
    float halfX = max(1e-6, 0.5 * uPx * wpx);
    float halfY = max(1e-6, 0.5 * uPx * wpy);

    // anti-alias width per axis
    float aax = fwidth(gx);
    float aay = fwidth(gy);

    // line masks per axis
    float lineX = 1.0 - smoothstep(halfX, halfX + aax, gx);
    float lineY = 1.0 - smoothstep(halfY, halfY + aay, gy);

    // combine: a grid cell edge is present if either axis hits
    float line = max(lineX, lineY);

    // Distance-based fade to feel "infinite"
    float r = length(P.xy);
    float fade = 1.0 - smoothstep(uFadeStart, uFadeEnd, r);

    float alpha = line * fade;
    if (alpha <= 0.001) discard;

    frag_col = vec4(uColor, alpha);
}
#version 450 core

in vec2 v_ndc;

out vec4 frag_col;

uniform mat4 projection;
uniform mat4 view;
uniform float fade_end_dist;
uniform vec3 grid_origin;

const float TARGET_PIX = 700.0;

vec3 view2world(vec3 v)
{
    return transpose(mat3(view))*v;
}

vec3 ndc2view(vec2 ndc)
{
    float invFx = 1.0/projection[0][0];
    float invFy = 1.0/projection[1][1];
    return normalize(vec3(ndc.x*invFx, ndc.y*invFy, -1.0));
}

float cov_for_cell(float cell, vec3 P, float half_size, float aaX, float aaY)
{
    float dX = abs(fract(P.x/cell) - 0.5)*cell;
    float dY = abs(fract(P.y/cell) - 0.5)*cell;

    float covX = 1.0 - smoothstep(half_size, half_size + aaX, dX);
    float covY = 1.0 - smoothstep(half_size, half_size + aaY, dY);

    return covX + covY - covX*covY;
}

void main()
{
    vec3 cam_pos_world = -transpose(mat3(view))*vec3(view[3]);
    vec3 ray_dir_world = normalize(view2world(ndc2view(v_ndc))); //Read it from right-to-left.

    //Intersect plane z = 0.
    if (abs(ray_dir_world.z) < 1e-6)
        discard;

    float t = (grid_origin.z - cam_pos_world.z)/ray_dir_world.z;
    if (t <= 0.0)
        discard;

    vec3 P = cam_pos_world + t*ray_dir_world;
    vec3 Plocal = P - grid_origin;

    //Depth :
    vec4 clip = projection*view*vec4(P, 1.0);
    gl_FragDepth = (clip.z/clip.w)*0.5 + 0.5;

    //World units per pixel (stable, from center ray)
    vec3 dirC = normalize(view2world(ndc2view(vec2(0.0))));
    float tC = abs(grid_origin.z - cam_pos_world.z)/max(abs(dirC.z), 1e-6);

    float ndcPxX = fwidth(v_ndc.x);
    float ndcPxY = fwidth(v_ndc.y);

    float invFx = 1.0/projection[0][0];
    float invFy = 1.0/projection[1][1];

    float wpp_center = max(tC*invFx*ndcPxX, tC*invFy*ndcPxY);

    float gradX = length(vec2(dFdx(Plocal.x), dFdy(Plocal.x)));
    float gradY = length(vec2(dFdx(Plocal.y), dFdy(Plocal.y)));

    float aaX = max(gradX, 1e-6);
    float aaY = max(gradY, 1e-6);

    // Continuous L in log2 cell-size space :
    float cell_ref = wpp_center*TARGET_PIX;
    float L = log2(max(cell_ref, 1e-12));

    int i0 = int(floor(L)) - 2;

    float cov_sum = 0.0, wsum = 0.0;
    for (int j = 0; j < 5; ++j)
    {
        int i = i0 + j;
        float cell = exp2(float(i));
        float d = L - float(i);
        float wj = exp(-0.5*d*d);
        float covj = cov_for_cell(cell, Plocal, 0.5*wpp_center, aaX, aaY);
        cov_sum += wj*covj;
        wsum += wj;
    }

    float cov = cov_sum/max(wsum, 1e-12);
    float fade = 1.0 - smoothstep(0.0, max(fade_end_dist, 1e-6), length(Plocal.xy));
    float alpha = cov*fade;
    if (alpha <= 0.001) 
        discard;

    frag_col = vec4(vec3(0.5), alpha);
}
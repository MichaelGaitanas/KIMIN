#version 450 core

in vec4 frag_pos_light;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.

uniform vec3 mesh_col; //Mesh color.
uniform vec3 light_dir; //Direction of the light in world coordinates.
uniform sampler2D sample_shadow; //Depth image texture, obtained by the other shader.

//Decide whether or not the fragment is in shadow.
float get_shadow(float diffuse)
{
    vec3 projected_coords = frag_pos_light.xyz/frag_pos_light.w; //Perspective division to transform each fragment's position (with respect to light) in NDC, i.e. in [-1, 1].
    projected_coords = 0.5f*projected_coords + vec3(0.5f); //Transformation from [-1, 1] to [0, 1]. This is required to correctly access the shadow map texture, because internally, the UVs range in [0, 1].
    
    //For any fragment that is outside the orthographic frustum, don't calculate shadow.
    if (projected_coords.x < 0.0f || projected_coords.x > 1.0f ||
        projected_coords.y < 0.0f || projected_coords.y > 1.0f ||
        projected_coords.z > 1.0f)
    {
        return 0.0f; //Not shadow. Fully lit.
    }

    //Shadow acne fix : Balance between shadow acne (self-shadowing) and Peter-shitty-Panning.
    float min_bias = 0.0005f, amplifier = 0.005f;
    float bias = max(amplifier*(1.0f - diffuse), min_bias);

    float nearest_frag_depth = texture(sample_shadow, projected_coords.xy).r; //Sample depth from the shadow map.
    if (projected_coords.z - bias > nearest_frag_depth)
        return 0.95f; //In shadow.
    return 0.0f; //Not in shadow.
}

void main()
{
    //Ambient color component.
    float ambient = 0.05f;

    //Diffuse + shadow color component.
    float diffuse = max(dot(normalize(normal), light_dir), 0.0f);
    
    frag_col = vec4(vec3(ambient + (1.0f - get_shadow(diffuse))*diffuse)*mesh_col, 1.0f);
}

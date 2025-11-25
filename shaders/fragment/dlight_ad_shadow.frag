#version 450 core

#define POISSON_SAMPLES 16

in vec3 frag_pos_world;
in vec4 frag_pos_light;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.

uniform vec3 mesh_col; //Mesh color.
uniform vec3 light_dir; //Direction of the light in world coordinates.

uniform sampler2D sample_shadow; //Depth image texture, obtained by the other shader.

//Hard-coded Poisson disk sampling offsets, used for smoothing the shadow edges (pcf).
const vec2 poisson_disk[POISSON_SAMPLES] = { vec2(-0.94201624, -0.39906216), 
                                             vec2( 0.94558609, -0.76890725), 
                                             vec2(-0.09418410, -0.92938870), 
                                             vec2( 0.34495938,  0.29387760), 
                                             vec2(-0.91588581,  0.45771432), 
                                             vec2(-0.81544232, -0.87912464), 
                                             vec2(-0.38277543,  0.27676845), 
                                             vec2( 0.97484398,  0.75648379), 
                                             vec2( 0.44323325, -0.97511554), 
                                             vec2( 0.53742981, -0.47373420), 
                                             vec2(-0.26496911, -0.41893023), 
                                             vec2( 0.79197514,  0.19090188), 
                                             vec2(-0.24188840,  0.99706507), 
                                             vec2(-0.81409955,  0.91437590), 
                                             vec2( 0.19984126,  0.78641367), 
                                             vec2( 0.14383161, -0.14100790)  };

//Decide whether or not the fragment is in shadow.
float get_shadow(float diffuse)
{
    vec3 projected_coords = frag_pos_light.xyz/frag_pos_light.w; //Perspective division to transform each fragment's position (with respect to light) in NDC, i.e. in [-1, 1].
    projected_coords = 0.5*projected_coords + vec3(0.5); //Transformation from [-1, 1] to [0, 1]. This is required to correctly access the shadow map texture, because internally, the UVs range in [0, 1].
    
    //For any fragment that is outside the orthographic frustum, don't calculate shadow.
    if (projected_coords.x < 0.0 || projected_coords.x > 1.0 ||
        projected_coords.y < 0.0 || projected_coords.y > 1.0 ||
        projected_coords.z > 1.0)
    {
        return 0.0; //Not in shadow. Fully lit.
    }

    //Shadow test + percentage closer filtering (pcf) with Poisson sampling + pseudo-random jittering : What we do is that we sample the
    //shadow map's texture coords (x,y) like common UVs, which contain the nearest fragment depth (red channel only coz the map has grayscale
    //values only). Then we compare this depth to the current fragment's depth (projected_coords.z), in order to decide if the fragment is in
    //shadow or not. This algorithm calculates the shadow but has 2 problems : 1) Shadow acne (see below), 2) Sharp shadow edges (see below).
    //We try to fix the acne via depth bias and the sharp edges via a smoothing algorithm.

    //Shadow acne fix : Balance between shadow acne (self-shadowing) and Peter-shitty-Panning.
    float min_bias = 0.0003, amplifier = 0.003;
    float bias = max(amplifier*(1.0 - diffuse), min_bias);

    vec2 texel_size = 1.0/textureSize(sample_shadow, 0);
    float shadow = 0.0; //Accumulator.
    for (int i = 0; i < POISSON_SAMPLES; ++i)
    {
        vec2 offset = texel_size*poisson_disk[i]; //First offset : Poisson distro.
        vec2 random_offset = (fract(sin(dot(frag_pos_world.xy, vec2(12.9898, 78.233)))*43758.5453))*texel_size*0.5; //Second offset : Pseudo-RNG.
        float nearest_frag_depth = texture(sample_shadow, projected_coords.xy + offset + random_offset).r; //Don't sample from projected_coords.xy, but slightly from a different position.
        if (projected_coords.z - bias > nearest_frag_depth)
            shadow += 1.0;
    }
    return shadow/POISSON_SAMPLES; //Return the averaged shadow factor (over the number of samples in the for loop).
}

void main()
{
    //Ambient color component.
    float ambient = 0.05;

    float diffuse = max(dot(normal, light_dir), 0.0);
    frag_col = vec4(vec3(ambient + (1.0 - get_shadow(diffuse))*diffuse)*mesh_col, 1.0);
}

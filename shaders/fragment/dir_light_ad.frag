#version 450 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.



vec3 mesh_col = vec3(0.7f,0.7f,0.7f); //Gray asteroid color.
vec3 light_dir = vec3(1.0f,1.0f,1.0f); //Direction of the (directional) light in world coordinates.

void main()
{
    //Ambient color component.
    float ambient = 0.15f;

    //Diffuse color component.
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir);
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);
    
    frag_col = vec4((ambient + diffuse)*mesh_col, 1.0f);
}
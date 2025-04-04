#version 450 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.



uniform vec3 mesh_col; //Mesh color.
uniform vec3 light_dir; //Direction of the light in world coordinates.

void main()
{
    //Ambient color component.
    float ambient = 0.1f;

    //Diffuse color component.
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir);
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);

    frag_col = vec4((ambient + diffuse)*mesh_col, 1.0f);
}

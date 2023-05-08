#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 final_col; //color of the fragment after lighting calculations

uniform vec3 light_dir; //direction of the light in world coordinates
uniform vec3 light_col; //light color
uniform vec3 model_col; //model color

void main()
{
    //ambient color component
    float ambient = 0.15f;

    //diffuse color component
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir); //light direction with respect to model coordinates
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);

    vec3 frag_col = (ambient + diffuse)*model_col*light_col;
    final_col = vec4(frag_col, 1.0f);
}

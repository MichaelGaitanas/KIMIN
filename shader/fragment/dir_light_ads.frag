#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 final_col; //color of the fragment after lighting calculations

uniform vec3 light_dir; //direction of the light in world coordinates
uniform vec3 light_col; //light color
uniform vec3 model_col; //model color
uniform vec3 cam_pos; //position of the camera in world coordinates

void main()
{    
    //ambient color component
    float ambient = 0.15f;
    
    //diffuse color component
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir); //light direction with respect to model coordinates
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);
    
    //specular color component
    vec3 view_dir = normalize(cam_pos - frag_pos); //camera direction with respect to model coordinates
    vec3 reflect_dir = reflect(-light_dir_norm,norm); //ray's reflection direction with respect to model coordinates
    float specular = 0.5f*pow(max(dot(view_dir, reflect_dir),0.0f), 128);
    
    vec3 frag_col = (ambient + diffuse + specular)*model_col*light_col;
    final_col = vec4(frag_col, 1.0f);
}






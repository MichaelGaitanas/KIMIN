#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 final_col; //color of the fragment after lighting calculations

uniform vec3 light_pos; //position of the light in world coordinates
uniform vec3 light_col; //light color
uniform vec3 model_col; //model color
uniform vec3 cam_pos; //position of the camera in world coordinates

void main()
{    
    //ambient color component
    float ambient = 0.15f;
    
    //diffuse color component
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos); //light direction with respect to model coordinates
    float diffuse = max(dot(norm, light_dir), 0.0f);
    
    //specular color component
    vec3 view_dir = normalize(cam_pos - frag_pos); //camera direction with respect to model coordinates
    vec3 reflect_dir = reflect(-light_dir, norm); //ray's reflection direction with respect to model coordinates
    float specular = 0.5f*pow(max(dot(view_dir, reflect_dir),0.0f), 128);
    
    //attenuation factor
    float light_dist = length(light_pos - frag_pos); //distance between point light source and fragment
    float k1 = 1.0f, k2 = 0.09f, k3 = 0.032f; //constant (k1), linear (k2) and quadratic (k3) attenuation parameters
    float atten = 1.0f/( k1 + k2*light_dist + k3*light_dist*light_dist );
    
    vec3 frag_col = (ambient + diffuse + specular)*model_col*light_col*atten;
    final_col = vec4(frag_col, 1.0f);
}

/* This class contains the core geometrical calculations of the directional light used to render the simulation in 3D. */

#ifndef DLIGHT_H
#define DLIGHT_H

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class dlight
{
public:
    glm::mat4 pv; //projection*view matrix of the directional light.

    dlight() : pv(glm::mat4(0.0f))
    { }

    //This function computes the light's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h.
    void set_geometry(const float ortho_extent, const glm::vec3 &light_dir)
    {
        const float fc = 1.01f, fl = 1.02f; //Scale factors that are used to scale the size of the orthographic box of the directional light.
        const float light_dist = fl*ortho_extent; //Directional light's 'dummy' distance. Note : this is NOT the real Sun's distance.
 
        const glm::vec3 up = (glm::abs(light_dir.z) > 0.999f) ? glm::vec3(0.0f,1.0f,0.0f) : glm::vec3(0.0f,0.0f,1.0f);

        const glm::mat4 projection = glm::ortho(-fc*ortho_extent,fc*ortho_extent,
                                                -fc*ortho_extent,fc*ortho_extent,
                                                (fl-fc)*ortho_extent, 2.0f*fc*ortho_extent);
        const glm::mat4 view = glm::lookAt(light_dist*light_dir, glm::vec3(0.0f), up);
        pv = projection*view;
    }
};

#endif
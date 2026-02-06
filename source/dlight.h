/* This class contains the core geometrical calculations of the 'directional light' used to render the simulation in 3D. */

#ifndef DLIGHT_H
#define DLIGHT_H

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class dlight
{
public:
    glm::vec3 pos, dir;
    float dist;
    glm::mat4 pv; //projection*view matrix of the directional light.

    dlight() : pos(glm::vec3(0.0f)),
               dir(glm::vec3(0.0f)),
               dist(0.0f),
               pv(glm::mat4(0.0f))
    { }

    //This function computes the light's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h.
    void set_geometry(const float ortho_size, const glm::vec3 &pos)
    {
        this->pos = pos; //Copy Sun's TRUE position vector to the member.
        dist = glm::length(pos);
        dir = pos/dist;

        const float fc = 1.01f, fl = 1.02f; //Scale factors that are used to scale the size of the orthographic box of the directional light.
        const float dummy_dist = fl*ortho_size; //Directional light's 'dummy' distance. Note : this is NOT the real Sun's distance!
 
        glm::vec3 up = glm::vec3(0.0f,0.0f,1.0f);
        if (glm::abs(dir.z) > 0.999f)
            up = glm::vec3(0.0f,1.0f,0.0f);

        const glm::mat4 projection = glm::ortho(    -fc*ortho_size,      fc*ortho_size,
                                                    -fc*ortho_size,      fc*ortho_size,
                                                (fl-fc)*ortho_size, 2.0f*fc*ortho_size);
        const glm::mat4 view = glm::lookAt(dummy_dist*dir, glm::vec3(0.0f), up);
        pv = projection*view;
    }
};

#endif
#ifndef LIGHT_H
#define LIGHT_H

#include<cmath>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class light
{
private:
    float dist, fc, fl; //These determine the size of the orthographic box of the directional light.
    glm::vec3 up;
    glm::mat4 projection, view;

public:
    float lon, lat;
    glm::vec3 dir;
    glm::mat4 pv; //projection*view (premultiplied to avoid doing it in the GPU per vertex).

    light() : dist(0.0f),
              fc(1.1f),
              fl(1.2f),
              up(glm::vec3(0.0f)),
              projection(glm::mat4(0.0f)),
              view(glm::mat4(0.0f)),
              lon(0.0f),
              lat(90.0f),
              dir(glm::vec3(0.0f)),
              pv(glm::mat4(0.0f))
    { }

    //This function runs one time after every simulation termination.
    void reset(const float dist_sum)
    {
        dist = fl*dist_sum; //Directional light's 'dummy' distance.
        projection = glm::ortho(-fc*dist,fc*dist, -fc*dist,fc*dist, (fl-fc)*dist, 2.0f*fc*dist); //Directional light's projection matrix.
    }

    //This function computes the light values of the variables that are passed as uniforms to the shaders in the render_3D_content().
    void set_geometry()
    {
        //The user controls the light's direction from the gui, assuming spherical coords (longitude and latitude).
        //So here we convert it back to Cartesian coords and send the vector to the fragment shader to evaluate the shadow.
        //Note : glm::normalize() is not necessary, but let it be deffensive for now.
        dir = glm::normalize(glm::vec3(cos(glm::radians(lon))*sin(glm::radians(lat)),
                                       sin(glm::radians(lon))*sin(glm::radians(lat)),
                                       cos(glm::radians(lat))));

        up = (glm::abs(dir).z > 0.999f) ? glm::vec3(0.0f,1.0f,0.0f) : glm::vec3(0.0f,0.0f,1.0f);
        view = glm::lookAt(dist*dir, glm::vec3(0.0f), up);
        pv = projection*view;
    }

};

#endif
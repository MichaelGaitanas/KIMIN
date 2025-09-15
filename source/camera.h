#ifndef CAMERA_H
#define CAMERA_H

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class camera
{
public:
    float dist, lon, lat, fov, min_dist, max_dist;
    glm::vec3 pos, aim, up;
    glm::mat4 projection, view;

    camera() : dist(0.0f),
               lon(40.0f),
               lat(60.0f),
               fov(60.0f),
               aim(glm::vec3(0.0f)),
               min_dist(0.0f),
               max_dist(0.0f)
    { }

    //This function runs one time after every simulation termination.
    void reset(const float brillouin_radii_sum, const float binary_max_dist)
    {
        min_dist = 1.1f*brillouin_radii_sum;
        max_dist = 40.0f*binary_max_dist;
        dist = min_dist + 0.1f*(max_dist - min_dist);
    }

    //This function computes the camera values of the variables that are passed as uniforms to the shaders in the render_3D_content().
    void set_geometry(const float win_aspect_ratio)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f); //<<<<<<<<<<<< I need to fix the hard-coded 0.1f...

        //Spherical to Cartesian...
        pos = dist*glm::vec3(cos(glm::radians(lon))*sin(glm::radians(lat)),
                             sin(glm::radians(lon))*sin(glm::radians(lat)),
                             cos(glm::radians(lat)));
        
        //The up vector is equal to the minus unit latitude basis vector (expressed as a function of the cartesian unit vectors) : up = -hat(θ(hat(x),hat(y),hat(z))).
        up = -glm::vec3(cos(glm::radians(lat))*cos(glm::radians(lon)),
                        cos(glm::radians(lat))*sin(glm::radians(lon)),
                       -sin(glm::radians(lat)));

        view = glm::lookAt(pos, aim, up);
    }
};


#endif
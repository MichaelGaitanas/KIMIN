#ifndef CAMERA_H
#define CAMERA_H

#include<cmath>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class camera
{
public:
    float dist, lon, lat, fov, min_dist, max_dist, min_fov, max_fov;
    glm::vec3 pos, aim, up;
    glm::mat4 projection, view;
    bool lock_revo;

    camera() : dist(0.0f),
               lon(40.0f),
               lat(60.0f),
               fov(60.0f),
               aim(glm::vec3(0.0f)),
               min_dist(0.0f),
               max_dist(0.0f),
               min_fov(1.0f),
               max_fov(179.0f),
               lock_revo(false)
    { }

    //This function runs one time after every simulation termination.
    void reset(const float brillouin_radii_sum, const float binary_max_dist)
    {
        min_dist = 1.1f*brillouin_radii_sum;
        max_dist = 40.0f*binary_max_dist;
        dist = min_dist + 0.1f*(max_dist - min_dist);
    }

    //This function alters the camera's 'dist' memeber, based on how much the user scrolled the mouse wheel since the last frame.
    void scroll_dist(const float mouse_delta_wheel)
    {
        dist *= pow(0.9f, mouse_delta_wheel);
        if (dist < min_dist) dist = min_dist;
        else if (dist > max_dist) dist = max_dist;
    }

    //This function alters the camera's 'fov' memeber, based on how much the user scrolled the mouse wheel + ctrl key since the last frame.
    void scroll_fov(const float mouse_delta_wheel)
    {
        fov -= mouse_delta_wheel;
        if (fov <= min_fov) fov = min_fov;
        else if (fov >= max_fov) fov = max_fov;
    }

    void rotate_lon_lat(const float dx, const float dy, const float mouse_sensitivity = 0.3f)
    {
        lon = fmod(lon - dx*mouse_sensitivity, 360.0f);
        if (lon < 0.0f) lon += 360.0f;

        lat -= dy*mouse_sensitivity;
        if (lat < 0.04f) lat = 0.04f;
        if (lat > 179.96f) lat = 179.96f;
    }

    //This function computes the camera values of the variables that are passed as uniforms to the shaders in the render_3D_content().
    void set_geometry_inertial(const float win_aspect_ratio)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f); //<<<<<<<<<<<< I need to fix the hard-coded 0.1f...

        //Spherical to Cartesian.
        pos = dist*glm::vec3(cos(glm::radians(lon))*sin(glm::radians(lat)),
                             sin(glm::radians(lon))*sin(glm::radians(lat)),
                             cos(glm::radians(lat)));
        
        //The up vector is equal to the minus unit latitude basis vector, but expressed as a function of the Cartesian unit vectors : up = -hat(θ(hat(x),hat(y),hat(z))).
        up = -glm::vec3(cos(glm::radians(lat))*cos(glm::radians(lon)),
                        cos(glm::radians(lat))*sin(glm::radians(lon)),
                       -sin(glm::radians(lat)));

        view = glm::lookAt(pos, aim, up);
    }

    void set_geometry_body(const float win_aspect_ratio, const glm::vec3 &body_pos, const float brillouin)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f);

        float rb = glm::length(body_pos);
        float xc = (rb + 3.0f*brillouin)*body_pos.x/rb;
        float yc = (rb + 3.0f*brillouin)*body_pos.y/rb;
        float zc = (rb + 3.0f*brillouin)*body_pos.z/rb;
        pos = glm::vec3(xc,yc,zc);

        

        up = glm::vec3(0.0f,0.0f,1.0f);

        view = glm::lookAt(pos, aim, up);
    }
};


#endif
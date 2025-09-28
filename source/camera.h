#ifndef CAMERA_H
#define CAMERA_H

#include<cmath>

#include"constant.h"
#include"typedef.h"
#include"conversion.h"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class camera
{
public:
    float dist, lon, lat, fov, min_dist, max_dist, min_fov, max_fov;
    glm::vec3 pos, aim, up;
    glm::mat4 projection, view;
    bool mount_body1, mount_body2;
    float rscale, vscale;
    glm::vec2 voffset_ndc;

    camera() : dist(0.0f),
               lon(40.0f),
               lat(60.0f),
               fov(60.0f),
               min_dist(0.0f),
               max_dist(0.0f),
               min_fov(1.0f),
               max_fov(179.0f),
               pos(glm::vec3(0.0f)),
               aim(glm::vec3(0.0f)),
               up(glm::vec3(0.0f)),
               mount_body1(false),
               mount_body2(false),
               rscale(5.0f),
               vscale(1.0f),
               voffset_ndc(glm::vec2(0.0f))
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
        lon = fmodf(lon - dx*mouse_sensitivity, 360.0f);
        if (lon < 0.0f) lon += 360.0f;

        lat -= dy*mouse_sensitivity;
        if (lat < 0.04f) lat = 0.04f;
        if (lat > 179.96f) lat = 179.96f;
    }

    //This function computes the camera values of the variables that are passed as uniforms to the shaders in the render_3D_content().
    void set_geometry_inertial(const float win_aspect_ratio)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f);

        //Spherical to Cartesian.
        pos = dist*glm::vec3(cos(glm::radians(lon))*sin(glm::radians(lat)),
                             sin(glm::radians(lon))*sin(glm::radians(lat)),
                             cos(glm::radians(lat)));

        aim = glm::vec3(0.0f);
        
        //The up vector is equal to the minus unit latitude basis vector, but expressed as a function of the Cartesian unit vectors : up = -hat(θ(hat(x),hat(y),hat(z))).
        up = -glm::vec3(cos(glm::radians(lat))*cos(glm::radians(lon)),
                        cos(glm::radians(lat))*sin(glm::radians(lon)),
                       -sin(glm::radians(lat)));

        view = glm::lookAt(pos, aim, up);
    }

    void set_geometry_body(const float win_aspect_ratio, const glm::vec3 &pos_body, const glm::vec3 &pos_other_body, const float brillouin)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f);

        aim = pos_other_body; //Aim is always "the other body".

        glm::vec3 pos_base = ( 1.0f + rscale*brillouin/glm::length(pos_body) )*pos_body;

        dvec3 spher = cart2spher(dvec3{pos_base.x, pos_base.y, pos_base.z});
        dist = (float)spher[0];
        lon  = (float)wrap_to_2pi(spher[1])*180.0f/pi;
        lat  = (float)spher[2]*180.0f/pi;

        up = -glm::vec3(cos(glm::radians(lat))*cos(glm::radians(lon)),
                        cos(glm::radians(lat))*sin(glm::radians(lon)),
                       -sin(glm::radians(lat)));
        glm::vec3 front = glm::normalize(aim - pos_base);
        glm::vec3 right = glm::cross(front, up);

        //Apply joystick V-offset on the (right - up) plane.
        //The ImGui quad currently maps top to -1 and bottom to +1, so flip y.
        glm::vec2 v = glm::vec2(voffset_ndc.x, -voffset_ndc.y);
        glm::vec3 offset = (v.x*vscale*brillouin)*right + (v.y*vscale*brillouin)*up;

        pos = pos_base + offset; //Final camera's position.

        view = glm::lookAt(pos, aim, up);
    }
};


#endif
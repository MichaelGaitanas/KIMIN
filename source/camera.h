/* This class contains the core geometrical calculations of the camera used to render the simulation in 3D. */

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
private:
    glm::vec3 pos, aim, up;

public:
    glm::mat4 projection, view;
    float dist, lon, lat, fov, min_dist, max_dist;    
    bool mount_body1, mount_body2;
    float rscale, vscale;
    glm::vec2 voffset_ndc;

    camera() : pos(glm::vec3(0.0f)),
               aim(glm::vec3(0.0f)),
               up(glm::vec3(0.0f)),
               projection(glm::mat4(0.0f)),
               view(glm::mat4(0.0f)),
               dist(0.0f),
               lon(40.0f),
               lat(60.0f),
               fov(60.0f),
               min_dist(0.0f),
               max_dist(0.0f),
               mount_body1(false),
               mount_body2(false),
               rscale(5.0f),
               vscale(1.0f),
               voffset_ndc(glm::vec2(0.0f))
    { }

    //This function runs one time after every simulation termination. It resets some of the members, depending on the scales (sizes) of the new simulation.
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

    //This function alters the camera's 'fov' memeber, based on how much the user scrolled the mouse wheel (+ ctrl key) since the last frame.
    void scroll_fov(const float mouse_delta_wheel)
    {
        fov -= mouse_delta_wheel;
        if (fov <= 1.0f) fov = 1.0f;
        else if (fov >= 179.0f) fov = 179.0f;
    }

    void rotate_lon_lat(const float dx, const float dy, const float mouse_sensitivity = 0.3f)
    {
        lon = fmodf(lon - dx*mouse_sensitivity, 360.0f);
        if (lon < 0.0f) lon += 360.0f;

        lat -= dy*mouse_sensitivity;
        if (lat < 0.04f) lat = 0.04f;
        else if (lat > 179.96f) lat = 179.96f;
    }

private:
    //The 'up' vector is equal to the minus unit latitude basis vector, but expressed as a function of the Cartesian unit vectors : up = -hat(θ(hat(x),hat(y),hat(z))).
    glm::vec3 get_up_vector()
    {
        return -glm::vec3(cos(glm::radians(lat))*cos(glm::radians(lon)),
                          cos(glm::radians(lat))*sin(glm::radians(lon)),
                         -sin(glm::radians(lat)));
    }

public:
    //This function computes the camera values of the variables that are passed as uniforms to the shaders in the render_3D_content(), but when the camera is in inertial frame mode.
    void set_geometry_inertial(const float win_aspect_ratio)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f); //I need to fix that...

        //Spherical to Cartesian.
        pos = dist*glm::vec3(cos(glm::radians(lon))*sin(glm::radians(lat)),
                             sin(glm::radians(lon))*sin(glm::radians(lat)),
                             cos(glm::radians(lat)));
        aim = glm::vec3(0.0f);
        up = get_up_vector();
        view = glm::lookAt(pos, aim, up);
    }

    //This function computes the camera values of the variables that are passed as uniforms to the shaders in the render_3D_content(), but when the camera is in mount/body frame mode.
    void set_geometry_body(const float win_aspect_ratio, const glm::vec3 &pos_body, const glm::vec3 &pos_other_body, const float brillouin)
    {
        projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f); //I need to fix that...

        aim = pos_other_body; //Camera aims always at "the other body".

        glm::vec3 pos_base = ( 1.0f + rscale*brillouin/glm::length(pos_body) )*pos_body;

        //Construct a right-handed Cartesian basis.
        up = get_up_vector();
        glm::vec3 front = glm::normalize(aim - pos_base);
        glm::vec3 right = glm::cross(front, up);

        //Apply joystick V-offset on the (right - up) plane. Note : the ImGui quad currently maps top to -1 and bottom to +1, so we flip y.
        glm::vec3 voffset = (voffset_ndc.x*vscale*brillouin)*right + (-voffset_ndc.y*vscale*brillouin)*up;

        pos = pos_base + voffset; //Final camera's position.

        view = glm::lookAt(pos, aim, up);

        //One thing remains : since the user cannot update the members 'dist', 'lon', 'lat', we do it manually so that they render correctly in the GUI.
        dvec3 spher = cart2spher(dvec3{pos_base.x, pos_base.y, pos_base.z});
        dist = static_cast<float>(spher[0]);
        lon  = static_cast<float>(wrap_to_2pi(spher[1])*180.0/pi);
        lat  = static_cast<float>(spher[2]*180.0/pi);
    }
};


#endif
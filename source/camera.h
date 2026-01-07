/* This class contains the geometrical calculations of the camera that are used to view the simulation in 3D. */

#ifndef CAMERA_H
#define CAMERA_H

#include<cmath>
#include<algorithm>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include"constants.h"

class camera
{
private:
    glm::vec3 pos, aim, up;

public:
    glm::mat4 projection, view;
    float dist, lon, lat, min_dist, max_dist;
    float fov;

    enum
    {
        MODE_COM,
        MODE_BODY1,
        MODE_BODY2
    } mode;

    camera() : pos(glm::vec3(0.0f)),
               aim(glm::vec3(0.0f)),
               up(glm::vec3(0.0f)),
               projection(glm::mat4(0.0f)),
               view(glm::mat4(0.0f)),
               dist(0.0f),
               lon(270.0f),
               lat(45.0f),
               min_dist(0.0f),
               max_dist(0.0f),
               fov(75.0f),
               mode(MODE_COM)
    { }

public:
    //This function computes the camera's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h.
    void set_geometry(const float aspect, const glm::vec3 &pivot)
    {
        const float lon_rad = glm::radians(lon);
        const float lat_rad = glm::radians(lat);
        //Spherical to Cartesian :
        pos = pivot + dist*glm::vec3(cos(lon_rad)*sin(lat_rad),
                                     sin(lon_rad)*sin(lat_rad),
                                     cos(lat_rad));
        aim = pivot;
        //Our up direction is the unit θhat vector of spherical coordinates :
        up = -glm::vec3(cos(lon_rad)*cos(lat_rad),
                        sin(lon_rad)*cos(lat_rad),
                       -sin(lat_rad));

        view = glm::lookAt(pos, aim, up);
        projection = glm::infinitePerspective(glm::radians(fov), aspect, std::max(0.001f*dist, 0.1f));
    }

    //This function runs once each time a simulation terminates. It resets min, max and current camera distance, depending on the scales of the simulation that just ran.
    void reset(const float brillouin_radii_sum, const float binary_max_dist)
    {
        min_dist = 1.1f*brillouin_radii_sum;
        max_dist = 40.0f*binary_max_dist;
        dist = min_dist + 0.1f*(max_dist - min_dist);
    }

    //This function is basically a 'zoom in/out' protocol. It alters the camera's fov, based on how much the user scrolled the mouse wheel (+ ctrl key) since the last frame.
    void zoom(const float mouse_delta_wheel)
    {
        fov -= mouse_delta_wheel;
        if (fov <= CAM_MIN_FOV)
            fov = CAM_MIN_FOV;
        else if (fov >= CAM_MAX_FOV)
            fov = CAM_MAX_FOV;
    }

    //This function changes the camera's distance from the origin, based on how much the user scrolled the mouse wheel since the last frame.
    void translate(const float mouse_delta_wheel)
    {
        dist *= pow(0.9f, mouse_delta_wheel);
        if (dist < min_dist)
            dist = min_dist;
        else if (dist > max_dist)
            dist = max_dist;
    }

    //This function changes the camera's angles (longitude & latitude), based on how much the user moved the mouse (+ middle button) since the last frame.
    void rotate(const float dx, const float dy, const float mouse_sensitivity = 0.3f)
    {
        lon = fmodf(lon - dx*mouse_sensitivity, 360.0f);
        if (lon < 0.0f)
            lon += 360.0f;

        lat -= dy*mouse_sensitivity;
        if (lat < 0.04f)
            lat = 0.04f;
        else if (lat > 179.96f)
            lat = 179.96f;
    }
};

#endif
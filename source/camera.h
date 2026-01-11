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

    enum
    {
        MODE_COM,
        MODE_BODY1,
        MODE_BODY2
    } mode;

    //Per-mode spherical coordinates and limits :
    float dist_com, lon_com, lat_com, min_dist_com, max_dist_com;
    float dist_b1,  lon_b1,  lat_b1,  min_dist_b1,  max_dist_b1;
    float dist_b2,  lon_b2,  lat_b2,  min_dist_b2,  max_dist_b2;

    float fov;

    camera() : pos(glm::vec3(0.0f)),
               aim(glm::vec3(0.0f)),
               up(glm::vec3(0.0f)),
               projection(glm::mat4(0.0f)),
               view(glm::mat4(0.0f)),
               mode(MODE_COM),
               dist_com(0.0f), lon_com(270.0f), lat_com(45.0f), min_dist_com(0.0f), max_dist_com(0.0f),
               dist_b1(0.0f),  lon_b1(270.0f),  lat_b1(45.0f),  min_dist_b1(0.0f),  max_dist_b1(0.0f),
               dist_b2(0.0f),  lon_b2(270.0f),  lat_b2(45.0f),  min_dist_b2(0.0f),  max_dist_b2(0.0f),
               fov(75.0f)
    { }

/*
private:
    glm::vec3 get_up_vector()
    {
        glm::vec3 world_up(0.0f, 0.0f, 1.0f);
        //return world_up;

        glm::vec3 front = glm::normalize(aim - pos);

        //Project world_up onto plane perpendicular to front:
        glm::vec3 up_proj = world_up - front*glm::dot(world_up, front);
        if (glm::dot(up_proj, up_proj) < 1e-8)
        {
            //We are extremely close to the poles. Keep continuity using the last 'up'.
            return up;
        }

        return glm::normalize(up_proj);
    }
*/

public:
    inline float &get_active_dist()
    {
        if (mode == MODE_COM)
            return dist_com;
        if (mode == MODE_BODY1)
            return dist_b1;
        return dist_b2;
    }

    inline float &get_active_lon()
    {
        if (mode == MODE_COM)
            return lon_com;
        if (mode == MODE_BODY1)
            return lon_b1;
        return lon_b2;
    }

    inline float &get_active_lat()
    {
        if (mode == MODE_COM)
            return lat_com;
        if (mode == MODE_BODY1)
            return lat_b1;
        return lat_b2;
    }

    inline float &get_active_min_dist()
    {
        if (mode == MODE_COM)
            return min_dist_com;
        if (mode == MODE_BODY1)
            return min_dist_b1;
        return min_dist_b2;
    }

    inline float &get_active_max_dist()
    {
        if (mode == MODE_COM)
            return max_dist_com;
        if (mode == MODE_BODY1)
            return max_dist_b1;
        return max_dist_b2;
    }

    //This function computes the camera's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h.
    void set_geometry(const float aspect, const glm::vec3 &pivot)
    {
        const float lon_act  = get_active_lon(); //[deg]
        const float lat_act  = get_active_lat(); //[deg]
        const float dist_act = get_active_dist();

        const float lon_rad = glm::radians(lon_act);
        const float lat_rad = glm::radians(lat_act);
        //Spherical to Cartesian :
        pos = pivot + dist_act*glm::vec3(cos(lon_rad)*sin(lat_rad),
                                         sin(lon_rad)*sin(lat_rad),
                                         cos(lat_rad));
        aim = pivot;
        //Our up direction is the unit θhat vector of spherical coordinates :
        up = -glm::vec3(cos(lon_rad)*cos(lat_rad),
                        sin(lon_rad)*cos(lat_rad),
                       -sin(lat_rad));
        //up = get_up_vector();

        view = glm::lookAt(pos, aim, up);
        projection = glm::infinitePerspective(glm::radians(fov), aspect, std::max(0.001f*dist_act, 0.1f));
    }

    //This function runs once each time a simulation terminates. It resets min, max and current camera distance, depending on the scales of the simulation that just ran.
    void reset(const float brillouin1, const float brillouin2, const float binary_max_dist)
    {
        min_dist_com = 1.1f*(brillouin1 + brillouin2);
        max_dist_com = 40.0f*binary_max_dist;
        dist_com = min_dist_com + 0.1f*(max_dist_com - min_dist_com);

        min_dist_b1 = 1.1f*brillouin1;
        max_dist_b1 = 100.0f*brillouin1;
        dist_b1 = min_dist_b1 + 0.1f*(max_dist_b1 - min_dist_b1);

        min_dist_b2 = 1.1f*brillouin2;
        max_dist_b2 = 100.0f*brillouin2;
        dist_b2 = min_dist_b2 + 0.1f*(max_dist_b2 - min_dist_b2);
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
        float &dist_act = get_active_dist();
        const float &min_dist_act = get_active_min_dist();
        const float &max_dist_act = get_active_max_dist();

        dist_act *= pow(0.9f, mouse_delta_wheel);
        if (dist_act < min_dist_act)
            dist_act = min_dist_act;
        else if (dist_act > max_dist_act)
            dist_act = max_dist_act;
    }

    //This function changes the camera's angles (longitude & latitude), based on how much the user moved the mouse (+ middle button) since the last frame.
    void rotate(const float dx, const float dy, const float mouse_sensitivity = 0.3f)
    {
        float &lon_act = get_active_lon(); //[deg]
        float &lat_act = get_active_lat(); //[deg]

        lon_act = fmodf(lon_act - dx*mouse_sensitivity, 360.0f);
        if (lon_act < 0.0f)
            lon_act += 360.0f;

        lat_act -= dy*mouse_sensitivity;
        if (lat_act < 0.04f)
            lat_act = 0.04f;
        else if (lat_act > 179.96f)
            lat_act = 179.96f;
    }
};

#endif
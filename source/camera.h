/* This class contains the geometrical calculations of the camera used to render the simulation in 3D. */

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
    float dist_world, lon_world, lat_world;
    float dist_com, lon_com, lat_com;
    float min_dist, max_dist;
    float rscale, vscale;
    glm::vec2 voffset_ndc;
    float fov;
    
    enum
    {
        WORLD,
        COM,
        BODY1,
        BODY2
    } frame_of_ref;

    camera() : pos(glm::vec3(0.0f)),
               aim(glm::vec3(0.0f)),
               up(glm::vec3(0.0f)),
               projection(glm::mat4(0.0f)),
               view(glm::mat4(0.0f)),
               dist_world(0.0f),
               lon_world(40.0f),
               lat_world(60.0f),
               dist_com(0.0f),
               lon_com(40.0f),
               lat_com(60.0f),
               min_dist(0.0f),
               max_dist(0.0f),
               rscale(5.0f),
               vscale(1.0f),
               voffset_ndc(glm::vec2(0.0f)),
               fov(CAM_INIT_FOV),
               frame_of_ref(WORLD)
    { }

public:
    inline float &get_active_dist()
    {
        return (frame_of_ref == WORLD) ? dist_world : dist_com;
    }
    
    inline float &get_active_lon()
    {
        return (frame_of_ref == WORLD) ? lon_world : lon_com;
    }

    inline float &get_active_lat()
    {
        return (frame_of_ref == WORLD) ? lat_world : lat_com;
    }

    //This function computes the camera's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h for the cases of WORLD or COM frame.
    void set_geometry_inertial(const float aspect, const glm::vec3 &pos_com)
    {
        //Spherical to Cartesian.
        const float dist = get_active_dist();
        const float lon  = glm::radians(get_active_lon());
        const float lat  = glm::radians(get_active_lat());
        pos = pos_com + dist*glm::vec3(cos(lon)*sin(lat),
                                       sin(lon)*sin(lat),
                                       cos(lat));
        aim = pos_com;
        up = -glm::vec3(cos(lon)*cos(lat),
                        sin(lon)*cos(lat),
                       -sin(lat));

        view = glm::lookAt(pos, aim, up);
        projection = glm::infinitePerspective(glm::radians(fov), aspect, std::max(0.001f*dist, 0.1f));
    }

    //This function computes the camera's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h for the cases of BODY1 or BODY2 frame.
    void set_geometry_body(const float aspect, const glm::vec3 &pos_body, const glm::vec3 &pos_other_body, const float brillouin)
    {
        aim = pos_other_body; //Camera aims at 'the other body'.

        glm::vec3 pos_base = (1.0f + rscale*brillouin/glm::length(pos_body))*pos_body; //Possible division by zero?

        //Construct a local right-handed Cartesian basis.
        up = glm::vec3(0.0f,0.0f,1.0f); //I need to fix this...
        glm::vec3 front = glm::normalize(aim - pos_base);
        glm::vec3 right = glm::cross(front, up);

        //Apply joystick V-offset on the (right - up) plane. Note : the ImGui quad currently maps top to -1 and bottom to +1, so we flip y.
        glm::vec3 voffset = (voffset_ndc.x*vscale*brillouin)*right + (-voffset_ndc.y*vscale*brillouin)*up;

        pos = pos_base + voffset; //Final camera's mounted position.

        view = glm::lookAt(pos, aim, up);
        projection = glm::infinitePerspective(glm::radians(fov), aspect, std::max(0.01f*brillouin, 0.1f));
    }

    //This function runs once each time a simulation terminates. It resets some of the members, depending on the scales (sizes) of the simulation that just ran.
    void reset(const float brillouin_radii_sum, const float binary_max_dist)
    {
        min_dist = 1.1f*brillouin_radii_sum;
        max_dist = 40.0f*binary_max_dist;
        dist_world = dist_com = min_dist + 0.1f*(max_dist - min_dist);
    }

    //This function is basically a 'zoom in/out' protocol. It alters the camera's 'fov' member, based on how much the user scrolled the mouse wheel (+ ctrl key) since the last frame.
    void scroll_fov(const float mouse_delta_wheel)
    {
        fov -= mouse_delta_wheel;
        if (fov <= CAM_MIN_FOV) fov = CAM_MIN_FOV;
        else if (fov >= CAM_MAX_FOV) fov = CAM_MAX_FOV;
    }

    //This function alters the camera's active distance member (i.e. 'dist_world' or 'dist_com'), based on how much the user scrolled the mouse wheel since the last frame. It is called only when the camera is either in WORLD or COM frame mode.
    void scroll_dist_inertial(const float mouse_delta_wheel)
    {
        float &dist = get_active_dist();
        dist *= pow(0.9f, mouse_delta_wheel);
        if (dist < min_dist) dist = min_dist;
        else if (dist > max_dist) dist = max_dist;
    }

    //This function changes the camera's distance from a body, based on how much the user scrolled the mouse wheel since the last frame. It is called only when the camera is either in BODY1 or BODY2 frame mode.
    void scroll_dist_body(const float mouse_delta_wheel)
    {
        rscale -= 0.2f*mouse_delta_wheel;
        if (rscale < 3.0f) rscale = 3.0f;
        else if (rscale > 10.0f) rscale = 10.0f;
    }

    //When in WORLD or COM frame, this function alters the camera's longitude/latitude, based on how much the user moved the mouse (+ middle click) since the last frame.
    void rotate_lon_lat_inertial(const float dx, const float dy, const float mouse_sensitivity = 0.3f)
    {
        float &lon = get_active_lon();
        float &lat = get_active_lat();

        lon = fmodf(lon - dx*mouse_sensitivity, 360.0f);
        if (lon < 0.0f) lon += 360.0f;

        lat -= dy*mouse_sensitivity;
        if (lat < 0.04f) lat = 0.04f;
        else if (lat > 179.96f) lat = 179.96f;
    }

    //When in BODY1 or BODY2 frame, this function causes the camera to move upon the vertical plane with respect to the 'aim' vector, based on how much the user moved the mouse (+ middle click) since the last frame.
    void move_upon_vplane(const float dx, const float dy, const int win_width, const int win_height, const float mouse_sensitivity = 1.0f)
    {
        if (win_width <= 0 || win_height <= 0) return;

        float ndc_step = 2.0f/static_cast<float>(win_height);
        voffset_ndc.x -= dx*ndc_step*mouse_sensitivity;
        voffset_ndc.y -= dy*ndc_step*mouse_sensitivity;
        voffset_ndc.x = std::clamp(voffset_ndc.x, -1.0f, 1.0f);
        voffset_ndc.y = std::clamp(voffset_ndc.y, -1.0f, 1.0f);
    }
};


#endif
/* This class contains the core geometrical calculations of the directional light used to render the simulation in 3D. */

#ifndef DLIGHT_H
#define DLIGHT_H

#include<cmath>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class dlight
{
public:
    float lon, lat;
    glm::vec3 dir;
    glm::mat4 pv; //projection*view (premultiplied to avoid doing it in the GPU per vertex).
    bool is_constrained;

    dlight() : lon(0.0f),
               lat(90.0f),
               dir(glm::vec3(0.0f)),
               pv(glm::mat4(0.0f)),
               is_constrained(false)
    { }

    //This function alters the light's 'lon' and 'lat' members, based on how much the user moved the mouse (+ middle click + ctrl key) since the last frame.
    void rotate_lon_lat(const float dx, const float dy, const float mouse_sensitivity = 0.3f)
    {
        lon = fmodf(lon - dx*mouse_sensitivity, 360.0f);
        if (lon < 0.0f) lon += 360.0f;

        lat -= dy*mouse_sensitivity;
        if (lat < 0.04f) lat = 0.04f;
        else if (lat > 179.96f) lat = 179.96f;
    }

    //This function computes the light's projection and view matrix that are passed as uniforms to the shaders in the renderer3D.h.
    void set_geometry(const float shadow_extent, const glm::vec3 &pos_com)
    {
        const float fc = 1.01f, fl = 1.02f; //Scale factors that are used to scale the size of the orthographic box of the directional light.
        const float lon_rad = glm::radians(lon), lat_rad = glm::radians(lat);
        dir = glm::vec3(cos(lon_rad)*sin(lat_rad),
                        sin(lon_rad)*sin(lat_rad),
                        cos(lat_rad));
        const float light_dist = fl*shadow_extent; //Directional light's 'dummy' distance.

        const glm::vec3 up = (glm::abs(dir.z) > 0.999f) ? glm::vec3(0.0f,1.0f,0.0f) : glm::vec3(0.0f,0.0f,1.0f);

        const glm::mat4 projection = glm::ortho(-fc*shadow_extent,fc*shadow_extent,
                                                -fc*shadow_extent,fc*shadow_extent,
                                                (fl-fc)*shadow_extent, 2.0f*fc*shadow_extent);
        const glm::mat4 view = glm::lookAt(pos_com + light_dist*dir, pos_com, up);
        pv = projection*view;
    }
};

#endif
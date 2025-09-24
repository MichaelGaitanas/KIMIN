#ifndef CAMERA_H
#define CAMERA_H

#include<cmath>

#include"constant.h"
#include"typedef.h"

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
    float brillouin_scale;
    glm::vec2 v_offset_ndc;   // in [-1,1]^2 coming from the ImGui quad (x=right, y=up)
    float     v_offset_scale; // world scale factor (in "Brillouin radii" units)

    camera() : dist(0.0f),
               lon(40.0f),
               lat(60.0f),
               fov(60.0f),
               aim(glm::vec3(0.0f)),
               min_dist(0.0f),
               max_dist(0.0f),
               min_fov(1.0f),
               max_fov(179.0f),
               mount_body1(false),
               mount_body2(false),
               brillouin_scale(5.0f),
               v_offset_ndc(0.0f, 0.0f),
               v_offset_scale(1.0f)
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

        aim = glm::vec3(0.0f);
        
        //The up vector is equal to the minus unit latitude basis vector, but expressed as a function of the Cartesian unit vectors : up = -hat(θ(hat(x),hat(y),hat(z))).
        up = -glm::vec3(cos(glm::radians(lat))*cos(glm::radians(lon)),
                        cos(glm::radians(lat))*sin(glm::radians(lon)),
                       -sin(glm::radians(lat)));

        view = glm::lookAt(pos, aim, up);
    }

    void set_geometry_body(const float win_aspect_ratio,
                       const glm::vec3 &pos_body,
                       const glm::vec3 &pos_other_body,
                       const float brillouin)
{
    projection = glm::infinitePerspective(glm::radians(fov), win_aspect_ratio, 0.1f);

    // --- Base position along body radial + Brillouin offset ---
    const float rb = glm::length(pos_body);
    const float eps = 1e-6f;
    const glm::vec3 dir_body = (rb > eps) ? (pos_body / rb) : glm::vec3(1,0,0);
    glm::vec3 pos_base = pos_body + (brillouin_scale * brillouin) * dir_body;

    // Aim is always "the other body"
    glm::vec3 aim_target = pos_other_body;

    // --- Build view basis at the base position ---
    glm::vec3 f = glm::normalize(aim_target - pos_base); // forward
    glm::vec3 worldUp(0.0f, 0.0f, 1.0f);
    if (std::abs(glm::dot(f, worldUp)) > 0.999f)
        worldUp = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::vec3 r = glm::normalize(glm::cross(f, worldUp)); // right
    glm::vec3 u = glm::normalize(glm::cross(r, f));       // up (corrected)

    // --- Apply joystick V-offset on the (r,u) plane ---
    // The ImGui quad currently maps top→-1 and bottom→+1; flip Y so "up on widget" = +up in world:
    glm::vec2 v = glm::vec2(v_offset_ndc.x, -v_offset_ndc.y);
    const float v_range = v_offset_scale * brillouin; // world units (km) for joystick radius
    glm::vec3 offset_world = r * (v.x * v_range) + u * (v.y * v_range);

    pos = pos_base + offset_world;
    aim = aim_target;

    // --- Robust up again for final pos (tiny change after offset) ---
    f = glm::normalize(aim - pos);
    if (std::abs(glm::dot(f, worldUp)) > 0.999f)
        worldUp = glm::vec3(1.0f, 0.0f, 0.0f);
    r = glm::normalize(glm::cross(f, worldUp));
    up = glm::normalize(glm::cross(r, f));

    view = glm::lookAt(pos, aim, up);

    // --- Update sliders from FINAL pos (for UI readout) ---
    dvec3 spher = cart2spher(dvec3{ pos.x, pos.y, pos.z }, true);
    auto wrap_deg360 = [](float a){ a = fmodf(a, 360.0f); return (a < 0.0f) ? a + 360.0f : a; };
    dist = (float)spher[0];
    lon  = wrap_deg360((float)(spher[1] * 180.0 / M_PI));
    lat  = glm::clamp((float)(spher[2] * 180.0 / M_PI), 0.04f, 179.96f);
}
};


#endif
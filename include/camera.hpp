#ifndef CAMERA_HPP
#define CAMERA_HPP

#include<GL/glew.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class camera
{
public:
    glm::vec3 pos; //position vector
    glm::vec3 right, front, up; //coordinate system
    glm::vec3 world_up; //world-global up direction
    float yaw, pitch; // yaw, pitch angles (roll = 0 always for the camera)
    float vel; //velocity magnitude
    float mouse_sensitivity;

    //constructor
    camera(glm::vec3 init_pos = glm::vec3(0.0f,0.0f,0.0f),
           glm::vec3 init_world_up = glm::vec3(0.0f,0.0f,1.0f),
           float init_yaw = 90.0f,
           float init_pitch = 0.0f,
           float init_vel = 10.0f,
           float init_mouse_sensitivity = 0.1f)
    {
        pos = init_pos;
        world_up = init_world_up;
        yaw = init_yaw;
        pitch = init_pitch;
        vel = init_vel;
        mouse_sensitivity = init_mouse_sensitivity;
        update_vectors();
    }

    void translate_front(float delta_time)
    {
        pos += vel*delta_time*front;
    }

    void translate_back(float delta_time)
    {
        pos -= vel*delta_time*front;
    }

    void translate_right(float delta_time)
    {
        pos += vel*delta_time*right;
    }

    void translate_left(float delta_time)
    {
        pos -= vel*delta_time*right;
    }

    void rotate(float xoffset, float yoffset)
    {
        if (pitch > 88.9f) pitch = 88.9f;
        else if (pitch <= -88.9f) pitch = -88.9f;

        if (abs(yaw) > 360.0f) yaw = 0.0f;

        yaw -= xoffset*mouse_sensitivity;
        pitch -= yoffset*mouse_sensitivity;

        update_vectors();
    }

    void update_vectors()
    {
        front.x = cos(glm::radians(pitch))*cos(glm::radians(yaw));
        front.y = cos(glm::radians(pitch))*sin(glm::radians(yaw));
        front.z = sin(glm::radians(pitch));
        front = normalize(front);
        right = glm::normalize(glm::cross(front, world_up));
        up = glm::normalize(glm::cross(right, front));
    }

    glm::mat4 view()
    {
        return glm::lookAt(pos, pos + front, up);
    }
};

#endif

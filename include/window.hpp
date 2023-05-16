#ifndef WINDOW_HPP
#define WINDOW_HPP

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>

#include"gui.hpp"

class Window
{
    
private:

    GLFWwindow *m_pointer;
    int m_width, m_height;
    float m_aspectratio;

    static void framebuffer_size_callback(GLFWwindow *pointer, int width, int height)
    {
        Window *instance = static_cast<Window*>(glfwGetWindowUserPointer(pointer));
        if (instance != NULL)
        {
            instance->m_width = width;
            instance->m_height = height;
            instance->m_aspectratio = width/(float)height;
            glViewport(0,0, width,height);
        }
        else
            printf("'glfwGetWindowUserPointer(pointer)' is NULL. Exiting framebuffer_size_callback()...\n");

        return;
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        return;
    }

public:

    Window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        m_width = mode->width;
        m_height = mode->height;

        m_pointer = glfwCreateWindow(m_width, m_height, "KIMIN", NULL, NULL);
        if (m_pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwSetWindowUserPointer(m_pointer, this);
        glfwMakeContextCurrent(m_pointer);
        glfwSetWindowSizeLimits(m_pointer, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwSetFramebufferSizeCallback(m_pointer, framebuffer_size_callback);
        glfwSetKeyCallback(m_pointer, key_callback);
    }

    ~Window()
    {
        glfwDestroyWindow(m_pointer);
        glfwTerminate();
    }

    void game_loop()
    {
        GUI gui(m_pointer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.01f,0.01f,0.01f,1.0f);
        while (!glfwWindowShouldClose(m_pointer))
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            gui.begin();
            gui.properties.render(m_pointer);
            gui.console.render();
            gui.graphics.render();
            gui.render();
            gui.on_click_run();
            glfwSwapBuffers(m_pointer);
            glfwPollEvents();
        }
        return;
    }
};

#endif
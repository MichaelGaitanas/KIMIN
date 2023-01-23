#ifndef WINDOW_HPP
#define WINDOW_HPP

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

class window
{
public:
    GLFWwindow *pointer;
    const GLFWvidmode *mode;

    window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);
        pointer = glfwCreateWindow(800, 800, "KIMIN", NULL, NULL);
        if (pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwMakeContextCurrent(pointer);
        glfwSwapInterval(1);
        glfwSetWindowSizeLimits(pointer, 300, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
        mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }
    }

    static void key_callback(GLFWwindow *pointer, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(pointer, true);
    }

    void add_key_callback()
    {
        glfwSetKeyCallback(pointer, key_callback);
    }

    static void framebuffer_size_callback(GLFWwindow *pointer, int w, int h)
    {
        if (w == 0)
            w = 1;
        if (h == 0)
            h = 1;
        
        glViewport(0,0, w,h);
    }

    void add_framebuffer_size_callback()
    {
        glfwSetFramebufferSizeCallback(pointer, framebuffer_size_callback);
    }

    ~window()
    {
        glfwTerminate();
    }

};

#endif
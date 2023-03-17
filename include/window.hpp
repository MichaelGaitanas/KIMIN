#ifndef WINDOW_HPP
#define WINDOW_HPP

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

class Window
{
public:
    GLFWwindow *pointer; //unique window pointer
    static int width, height; //glfw window size
    static float aspectratio; //glfw window aspect ratio, basically width/height

    //Costructor to run once a glfw (window) object is instantiated.
    Window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        width = height = 800;
        pointer = glfwCreateWindow(width, height, "KIMIN", NULL, NULL);
        if (pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwMakeContextCurrent(pointer);
        glfwSwapInterval(1);
        glfwSetWindowSizeLimits(pointer, 600, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwGetWindowSize(pointer, &width, &height);
        aspectratio = width/(float)height;

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }
    }

    static void framebuffer_size_callback(GLFWwindow *pointer, int w, int h)
    {
        width  = w;
        height = h;
        aspectratio = (float)w/h;
        glViewport(0,0, w,h);
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

    void add_framebuffer_size_callback()
    {
        glfwSetFramebufferSizeCallback(pointer, framebuffer_size_callback);
    }

    //Destructor.
    ~Window()
    {
        glfwTerminate();
    }

};

int Window::width, Window::height;
float Window::aspectratio;

#endif
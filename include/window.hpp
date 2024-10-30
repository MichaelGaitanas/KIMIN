#ifndef WINDOW_HPP
#define WINDOW_HPP

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

#include"gui.hpp"


class Window
{
private:
    GLFWwindow *pointer;
    int width, height;
    float aspectratio;
    
    static void framebuffer_size_callback(GLFWwindow *ptr, int w, int h)
    {
        Window *instance = static_cast<Window*>(glfwGetWindowUserPointer(ptr));
        if (instance != nullptr)
        {
            instance->width = w;
            instance->height = h;
            instance->aspectratio = w/(float)h;
            glViewport(0,0, w,h);
        }
        else
        {
            //We print the followng for debugging purposes because IF instance == nullptr, it will be impossible to find the malfuncion...
            fprintf(stderr, "'glfwGetWindowUserPointer(ptr)' is nullptr. Exiting framebuffer_size_callback()...\n");
        }

        return;
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        //Terminate KIMIN in case the 'Esc' key is pressed.
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
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
        glfwWindowHint(GLFW_SAMPLES, 4);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;

        pointer = glfwCreateWindow(width, height, "KIMIN", nullptr, nullptr);
        if (pointer == nullptr)
        {
            fprintf(stderr, "Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwSetWindowUserPointer(pointer, this);
        glfwMakeContextCurrent(pointer);
        glfwSetWindowSizeLimits(pointer, 400,400, GLFW_DONT_CARE,GLFW_DONT_CARE);
        glfwSwapInterval(1);

        /* Glew is globally handled once in logo.hpp */

        //Register the callback functions.
        glfwSetFramebufferSizeCallback(pointer, framebuffer_size_callback);
        glfwSetKeyCallback(pointer, key_callback);
    }

    ~Window()
    {
        glfwDestroyWindow(pointer);
        glfwTerminate();
    }

    void game_loop()
    {   
        GUI gui(pointer);
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(0.05f,0.05f,0.05f,1.0f);
        while (!glfwWindowShouldClose(pointer))
        {
            glClear(GL_DEPTH_BUFFER_BIT); //The depth buffer is cleared when the 3D content is displayed.

            gui.begin();
            gui.properties.render();
            gui.console.render();
            gui.graphics.render();
            gui.render_integrator_controls();                            
            gui.render();     

            glfwSwapBuffers(pointer);
            glfwPollEvents();
        }

        return;
    }
};

#endif
#ifndef WINDOW_HPP
#define WINDOW_HPP

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

#include"gui.hpp"


class window
{
private:
    GLFWwindow *wpointer;
    int width, height;
    float aspect_ratio;
    
    static void framebuffer_size_callback(GLFWwindow *ptr, int w, int h)
    {
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        window *instance = static_cast<window*>(glfwGetWindowUserPointer(ptr));
        if (instance != nullptr)
        {
            instance->width = w;
            instance->height = h;
            instance->aspect_ratio = w/(float)h;
            glViewport(0,0, w,h);
        }
        else
        {
            //We print the followng for debugging purposes because IF instance == nullptr, it will be impossible to find the malfunction...
            fprintf(stderr, "'glfwGetWindowUserPointer(ptr)' is nullptr. Exiting framebuffer_size_callback()...\n");
        }
    }

    static void key_callback(GLFWwindow *win, int key, int /*scancode*/, int action, int /*mods*/)
    {
        //Terminate KIMIN in case the 'Esc' key is pressed.
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(win, GL_TRUE);
    }

public:
    window()
    {
        //(Re)initialize glfw along with some different settings. Since we explicitely terminated glfw in the logo.hpp, all (previous) corresponding resources are freed.
        //Now they are allocated again.
        if(!glfwInit())
        {
            fprintf(stderr, "Error : Failed to initialize glfw (window). Exiting...\n");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);
        glfwWindowHint(GLFW_SAMPLES, 4); //Anti-aliasing samples.

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;

        wpointer = glfwCreateWindow(width, height, "KIMIN", nullptr, nullptr);
        if (wpointer == nullptr)
        {
            fprintf(stderr, "Error : Failed to create glfw window. Exiting...\n");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwSetWindowUserPointer(wpointer, this);
        glfwMakeContextCurrent(wpointer);
        glfwSetWindowSizeLimits(wpointer, 400,400, GLFW_DONT_CARE,GLFW_DONT_CARE);
        glfwSwapInterval(1);

        //Glew however is still set up due to logo.hpp... */
        /*
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            fprintf(stderr, "Error : Failed to initialize glew. Exiting...\n");
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        */

        //Register the desired callback functions.
        glfwSetFramebufferSizeCallback(wpointer, framebuffer_size_callback);
        glfwSetKeyCallback(wpointer, key_callback);
    }

    ~window()
    {
        glfwDestroyWindow(wpointer);
        glfwTerminate();
    }

    //This is the app's function that runs all the time.
    void game_loop()
    {   
        gui ui(wpointer); //Instantiate the user interface along with some settings defined in the corresponding contructor.
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(0.08f,0.08f,0.08f,1.0f);
        while (!glfwWindowShouldClose(wpointer))
        {
            //At every frame (iteration), clear the color buffer (background) with the corresponding clear color set by glClearColor().
            //The depth buffer is cleared, only if 3D content is displayed.
            glClear(GL_COLOR_BUFFER_BIT);

            ui.begin();
            ui.properties.render();
            ui.console.render();
            //ui.graphics.render();
            //ui.render_integrator_controls();
            ui.render();

            if (ui.properties.run_pressed) {ui.when_run_is_clicked(); ui.properties.run_pressed = false;}

            glfwSwapBuffers(wpointer);
            glfwPollEvents();
        }
    }
};

#endif
/* This class handles the glfw window core functionality. */

#ifndef WINDOW_H
#define WINDOW_H

#include<cstdio>
#include<cstdlib>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"constants.h"
#include"gui.h"

class window
{
private:
    GLFWwindow *wpointer;
    int width, height;
    bool confirm_exit;
    
    static void framebuffer_size_callback(GLFWwindow *ptr, int w, int h)
    {
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        window *instance = static_cast<window*>(glfwGetWindowUserPointer(ptr));
        if (instance != nullptr)
        {
            instance->width  = w;
            instance->height = h;
            glViewport(0,0, w,h);
        }
        else
            fprintf(stderr, "[Warning] : In window::framebuffer_size_callback(), 'glfwGetWindowUserPointer(ptr)' returned nullptr. No call to 'glViewport()'. Proceeding...\n");
    }

    static void key_callback(GLFWwindow *ptr, int key, int /*scancode*/, int action, int /*mods*/)
    {
        window *instance = static_cast<window*>(glfwGetWindowUserPointer(ptr));
        if (instance != nullptr)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                instance->confirm_exit = !(instance->confirm_exit);
        }
        else
            fprintf(stderr, "[Warning] : In window::key_callback(), 'glfwGetWindowUserPointer(ptr)' returned nullptr. Unable to trigger the 'confirm_exit' logic. Proceeding...\n");
    }

public:
    window() : wpointer(nullptr),
               width(1),
               height(1),
               confirm_exit(false)
    {
        //(Re)initialize glfw along with some different settings. Since we explicitely terminated glfw in the logo.h, all (previous) corresponding resources are freed and now they are allocated again.
        if(!glfwInit())
        {
            fprintf(stderr, "[Error] : window() constructor failed to initialize glfw. Exiting...\n");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 60);
        glfwWindowHint(GLFW_SAMPLES, GLFW_SSAS_SAMPLES);
        glfwWindowHint(GLFW_DEPTH_BITS, GLFW_REQUESTED_DEPTH_BITS);

        //When the app launches, it shall be windowed-fullscreen mode. If the rescale button is clicked, the size remains, unless the user resizes it from the corners.
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        width  = mode->width;
        height = mode->height;
        SCX = (mode->width)/1920.0f;
        SCY = (mode->height)/1080.0f;
        //Now the computer knows how much to scale things based on your monitor's resolution.
        MONITOR_HZ = mode->refreshRate;

        wpointer = glfwCreateWindow(width, height, "KIMIN - Kinetic Impact Mission to NEO", nullptr, nullptr);
        if (wpointer == nullptr)
        {
            fprintf(stderr, "[Error] : window() constructor failed to create glfw window. Exiting...\n");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwSetWindowUserPointer(wpointer, this);
        glfwMakeContextCurrent(wpointer);
        glfwGetFramebufferSize(wpointer, &width, &height);
        glViewport(0,0, width,height);
        glfwSetWindowSizeLimits(wpointer, int(GLFW_MIN_WIDTH*SCX),int(GLFW_MIN_HEIGHT*SCY), GLFW_DONT_CARE,GLFW_DONT_CARE);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            fprintf(stderr, "[Error] : window() constructor failed to initialize glew. Exiting...\n");
            glfwDestroyWindow(wpointer);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        //Register the desired callback functions.
        glfwSetFramebufferSizeCallback(wpointer, framebuffer_size_callback);
        glfwSetKeyCallback(wpointer, key_callback);
    }

    ~window()
    {
        glfwDestroyWindow(wpointer);
        glfwTerminate();
    }

    void game_loop()
    {   
        gui ui(wpointer); //Instantiate the user interface along with some settings defined in the corresponding contructor (gui.h).
        
        glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, 1.0f);
        glEnable(GL_DEPTH_TEST);
        while (!glfwWindowShouldClose(wpointer))
        {
            glClear(GL_COLOR_BUFFER_BIT); //Depth is cleared in ui.sce.render(), where it is necessary.

            ui.begin();
            ui.tbar.render(wpointer, confirm_exit);
            ui.props.render(ui.task_is_running.load(), ui.task_was_aborted.load(), ui.task_progress.load());
            ui.cons.render();
            ui.update_solution_if_ready();
            ui.sce.render(width, height);
            ui.render();
            ui.poll_events();

            glfwSwapBuffers(wpointer);
            glfwPollEvents();
        }
    }
};

#endif
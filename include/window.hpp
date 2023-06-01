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
            printf("'glfwGetWindowUserPointer(ptr)' is nullptr. Exiting framebuffer_size_callback()...\n");
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
        glfwWindowHint(GLFW_SAMPLES,4);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        width = mode->width;
        height = mode->height;

        pointer = glfwCreateWindow(width, height, "KIMIN", nullptr, nullptr);
        if (pointer == nullptr)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwSetWindowUserPointer(pointer, this);
        glfwMakeContextCurrent(pointer);
        glfwSetWindowSizeLimits(pointer, 400,400, GLFW_DONT_CARE,GLFW_DONT_CARE);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            //Again for debugging purposes.
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }

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
        glClearColor(0.01f,0.01f,0.01f,1.0f);
        //gui.console.timedlog("[KIMIN] Welcome to KIMIN");
        //gui.console.timedlog("[KIMIN] Please select shape models and theory order for the simulation.");
        while (!glfwWindowShouldClose(pointer))
        {
            glClear(GL_COLOR_BUFFER_BIT);
            //The depth buffer is cleared when the 3D content is displayed.

            //Render the gui.
            gui.begin();
                gui.properties.render();
                gui.console.render();
                gui.graphics.render();
                // Integrator Info                
                ImGui::SetNextWindowPos( ImVec2(2.0f*ImGui::GetIO().DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 200.0), ImGuiCond_FirstUseEver);
                ImGui::Begin("Simulation Controls ", nullptr);
                ImGui::Dummy(ImVec2(10.0f,0.0f));
                ImGui::Indent(10.0f);
                if (gui.integrator.is_running)
                    ImGui::BeginDisabled(true);
                if (ImGui::Button("Run", ImVec2(70.0f,50.0f)))
                    gui.when_run_is_clicked();
                if (gui.integrator.is_running)
                    ImGui::EndDisabled();
                ImGui::SameLine();
                if (ImGui::Button("Kill", ImVec2(70.0f,50.0f))) gui.integrator.force_kill = true;
                ImGui::SameLine();
                if (!gui.integrator.exists_solution)
                    ImGui::BeginDisabled(true);
                if (ImGui::Button("Plot", ImVec2(70.0f,50.0f))){
                    Solution solution(gui.integrator);
                    solution.export_txt_files(gui.properties.simname);
                    gui.graphics.yield_solution(solution);
                    gui.graphics.aster1.update_mesh(solution.obj_path1.c_str());
                    gui.graphics.aster2.update_mesh(solution.obj_path2.c_str());
                }
                
                if (!gui.integrator.exists_solution)
                    ImGui::EndDisabled(); 
                
                ImGui::Unindent(10.0f);
                ImGui::Text("Progress:");
                ImGui::ProgressBar(gui.integrator.progress,ImVec2(260,50));
                ImGui::End();
                
            gui.render();               

            glfwSwapBuffers(pointer);
            glfwPollEvents();
        }

        return;
    }
};

#endif
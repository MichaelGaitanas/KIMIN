#ifndef GUI_HPP
#define GUI_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<thread>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"typedef.hpp"

#include"console.hpp"
#include"properties.hpp"
#include"integrator.hpp"
#include"solution.hpp"
#include"graphics.hpp"

class GUI
{

public:

    Properties properties;
    Graphics graphics;
    Console console;
    Integrator integrator;

    //Constructor and correct initialization of &io.
    GUI(GLFWwindow *pointer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.Fonts->AddFontFromFileTTF("../font/Roboto-Regular.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesGreek());
        //(void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGuiStyle &imstyle = ImGui::GetStyle();
        imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
        imstyle.FrameRounding = 5.0f;
        imstyle.WindowRounding = 5.0f;
        imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    }

    //Destructor
    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    //Create an new imgui frame.
    void begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    //Render the gui.
    void render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void when_run_is_clicked()
    {
        strvec errors = properties.validate();
        if (!errors.size())
        {
            //console.add_text("[Integrator] Running... ");
            integrator.update_properties(properties);
            integrator.force_kill = false;
            std::thread th(&Integrator::run, &integrator,std::ref(console));
            th.detach();
        }
        else
        {
            for (int i = 0; i < errors.size(); ++i)
            {
                console.timedlog(errors[i].c_str());
            }
        }
        return;
    }


};

#endif
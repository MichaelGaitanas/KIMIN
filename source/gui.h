#ifndef GUI_H
#define GUI_H

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<thread>
#include<atomic>
#include<cmath>

#include"typedef.h"
#include"properties_panel.h"
#include"console_panel.h"
#include"scene_panel.h"
#include"integrator.h"
#include"solution.h"

class gui
{
public:
    std::atomic<bool> simulation_is_running{false};
    std::atomic<bool> simulation_was_aborted{false};
    std::atomic<float> simulation_progress{0.0f};

    properties_panel properties;
    console_panel console;
    scene_panel scene;
    integrator integr;
    solution sol;

    //Initialize imgui and implot along with some settings.
    gui(GLFWwindow *wpointer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext(); //Strictly AFTER Imgui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.Fonts->AddFontFromFileTTF("../fonts/RobotoRegular.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesGreek()); //Dangerous...
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(wpointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGuiStyle &imstyle = ImGui::GetStyle();
        imstyle.FrameRounding = 6.0f;
        imstyle.WindowRounding = 6.0f;
        //imstyle.WindowMinSize = ImVec2(200.0f, 200.0f);
        ImVec4 *colors = imstyle.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f,0.1f,0.1f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.2f,0.2f,0.2f, 1.0f);
        colors[ImGuiCol_Header] = ImVec4(0.2f,0.2f,0.2f, 1.0f);
    }

    //Free gui resources.
    ~gui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext(); //Strictly BEFORE Imgui::DestroyContext();
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

   //'Run' and 'Abort' buttons functionality logic.
    void process_run_and_abort_buttons()
    {
        //'Run' protocol.
        if (properties.run_pressed && !simulation_is_running.load())
        {
            //Reset the 2 flags.
            properties.run_pressed = false;
            simulation_was_aborted.store(false);
            
            strvec errors = properties.validate();
            if (!errors.size())
            {
                simulation_is_running.store(true);
                //Launch a new simulation in a separate thread.
                std::thread simulation_thread([&]()
                {
                    integr.copy_properties(properties);
                    integr.prepare(simulation_was_aborted, simulation_progress, console);
                    integr.run(simulation_was_aborted, simulation_progress, console);
                    sol.copy_integrator(integr);
                    sol.construct(simulation_was_aborted, simulation_progress, console);
                    scene.copy_solution(sol);
                    simulation_is_running.store(false);
                });
                simulation_thread.detach();
            }
            else
                for (int i = 0; i < errors.size(); ++i)
                    console.add_time_and_then_text(errors[i].c_str());
        }

        //'Abort' protocol.
        if (properties.abort_pressed && simulation_is_running.load())
        {
            properties.abort_pressed = false; //Reset abort_pressed to prevent repeated triggering.
            simulation_was_aborted.store(true);
            simulation_is_running.store(false);
        }
    }
};

#endif
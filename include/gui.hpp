#ifndef GUI_HPP
#define GUI_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<thread>
#include<atomic>
#include<chrono>
#include<cmath>

#include"typedef.hpp"
#include"properties_panel.hpp"
#include"console_panel.hpp"
#include"integration.hpp"

void func_many_iters(std::atomic<bool> &abort_flag, std::atomic<float> &progress)
{
    for (float z = 0.0f; z < 20000.0f; z += 0.001f)
    {
        if (abort_flag.load())
            return;
        (void)exp(sin(sqrt(z*fabs(z)+ cos(z))));
        progress.store(z/20000.0f);
    }
}

class gui
{
private:

public:
    std::atomic<bool> simulation_is_running{false};
    std::atomic<bool> simulation_was_aborted{false};

    properties_panel properties;
    console_panel console;

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
        if (properties.run_pressed && !simulation_is_running)
        {
            //Reset the 2 flags.
            properties.run_pressed = false;
            simulation_was_aborted = false;
            properties.progress.store(0.0f);
            strvec errors = properties.validate();
            if (!errors.size())
            {
                console.add_time_and_then_text("Simulation is running... ");
                simulation_is_running = true;
                //Launch a fake simulation in a separate thread.
                std::thread simulation_thread([&]()
                {
                    func_many_iters(simulation_was_aborted, properties.progress);
                    simulation_is_running = false;
                });
                simulation_thread.detach();
            }
            else
                for (int i = 0; i < errors.size(); ++i)
                    console.add_time_and_then_text(errors[i].c_str());
        }

        //'Abort' protocol.
        if (properties.abort_pressed && simulation_is_running)
        {
            properties.abort_pressed = false; //Reset abort_pressed to prevent repeated triggering.
            console.add_time_and_then_text("Simulation was aborted.");
            simulation_was_aborted = true;
            simulation_is_running = false;
        }
    }
};

#endif
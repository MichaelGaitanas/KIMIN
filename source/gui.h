#ifndef GUI_H
#define GUI_H

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cmath>
#include<thread>
#include<atomic>

#include"typedef.h"
#include"top_bar_panel.h"
#include"properties_panel.h"
#include"console_panel.h"
#include"scene_panel.h"
#include"integrator.h"
#include"solution.h"

class gui
{
public:
    std::atomic<bool> task_is_running{false};
    std::atomic<bool> task_was_aborted{false};
    std::atomic<float> task_progress{0.0f};
    //We enumerate 3 possible time consuming tasks throughout the whole code : 1) Shape (.obj) loading and preparation, 2) Numerical integration, 3) Solution construction.
    //We set these 3 tasks to run at a separate thread to prevent gui freezing.

    //The following class instances are basically what you see in the gui, once KIMIN launches.
    top_bar_panel topbar;
    properties_panel properties;
    console_panel console;
    scene_panel scene;

    solution *sol;

    //Initialize imgui and implot along with some settings.
    gui(GLFWwindow *wpointer) : sol(nullptr)
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
        ImGui_ImplOpenGL3_Init("#version 450");
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
        delete sol; //Clean up the solution if allocated.
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
        if (properties.run_pressed && !task_is_running.load())
        {
            //Reset the 2 flags.
            properties.run_pressed = false;
            task_was_aborted.store(false);
            
            strvec errors = properties.validate();
            if (!errors.size())
            {
                task_is_running.store(true);
                //Launch a new simulation in a separate thread.
                std::thread task_thread([&]()
                {
                    integrator *integr = new integrator(properties);
                    integr->prepare(task_was_aborted, task_progress, console);
                    integr->run(task_was_aborted, task_progress, console);
                    if (!task_was_aborted.load())
                    {
                        delete sol;
                        sol = new solution(*integr);
                        sol->construct(task_was_aborted, task_progress, console);
                        scene.copy_solution(*sol);
                    }
                    delete integr;
                    task_is_running.store(false);
                });
                task_thread.detach();
            }
            else
                for (size_t i = 0; i < errors.size(); ++i)
                    console.add_time_and_then_text(errors[i].c_str());
        }

        //'Abort' protocol.
        if (properties.abort_pressed && task_is_running.load())
        {
            properties.abort_pressed = false; //Reset abort_pressed to prevent repeated triggering.
            task_was_aborted.store(true);
            task_is_running.store(false);
        }
    }

     //Export the solution when requested from the top bar panel.
    void process_export_buttons()
    {
        if (sol != nullptr && sol->t.size() > 0) //Ensure that a solution is available.
        {
            topbar.export_is_enabled = true;

            if (topbar.export_txt_clicked)
            {
                sol->export_txt_files(task_was_aborted, task_progress, console);
                topbar.export_txt_clicked = false; //Reset the flag after exporting.
            }

            if (topbar.export_json_clicked)
            {
                sol->export_json_files(task_was_aborted, task_progress, console);
                topbar.export_json_clicked = false; //Reset the flag after exporting.
            }
        }
        else
        {
            topbar.export_is_enabled = false;
        }
    }
};

#endif
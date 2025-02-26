/* This class acts as the kernel of the gui. It does not contain the commands that actually render the GUI (e.g. ImGui::Button() ...), but rather handles
   initializations, allocates/deallocates memory for some core stuff, handles threading, etc... */

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
    //The following class instances are basically what you see in the gui, once KIMIN is launched.
    top_bar_panel topbar;
    properties_panel properties;
    console_panel console;
    scene_panel scene;

    //The 'solution' class contains all data regarding the simulation (user inputs, numerical integrator results, orbit, etc...).
    solution *sol;

    //These variables are meant to track and control separate thread tasks, to prevent GUI freezing.
    std::atomic<bool> task_is_running, task_was_aborted;
    std::atomic<float> task_progress;

    //Initialize imgui, implot (along with some settings)a nd the class members.
    gui(GLFWwindow *wpointer) : sol(nullptr),
                                task_is_running(false),
                                task_was_aborted(false),
                                task_progress(0.0f)
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
        delete sol; //Clean up the solution if allocated. If not (i.e. if sol is nullptr), the 'delete' operator does nothing.
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

    //This function basically sums up the logic of all possible gui events.
    void poll_events()
    {
        process_run_and_abort_buttons();
        process_export_buttons();
    }

private:
    //'Run' and 'Abort' buttons logic.
    void process_run_and_abort_buttons()
    {
        if (properties.run_pressed && !task_is_running.load())
        {
            //(Re)set the 2 flags.
            properties.run_pressed = false;
            task_was_aborted.store(false);
            
            std::thread task_thread([&]()
            {
                if (properties.validate(task_progress, console)) //No input errors were found, so proceed with the simulation.
                {
                    integrator *integr = new integrator(properties);
                    integr->prepare(console);
                    task_is_running.store(true); //We assume that from this point on, the task is running because this affects the state of the 'Abort' button, which can be pressed only during the integration.
                    integr->run(task_was_aborted, task_progress, console);
                    if (!task_was_aborted.load())
                    {
                        delete sol; //Clean up the solution if allocated. If not (nullptr), the 'delete' operator does nothing.
                        sol = new solution(*integr);
                        sol->construct(console);
                        scene.copy_solution(*sol);
                    }
                    //In case abort, the previous solution (if present) exists in the memory.
                    delete integr; //The integrator lives only inside the current thread scope.
                    task_is_running.store(false);
                }
            });
            task_thread.detach();
        }

        //'Abort' protocol.
        if (properties.abort_pressed && task_is_running.load())
        {
            properties.abort_pressed = false; //Reset abort_pressed to prevent repeated triggering.
            task_was_aborted.store(true);
            task_is_running.store(false);
        }
    }

    //Export the solution when requested from the top bar panel. This happens with separate threads, just like
    //the integr->prepare(), integr->run(), etc... However, note that currently, this is not thread safe because
    //one might attempt to export a previous solution, while a new one is on the fly. I'll fix it, but for now, only
    //export when solution is completed.
    void process_export_buttons()
    {
        if (sol != nullptr && sol->t.size() > 0) //Ensure that a solution is available.
        {
            topbar.export_is_enabled = true;

            if (topbar.export_txt_clicked)
            {
                std::thread export_sol_thread([this]()
                {
                    sol->export_txt_files(console);
                });
                export_sol_thread.detach();
                topbar.export_txt_clicked = false; //Reset the flag after exporting the txt.
            }

            if (topbar.export_json_clicked)
            {
                std::thread export_sol_thread([this]()
                {
                    sol->export_json_files(console);
                });
                export_sol_thread.detach();
                topbar.export_json_clicked = false; //Reset the flag after exporting json.
            }
        }
        else
            topbar.export_is_enabled = false;
    }
};

#endif
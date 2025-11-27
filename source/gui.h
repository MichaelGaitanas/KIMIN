/* This class acts as the kernel of the gui. It does not contain the commands that actually render the visible gui (e.g. ImGui::Button(), etc.), but rather handles
   initializations, allocates/deallocates memory for some core stuff, handles threading, etc. */

#ifndef GUI_H
#define GUI_H

#include<thread>
#include<atomic>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include"constants.h"
#include"top_bar_panel.h"
#include"properties_panel.h"
#include"console_panel.h"
#include"scene_panel.h"
#include"integrator.h"
#include"solution.h"
#include"icons.h"

class gui
{
public:
    //The following members are basically what you see in the gui, once KIMIN is launched.
    top_bar_panel topbar;
    properties_panel properties;
    console_panel console;
    scene_panel scene;

    //A solution contains all data regarding a simulation (user inputs, numerical integrator results, orbit, etc.).
    solution sol, sol_pending;

    //These variables are meant to track and control separate thread tasks, in order to prevent the gui from 'freezing'.
    std::atomic<bool> task_is_running, task_was_aborted, solution_is_ready;
    std::atomic<float> task_progress;

    //Initialize imgui, implot (along with some settings) and the class members.
    gui(GLFWwindow *wpointer) : topbar(),
                                properties(),
                                console(),
                                scene(),
                                sol(),
                                sol_pending(),
                                task_is_running(false),
                                task_was_aborted(false),
                                solution_is_ready(false),
                                task_progress(0.0f)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext(); //AFTER Imgui::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.Fonts->AddFontFromFileTTF("../fonts/RobotoRegular.ttf", 15.0f*SCY, nullptr, io.Fonts->GetGlyphRangesGreek());
        (void)io;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(wpointer, true);
        ImGui_ImplOpenGL3_Init("#version 450");
        
        ImGuiStyle &imstyle = ImGui::GetStyle();
        imstyle.FrameRounding  = 6.0f*SCY;
        imstyle.WindowRounding = 6.0f*SCY;
        imstyle.ScrollbarRounding  *= SCY;
        imstyle.ChildRounding      *= SCY;
        imstyle.GrabRounding       *= SCY;
        imstyle.TabRounding        *= SCY;
        imstyle.ScrollbarSize      *= SCY;
        imstyle.GrabMinSize        *= SCY;
        imstyle.IndentSpacing      *= SCX; //Horizontal indentation used by ImGui::Indent()/Unindent()
        imstyle.ItemSpacing.x      *= SCX;
        imstyle.ItemSpacing.y      *= SCY;
        imstyle.ItemInnerSpacing.x *= SCX;
        imstyle.ItemInnerSpacing.y *= SCY;
        imstyle.CellPadding.x      *= SCX;
        imstyle.CellPadding.y      *= SCY;
        imstyle.WindowPadding.x    *= SCX;
        imstyle.WindowPadding.y    *= SCY;
        imstyle.FramePadding.x     *= SCX;
        imstyle.FramePadding.y     *= SCY;
        imstyle.Colors[ImGuiCol_WindowBg]      = ImVec4(0.1f,0.1f,0.1f, 1.0f);
        imstyle.Colors[ImGuiCol_FrameBg]       = ImVec4(0.2f,0.2f,0.2f, 1.0f);
        imstyle.Colors[ImGuiCol_Header]        = ImVec4(0.2f,0.2f,0.2f, 1.0f);
        imstyle.Colors[ImGuiCol_Border]        = ImVec4(0.15f,0.15f,0.15f, 1.0f);
        imstyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f,0.2f,0.2f, 1.0f);

        ImPlotStyle& plstyle = ImPlot::GetStyle();
        plstyle.PlotBorderSize       *= SCX;
        plstyle.LineWeight           *= SCX;
        plstyle.ErrorBarSize         *= SCX;
        plstyle.ErrorBarWeight       *= SCX;
        plstyle.PlotPadding.x        *= SCX;
        plstyle.PlotPadding.y        *= SCY;
        plstyle.LabelPadding.x       *= SCX;
        plstyle.LabelPadding.y       *= SCY;
        plstyle.LegendPadding.x      *= SCX;
        plstyle.LegendPadding.y      *= SCY;
        plstyle.LegendInnerPadding.x *= SCX;
        plstyle.LegendInnerPadding.y *= SCY;
        plstyle.PlotDefaultSize.x    *= SCX;
        plstyle.PlotDefaultSize.y    *= SCY;
        plstyle.LegendSpacing.x      *= SCX;
        plstyle.LegendSpacing.y      *= SCY;
        plstyle.MajorTickLen.x       *= SCX;
        plstyle.MajorTickLen.y       *= SCY;
        plstyle.MinorTickLen.x       *= SCX;
        plstyle.MinorTickLen.y       *= SCY;
        plstyle.MajorTickSize.x      *= SCX;
        plstyle.MajorTickSize.y      *= SCY;
        plstyle.MinorTickSize.x      *= SCX;
        plstyle.MinorTickSize.y      *= SCY;

        ImFontConfig cfg;
        cfg.MergeMode  = true;
        cfg.PixelSnapH = true;
        static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        io.Fonts->AddFontFromFileTTF("../fonts/Icons.otf", 15.0f*SCY, &cfg, icon_ranges);
    }

    //Free gui resources.
    ~gui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext(); //BEFORE Imgui::DestroyContext();
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

    void update_solution_if_ready()
    {
        if (solution_is_ready.exchange(false, std::memory_order_acquire))
        {
            sol = std::move(sol_pending);
            scene.setup(sol); //This happens in the main thread!
        }
    }

    void poll_events()
    {
        poll_topbar_events();
        poll_properties_events();
    }

private:
    //'Run' and 'Abort' buttons logic in the properties panel.
    void poll_properties_events()
    {
        //'Run' protocol.
        if (properties.run_pressed && !task_is_running.load())
        {
            //(Re)set the 2 flags.
            properties.run_pressed = false;
            task_was_aborted.store(false);
            
            std::thread task([this]()
            {
                if (properties.validate(console)) //If no input errors are found, proceed with the simulation.
                {
                    integrator integr(properties);
                    integr.prepare(console);
                    task_is_running.store(true); //From this point on, we assume that the task is running because this affects the state of the 'Abort' button, which can be pressed only during the integration.
                    integr.run(task_was_aborted, task_progress, console);
                    if (!task_was_aborted.load())
                    {
                        sol_pending = solution(integr);
                        sol_pending.construct(console);
                        integr.orbit.clear();
                        integr.orbit.shrink_to_fit();
                        solution_is_ready.store(true, std::memory_order_release);
                    }
                    task_is_running.store(false);
                }
            });
            task.detach();
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
    void poll_topbar_events()
    {
        //Event 1 : What happens after choosing to import a properties file :
        if (topbar.import_props_confirm && !topbar.properties_path.empty())
        {
            properties.import_file(topbar.properties_path.c_str(), console);
            topbar.import_props_confirm = false;
        }

        //Event 2 : What happens after choosing to a simulation solution :
        if (!sol.t.empty())
        {
            topbar.export_is_enabled = true;
            if (topbar.export_sol_clicked)
            {
                std::thread task([this]()
                {
                    sol.export_files(console);
                });
                task.detach();
                topbar.export_sol_clicked = false; //Since we exported, reset the flag.
            }
        }
        else
            topbar.export_is_enabled = false;
    }
};

#endif
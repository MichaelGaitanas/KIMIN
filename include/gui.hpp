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

    void render_integrator_controls(){
        float offsety = 0;
        float ysize = ImGui::GetIO().DisplaySize.y/7.0f;
        if ( ysize < 200){
            offsety = 200 - ysize;
        }
        ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x/7.0f, 6.0f*ImGui::GetIO().DisplaySize.y/7.0f - offsety), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2( ImGui::GetIO().DisplaySize.x/7.0f,      ImGui::GetIO().DisplaySize.y/7.0f), ImGuiCond_FirstUseEver);                
        //ImGui::SetNextWindowPos( ImVec2(2.0f*ImGui::GetIO().DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        //ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 200.0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Simulation controls ", nullptr);
        ImGui::Dummy(ImVec2(10.0f,0.0f));
        

        if (integrator.is_running)
            ImGui::BeginDisabled(true);

        if (ImGui::Button("Run", ImVec2(70.0f,50.0f)))
            when_run_is_clicked();

        if (integrator.is_running)
            ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Abort", ImVec2(70.0f,50.0f)))
            integrator.force_kill = true;

        ImGui::SameLine();
        if (!integrator.exists_solution)
            ImGui::BeginDisabled(true);
            
        if (ImGui::Button("Update Plot", ImVec2(100.0f,50.0f))){
            Solution solution(integrator);
            solution.export_txt_files(properties.simname);
            graphics.yield_solution(solution);
            graphics.aster1.update_mesh(solution.obj_path1.c_str());
            graphics.aster2.update_mesh(solution.obj_path2.c_str());
        }
        
        if (!integrator.exists_solution)
            ImGui::EndDisabled(); 
        
    
        ImGui::Text("Progress:");
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0/255,200.0/255,255.0/255, 1.0));
        ImGui::ProgressBar(integrator.progress,ImVec2(260,50));
        ImGui::PopStyleColor();
        ImGui::End();
    }

    void when_run_is_clicked()
    {
        strvec errors = properties.validate();
        if (!errors.size())
        {
            //console.add_text("[Integrator] Running... ");
            integrator.update_properties(properties);
            integrator.force_kill = false;
            std::thread th(&Integrator::run, &integrator, std::ref(console));
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
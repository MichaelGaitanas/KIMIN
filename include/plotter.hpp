#ifndef PLOTTER_HPP
#define PLOTTER_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include"typedef.hpp"

#include<cstdio>

class plotter
{
public:
    
    bool show_relx_plot=false;
    //bool show_w2bx;

    void plot_relx(const dmat &sol)
    {
        dvec tm, relxt;
        for (int i = 0; i < sol.size(); ++i)
        {
            tm.push_back(sol[i][0]);
            relxt.push_back(sol[i][1]);

        }

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x - ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
        ImGui::Begin("relx", &show_relx_plot);
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 offset = ImVec2(20.0f,40.0f);
        ImVec2 fin = ImVec2(window_size.x - offset.x, window_size.y - offset.y);
        if (ImPlot::BeginPlot("rel x",fin))
        {
            ImPlot::SetupAxes("t","relx(t)");
            ImPlot::PlotLine("", tm,relxt,tm.size());
            ImPlot::EndPlot();  
        }
        ImGui::End();
    }
};

#endif
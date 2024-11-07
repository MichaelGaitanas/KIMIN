#ifndef SCENE_PANEL_HPP
#define SCENE_PANEL_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"typedef.hpp"

class scene_panel
{
private:
    bvec plot_cart; //Buttons : [x, y, z, r] and [υx, υy, υz, υ].
    bvec plot_kep; //Buttons : [a, e, i, Ω, ω, M].
    bvec plot_rpy1, plot_rpy2; //Buttons : [roll 1, pitch 1, yaw 1] and [roll 2, pitch 2, yaw 2].
    bvec plot_w1i, plot_w1b; //Buttons : [ω1ix, ω1iy, ω1iz] and [ω1bx, ω1by, ω1bz].
    bvec plot_w2i, plot_w2b; //Buttons : [ω2ix, ω2iy, ω2iz] and [ω2bx, ω2by, ω2bz].
    bvec plot_ener_mom_rel_err; //Energy and angular momentum magnitude relative errors.

    bool common_plot_button(const char *label, const ImVec2 &dimensions, bool plot_func)
    {
        if (plot_func)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));

        if (ImGui::Button(label, dimensions))
            plot_func = !plot_func;
        ImGui::PopStyleColor();
        
        return plot_func;
    }
    
    void plot_buttons()
    {
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Position (relative)");
        plot_cart[0] = common_plot_button("x", ImVec2(50.0f, 20.0f), plot_cart[0]); ImGui::SameLine();
        plot_cart[1] = common_plot_button("y", ImVec2(50.0f, 20.0f), plot_cart[1]); ImGui::SameLine();
        plot_cart[2] = common_plot_button("z", ImVec2(50.0f, 20.0f), plot_cart[2]); ImGui::SameLine();
        plot_cart[3] = common_plot_button("r", ImVec2(50.0f, 20.0f), plot_cart[3]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Velocity (relative)");
        plot_cart[4] = common_plot_button("υx", ImVec2(50.0f, 20.0f), plot_cart[4]); ImGui::SameLine();
        plot_cart[5] = common_plot_button("υy", ImVec2(50.0f, 20.0f), plot_cart[5]); ImGui::SameLine();
        plot_cart[6] = common_plot_button("υz", ImVec2(50.0f, 20.0f), plot_cart[6]); ImGui::SameLine();
        plot_cart[7] = common_plot_button("υ" , ImVec2(50.0f, 20.0f), plot_cart[7]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Keplerian elements (relative)");
        plot_kep[0] = common_plot_button("a", ImVec2(35.0f, 20.0f), plot_kep[0]); ImGui::SameLine();
        plot_kep[1] = common_plot_button("e", ImVec2(35.0f, 20.0f), plot_kep[1]); ImGui::SameLine();
        plot_kep[2] = common_plot_button("i", ImVec2(35.0f, 20.0f), plot_kep[2]); ImGui::SameLine();
        plot_kep[3] = common_plot_button("Ω", ImVec2(35.0f, 20.0f), plot_kep[3]); ImGui::SameLine();
        plot_kep[4] = common_plot_button("ω", ImVec2(35.0f, 20.0f), plot_kep[4]); ImGui::SameLine();
        plot_kep[5] = common_plot_button("M", ImVec2(35.0f, 20.0f), plot_kep[5]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Euler angles (XYZ)");
        plot_rpy1[0] = common_plot_button("roll 1",  ImVec2(50.0f, 20.0f), plot_rpy1[0]); ImGui::SameLine();
        plot_rpy1[1] = common_plot_button("pitch 1", ImVec2(50.0f, 20.0f), plot_rpy1[1]); ImGui::SameLine();
        plot_rpy1[2] = common_plot_button("yaw 1",   ImVec2(50.0f, 20.0f), plot_rpy1[2]);
        plot_rpy2[0] = common_plot_button("roll 2",  ImVec2(50.0f, 20.0f), plot_rpy2[0]); ImGui::SameLine();
        plot_rpy2[1] = common_plot_button("pitch 2", ImVec2(50.0f, 20.0f), plot_rpy2[1]); ImGui::SameLine();
        plot_rpy2[2] = common_plot_button("yaw 2",   ImVec2(50.0f, 20.0f), plot_rpy2[2]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Body 1 angular velocity (inertial/body)");
        plot_w1i[0] = common_plot_button("ω1ix", ImVec2(50.0f, 20.0f), plot_w1i[0]); ImGui::SameLine();
        plot_w1i[1] = common_plot_button("ω1iy", ImVec2(50.0f, 20.0f), plot_w1i[1]); ImGui::SameLine();
        plot_w1i[2] = common_plot_button("ω1iz", ImVec2(50.0f, 20.0f), plot_w1i[2]);
        plot_w1b[0] = common_plot_button("ω1bx", ImVec2(50.0f, 20.0f), plot_w1b[0]); ImGui::SameLine();
        plot_w1b[1] = common_plot_button("ω1by", ImVec2(50.0f, 20.0f), plot_w1b[1]); ImGui::SameLine();
        plot_w1b[2] = common_plot_button("ω1bz", ImVec2(50.0f, 20.0f), plot_w1b[2]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Body 2 angular velocity (inertial/body)");
        plot_w2i[0] = common_plot_button("ω2ix", ImVec2(50.0f, 20.0f), plot_w2i[0]); ImGui::SameLine();
        plot_w2i[1] = common_plot_button("ω2iy", ImVec2(50.0f, 20.0f), plot_w2i[1]); ImGui::SameLine();
        plot_w2i[2] = common_plot_button("ω2iz", ImVec2(50.0f, 20.0f), plot_w2i[2]);
        plot_w2b[0] = common_plot_button("ω2bx", ImVec2(50.0f, 20.0f), plot_w2b[0]); ImGui::SameLine();
        plot_w2b[1] = common_plot_button("ω2by", ImVec2(50.0f, 20.0f), plot_w2b[1]); ImGui::SameLine();
        plot_w2b[2] = common_plot_button("ω2bz", ImVec2(50.0f, 20.0f), plot_w2b[2]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Integrals of motion");
        plot_ener_mom_rel_err[0] = common_plot_button("Energy",   ImVec2(80.0f, 25.0f), plot_ener_mom_rel_err[0]); ImGui::SameLine();
        plot_ener_mom_rel_err[1] = common_plot_button("Momentum", ImVec2(80.0f, 25.0f), plot_ener_mom_rel_err[1]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
    }

public:
    scene_panel() : plot_cart({false,false,false,false, false,false,false,false}),
                    plot_kep({false,false,false,false,false,false}),
                    plot_rpy1({false,false,false}),
                    plot_rpy2({false,false,false}),
                    plot_w1i({false,false,false}),
                    plot_w1b({false,false,false}),
                    plot_w2i({false,false,false}),
                    plot_w2b({false,false,false}),
                    plot_ener_mom_rel_err({false,false})
    { }
    
    void render()
    {
        ImGui::SetNextWindowPos( ImVec2(6.0f*ImGui::GetIO().DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(     ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene", nullptr);
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Plots 2D"))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.2f,1.0f));
            if (1) //Criterion that applies when the simulation hasn't been performed.
            {
                ImGui::BeginDisabled();
                plot_buttons();
                ImGui::EndDisabled();
            }
            else
            {
                plot_buttons();
                //if (plot_cart[0]) plot_cart[0] = common_plot("##1",  "##2",  "x [km]",               plot_cart[0], solution.x);
            }
            ImGui::PopStyleColor();
        }
        ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Video 3D"))
        {

        }
        ImGui::End();
    }
};

#endif
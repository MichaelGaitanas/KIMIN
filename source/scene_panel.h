#ifndef SCENE_PANEL_H
#define SCENE_PANEL_H

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include"typedef.h"
#include"solution.h"

class scene_panel
{
private:
    bvec plot_cart; //Buttons : [x, y, z, r] and [υx, υy, υz, υ].
    bvec plot_kep; //Buttons : [a, e, i, Ω, ω, M].
    bvec plot_rpy1, plot_rpy2; //Buttons : [roll 1, pitch 1, yaw 1] and [roll 2, pitch 2, yaw 2].
    bvec plot_w1i, plot_w1b; //Buttons : [ω1ix, ω1iy, ω1iz] and [ω1bx, ω1by, ω1bz].
    bvec plot_w2i, plot_w2b; //Buttons : [ω2ix, ω2iy, ω2iz] and [ω2bx, ω2by, ω2bz].
    bvec plot_ener_mom_rel_err; //Energy and angular momentum magnitude relative errors.

    solution sol;

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

    bool common_plot(const char *begin_id, const char *begin_plot_id, const char *yaxis_str, bool bool_plot_func, dvec &plot_func)
    {
        ImGui::SetNextWindowPos( ImVec2(4.0f*ImGui::GetIO().DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(2.0f*ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.x/5.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin(begin_id, &bool_plot_func);
        ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        if (ImPlot::BeginPlot(begin_plot_id, plot_win_size))
        {
            ImPlot::SetupAxes("time [days]", yaxis_str);
            ImPlot::PlotLine("", &sol.t[0], &plot_func[0], sol.t.size());
            if (sol.integr.collision)
            {
                ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 6.0f); //Set marker size for better visibility.
                ImPlot::PushStyleColor(ImPlotCol_MarkerFill, IM_COL32(255, 100, 0, 255)); //Set the bullet color to red.
                ImPlot::PlotScatter("", &sol.t.back(), &plot_func.back(), 1); //Plot the final point as a scatter plot.
                ImPlot::PopStyleColor();
                ImPlot::PopStyleVar();
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
        return bool_plot_func;
    }
    
    void render_plot_buttons()
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

    void copy_solution(solution &sol)
    {
        sol.reduce_to(1000); //This is a stack overflow (crash) savior!
        this->sol = sol;
    }
    
    void render()
    {
        ImGui::SetNextWindowPos( ImVec2(6.0f*ImGui::GetIO().DisplaySize.x/7.0f, 21.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(     ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.y - 21.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene", nullptr);
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Plots 2D"))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.2f,1.0f));
            if (!sol.dist.size()) //Criterion that applies when the simulation hasn't been performed.
            {
                ImGui::BeginDisabled();
                render_plot_buttons();
                ImGui::EndDisabled();   
            }
            else
            {
                render_plot_buttons();

                if (plot_cart[0]) plot_cart[0] = common_plot("##1",  "##2",  "x [km]",               plot_cart[0], sol.x);
                if (plot_cart[1]) plot_cart[1] = common_plot("##3",  "##4",  "y [km]",               plot_cart[1], sol.y);
                if (plot_cart[2]) plot_cart[2] = common_plot("##5",  "##6",  "z [km]",               plot_cart[2], sol.z);
                if (plot_cart[3]) plot_cart[3] = common_plot("##7",  "##8",  "distance [km]",        plot_cart[3], sol.dist);

                if (plot_cart[4]) plot_cart[4] = common_plot("##9",  "##10", "υx [km/sec]",          plot_cart[4], sol.vx);
                if (plot_cart[5]) plot_cart[5] = common_plot("##11", "##12", "υy [km/sec]",          plot_cart[5], sol.vy);
                if (plot_cart[6]) plot_cart[6] = common_plot("##13", "##14", "υz [km/sec]",          plot_cart[6], sol.vz);
                if (plot_cart[7]) plot_cart[7] = common_plot("##15", "##16", "υ magnitude [km/sec]", plot_cart[7], sol.vel);

                if (plot_kep[0]) plot_kep[0] = common_plot("##17", "##18", "a [km]",  plot_kep[0], sol.sma);
                if (plot_kep[1]) plot_kep[1] = common_plot("##19", "##20", "e [  ]",  plot_kep[1], sol.ecc);
                if (plot_kep[2]) plot_kep[2] = common_plot("##21", "##22", "i [deg]", plot_kep[2], sol.inc);
                if (plot_kep[3]) plot_kep[3] = common_plot("##23", "##24", "Ω [deg]", plot_kep[3], sol.raan);
                if (plot_kep[4]) plot_kep[4] = common_plot("##25", "##26", "ω [deg]", plot_kep[4], sol.argper);
                if (plot_kep[5]) plot_kep[5] = common_plot("##27", "##28", "M [deg]", plot_kep[5], sol.manom);

                if (plot_rpy1[0]) plot_rpy1[0] = common_plot("##29", "##30", "roll 1 [deg]",  plot_rpy1[0], sol.roll1);
                if (plot_rpy1[1]) plot_rpy1[1] = common_plot("##31", "##32", "pitch 1 [deg]", plot_rpy1[1], sol.pitch1);
                if (plot_rpy1[2]) plot_rpy1[2] = common_plot("##33", "##34", "yaw 1 [deg]",   plot_rpy1[2], sol.yaw1);

                if (plot_rpy2[0]) plot_rpy2[0] = common_plot("##35", "##36", "roll 2 [deg]",  plot_rpy2[0], sol.roll2);
                if (plot_rpy2[1]) plot_rpy2[1] = common_plot("##37", "##38", "pitch 2 [deg]", plot_rpy2[1], sol.pitch2);
                if (plot_rpy2[2]) plot_rpy2[2] = common_plot("##39", "##40", "yaw 2 [deg]",   plot_rpy2[2], sol.yaw2);

                if (plot_w1i[0]) plot_w1i[0] = common_plot("##41", "##42", "ω1ix [rad/sec]", plot_w1i[0], sol.w1ix);
                if (plot_w1i[1]) plot_w1i[1] = common_plot("##43", "##44", "ω1iy [rad/sec]", plot_w1i[1], sol.w1iy);
                if (plot_w1i[2]) plot_w1i[2] = common_plot("##45", "##46", "ω1iz [rad/sec]", plot_w1i[2], sol.w1iz);

                if (plot_w1b[0]) plot_w1b[0] = common_plot("##47", "##48", "ω1bx [rad/sec]", plot_w1b[0], sol.w1bx);
                if (plot_w1b[1]) plot_w1b[1] = common_plot("##49", "##50", "ω1by [rad/sec]", plot_w1b[1], sol.w1by);
                if (plot_w1b[2]) plot_w1b[2] = common_plot("##51", "##52", "ω1bz [rad/sec]", plot_w1b[2], sol.w1bz);

                if (plot_w2i[0]) plot_w2i[0] = common_plot("##53", "##54", "ω2ix [rad/sec]", plot_w2i[0], sol.w2ix);
                if (plot_w2i[1]) plot_w2i[1] = common_plot("##55", "##56", "ω2iy [rad/sec]", plot_w2i[1], sol.w2iy);
                if (plot_w2i[2]) plot_w2i[2] = common_plot("##57", "##58", "ω2iz [rad/sec]", plot_w2i[2], sol.w2iz);

                if (plot_w2b[0]) plot_w2b[0] = common_plot("##59", "##60", "ω2bx [rad/sec]", plot_w2b[0], sol.w2bx);
                if (plot_w2b[1]) plot_w2b[1] = common_plot("##61", "##62", "ω2by [rad/sec]", plot_w2b[1], sol.w2by);
                if (plot_w2b[2]) plot_w2b[2] = common_plot("##63", "##64", "ω2bz [rad/sec]", plot_w2b[2], sol.w2bz);

                if (plot_ener_mom_rel_err[0]) plot_ener_mom_rel_err[0] = common_plot("##65", "##66", "energy error [  ]",   plot_ener_mom_rel_err[0], sol.ener_rel_err);
                if (plot_ener_mom_rel_err[1]) plot_ener_mom_rel_err[1] = common_plot("##67", "##68", "momentum error [  ]", plot_ener_mom_rel_err[1], sol.mom_rel_err);

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
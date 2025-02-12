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
    
    bool render_scene, play_pause_video;
    uint64_t current_frame;

    solution sol;

    bool common_onoff_button(const char *label, const ImVec2 &dimensions, bool state)
    {
        if (state)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));

        if (ImGui::Button(label, dimensions))
            state = !state;
        ImGui::PopStyleColor();
        
        return state;
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
        plot_cart[0] = common_onoff_button("x", ImVec2(50.0f, 20.0f), plot_cart[0]); ImGui::SameLine();
        plot_cart[1] = common_onoff_button("y", ImVec2(50.0f, 20.0f), plot_cart[1]); ImGui::SameLine();
        plot_cart[2] = common_onoff_button("z", ImVec2(50.0f, 20.0f), plot_cart[2]); ImGui::SameLine();
        plot_cart[3] = common_onoff_button("r", ImVec2(50.0f, 20.0f), plot_cart[3]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Velocity (relative)");
        plot_cart[4] = common_onoff_button("υx", ImVec2(50.0f, 20.0f), plot_cart[4]); ImGui::SameLine();
        plot_cart[5] = common_onoff_button("υy", ImVec2(50.0f, 20.0f), plot_cart[5]); ImGui::SameLine();
        plot_cart[6] = common_onoff_button("υz", ImVec2(50.0f, 20.0f), plot_cart[6]); ImGui::SameLine();
        plot_cart[7] = common_onoff_button("υ" , ImVec2(50.0f, 20.0f), plot_cart[7]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Keplerian elements (relative)");
        plot_kep[0] = common_onoff_button("a", ImVec2(35.0f, 20.0f), plot_kep[0]); ImGui::SameLine();
        plot_kep[1] = common_onoff_button("e", ImVec2(35.0f, 20.0f), plot_kep[1]); ImGui::SameLine();
        plot_kep[2] = common_onoff_button("i", ImVec2(35.0f, 20.0f), plot_kep[2]); ImGui::SameLine();
        plot_kep[3] = common_onoff_button("Ω", ImVec2(35.0f, 20.0f), plot_kep[3]); ImGui::SameLine();
        plot_kep[4] = common_onoff_button("ω", ImVec2(35.0f, 20.0f), plot_kep[4]); ImGui::SameLine();
        plot_kep[5] = common_onoff_button("M", ImVec2(35.0f, 20.0f), plot_kep[5]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Euler angles (XYZ)");
        plot_rpy1[0] = common_onoff_button("roll 1",  ImVec2(50.0f, 20.0f), plot_rpy1[0]); ImGui::SameLine();
        plot_rpy1[1] = common_onoff_button("pitch 1", ImVec2(50.0f, 20.0f), plot_rpy1[1]); ImGui::SameLine();
        plot_rpy1[2] = common_onoff_button("yaw 1",   ImVec2(50.0f, 20.0f), plot_rpy1[2]);
        plot_rpy2[0] = common_onoff_button("roll 2",  ImVec2(50.0f, 20.0f), plot_rpy2[0]); ImGui::SameLine();
        plot_rpy2[1] = common_onoff_button("pitch 2", ImVec2(50.0f, 20.0f), plot_rpy2[1]); ImGui::SameLine();
        plot_rpy2[2] = common_onoff_button("yaw 2",   ImVec2(50.0f, 20.0f), plot_rpy2[2]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Body 1 angular velocity (inertial/body)");
        plot_w1i[0] = common_onoff_button("ω1ix", ImVec2(50.0f, 20.0f), plot_w1i[0]); ImGui::SameLine();
        plot_w1i[1] = common_onoff_button("ω1iy", ImVec2(50.0f, 20.0f), plot_w1i[1]); ImGui::SameLine();
        plot_w1i[2] = common_onoff_button("ω1iz", ImVec2(50.0f, 20.0f), plot_w1i[2]);
        plot_w1b[0] = common_onoff_button("ω1bx", ImVec2(50.0f, 20.0f), plot_w1b[0]); ImGui::SameLine();
        plot_w1b[1] = common_onoff_button("ω1by", ImVec2(50.0f, 20.0f), plot_w1b[1]); ImGui::SameLine();
        plot_w1b[2] = common_onoff_button("ω1bz", ImVec2(50.0f, 20.0f), plot_w1b[2]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Body 2 angular velocity (inertial/body)");
        plot_w2i[0] = common_onoff_button("ω2ix", ImVec2(50.0f, 20.0f), plot_w2i[0]); ImGui::SameLine();
        plot_w2i[1] = common_onoff_button("ω2iy", ImVec2(50.0f, 20.0f), plot_w2i[1]); ImGui::SameLine();
        plot_w2i[2] = common_onoff_button("ω2iz", ImVec2(50.0f, 20.0f), plot_w2i[2]);
        plot_w2b[0] = common_onoff_button("ω2bx", ImVec2(50.0f, 20.0f), plot_w2b[0]); ImGui::SameLine();
        plot_w2b[1] = common_onoff_button("ω2by", ImVec2(50.0f, 20.0f), plot_w2b[1]); ImGui::SameLine();
        plot_w2b[2] = common_onoff_button("ω2bz", ImVec2(50.0f, 20.0f), plot_w2b[2]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Integrals of motion");
        plot_ener_mom_rel_err[0] = common_onoff_button("Energy",   ImVec2(80.0f, 25.0f), plot_ener_mom_rel_err[0]); ImGui::SameLine();
        plot_ener_mom_rel_err[1] = common_onoff_button("Momentum", ImVec2(80.0f, 25.0f), plot_ener_mom_rel_err[1]);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
    }

    void render_scene_buttons()
    {
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Text("State");
        render_scene = common_onoff_button("Render", ImVec2(80.0f, 25.0f), render_scene);

        ImGui::SameLine();
        
        uint64_t zero_frame = 0;
        uint64_t total_frames = static_cast<uint64_t>(sol.t.size());
        if (!render_scene)
        {
            ImGui::BeginDisabled();
            ImGui::Button("Play/Pause",  ImVec2(80.0f, 25.0f));
            ImGui::Text("Frame");
            ImGui::SameLine();
            ImGui::SliderScalar("##35", ImGuiDataType_U64, &current_frame, &zero_frame, &total_frames, "%llu");
            ImGui::EndDisabled();
        }
        else
        {
            ImGui::Button("Play/Pause",  ImVec2(80.0f, 25.0f));
            ImGui::Text("Frame");
            ImGui::SameLine();
            ImGui::SliderScalar("##35", ImGuiDataType_U64, &current_frame, &zero_frame, &total_frames, "%llu");
        }

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
                    plot_ener_mom_rel_err({false,false}),
                    render_scene(false),
                    play_pause_video(false),
                    current_frame(0)
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

                if (plot_cart[0]) plot_cart[0] = common_plot("##1", "Relative x",        "x [km]",        plot_cart[0], sol.x);
                if (plot_cart[1]) plot_cart[1] = common_plot("##2", "Relative y",        "y [km]",        plot_cart[1], sol.y);
                if (plot_cart[2]) plot_cart[2] = common_plot("##3", "Relative z",        "z [km]",        plot_cart[2], sol.z);
                if (plot_cart[3]) plot_cart[3] = common_plot("##4", "Relative distance", "distance [km]", plot_cart[3], sol.dist);

                if (plot_cart[4]) plot_cart[4] = common_plot("##5", "Relative υx",          "υx [km/sec]",          plot_cart[4], sol.vx);
                if (plot_cart[5]) plot_cart[5] = common_plot("##6", "Relative υy",          "υy [km/sec]",          plot_cart[5], sol.vy);
                if (plot_cart[6]) plot_cart[6] = common_plot("##7", "Relative υz",          "υz [km/sec]",          plot_cart[6], sol.vz);
                if (plot_cart[7]) plot_cart[7] = common_plot("##8", "Relative υ magnitude", "υ magnitude [km/sec]", plot_cart[7], sol.vel);

                if (plot_kep[0]) plot_kep[0] = common_plot("##9",  "Semi-major axis",             "a [km]",  plot_kep[0], sol.sma);
                if (plot_kep[1]) plot_kep[1] = common_plot("##10", "Eccentricity",                "e [  ]",  plot_kep[1], sol.ecc);
                if (plot_kep[2]) plot_kep[2] = common_plot("##11", "Inclination",                 "i [deg]", plot_kep[2], sol.inc);
                if (plot_kep[3]) plot_kep[3] = common_plot("##12", "Longitude of ascending node", "Ω [deg]", plot_kep[3], sol.raan);
                if (plot_kep[4]) plot_kep[4] = common_plot("##13", "Argument of periapsis",       "ω [deg]", plot_kep[4], sol.argper);
                if (plot_kep[5]) plot_kep[5] = common_plot("##14", "Mean anomaly",                "M [deg]", plot_kep[5], sol.manom);

                if (plot_rpy1[0]) plot_rpy1[0] = common_plot("##15", "Body 1 roll",  "roll 1 [deg]",  plot_rpy1[0], sol.roll1);
                if (plot_rpy1[1]) plot_rpy1[1] = common_plot("##16", "Body 1 pitch", "pitch 1 [deg]", plot_rpy1[1], sol.pitch1);
                if (plot_rpy1[2]) plot_rpy1[2] = common_plot("##17", "Body 1 yaw",   "yaw 1 [deg]",   plot_rpy1[2], sol.yaw1);

                if (plot_rpy2[0]) plot_rpy2[0] = common_plot("##18", "Body 2 roll",  "roll 2 [deg]",  plot_rpy2[0], sol.roll2);
                if (plot_rpy2[1]) plot_rpy2[1] = common_plot("##19", "Body 2 pitch", "pitch 2 [deg]", plot_rpy2[1], sol.pitch2);
                if (plot_rpy2[2]) plot_rpy2[2] = common_plot("##20", "Body 2 yaw",   "yaw 2 [deg]",   plot_rpy2[2], sol.yaw2);

                if (plot_w1i[0]) plot_w1i[0] = common_plot("##21", "Body 1 ωx (inertial frame)", "ω1ix [rad/sec]", plot_w1i[0], sol.w1ix);
                if (plot_w1i[1]) plot_w1i[1] = common_plot("##22", "Body 1 ωy (inertial frame)", "ω1iy [rad/sec]", plot_w1i[1], sol.w1iy);
                if (plot_w1i[2]) plot_w1i[2] = common_plot("##23", "Body 1 ωz (inertial frame)", "ω1iz [rad/sec]", plot_w1i[2], sol.w1iz);

                if (plot_w1b[0]) plot_w1b[0] = common_plot("##24", "Body 1 ωx (body frame)", "ω1bx [rad/sec]", plot_w1b[0], sol.w1bx);
                if (plot_w1b[1]) plot_w1b[1] = common_plot("##25", "Body 1 ωy (body frame)", "ω1by [rad/sec]", plot_w1b[1], sol.w1by);
                if (plot_w1b[2]) plot_w1b[2] = common_plot("##26", "Body 1 ωz (body frame)", "ω1bz [rad/sec]", plot_w1b[2], sol.w1bz);

                if (plot_w2i[0]) plot_w2i[0] = common_plot("##27", "Body 2 ωx (inertial frame)", "ω2ix [rad/sec]", plot_w2i[0], sol.w2ix);
                if (plot_w2i[1]) plot_w2i[1] = common_plot("##28", "Body 2 ωy (inertial frame)", "ω2iy [rad/sec]", plot_w2i[1], sol.w2iy);
                if (plot_w2i[2]) plot_w2i[2] = common_plot("##29", "Body 2 ωz (inertial frame)", "ω2iz [rad/sec]", plot_w2i[2], sol.w2iz);

                if (plot_w2b[0]) plot_w2b[0] = common_plot("##30", "Body 2 ωx (body frame)", "ω2bx [rad/sec]", plot_w2b[0], sol.w2bx);
                if (plot_w2b[1]) plot_w2b[1] = common_plot("##31", "Body 2 ωy (body frame)", "ω2by [rad/sec]", plot_w2b[1], sol.w2by);
                if (plot_w2b[2]) plot_w2b[2] = common_plot("##32", "Body 2 ωz (body frame)", "ω2bz [rad/sec]", plot_w2b[2], sol.w2bz);

                if (plot_ener_mom_rel_err[0]) plot_ener_mom_rel_err[0] = common_plot("##33", "Energy relative error",             "energy error [  ]",   plot_ener_mom_rel_err[0], sol.ener_rel_err);
                if (plot_ener_mom_rel_err[1]) plot_ener_mom_rel_err[1] = common_plot("##34", "Momentum magnitude relative error", "momentum error [  ]", plot_ener_mom_rel_err[1], sol.mom_rel_err);

            }
            ImGui::PopStyleColor();
        }
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Video 3D"))
        {
            if (!sol.dist.size()) //Criterion that applies when the simulation hasn't been performed.
            {
                ImGui::BeginDisabled();
                render_scene_buttons();
                ImGui::EndDisabled();   
            }
            else
            {
                render_scene_buttons();
            }
        }
        ImGui::End();
    }
};

#endif
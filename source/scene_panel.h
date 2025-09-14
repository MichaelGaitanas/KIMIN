/* This class handles the rendering logic of the right panel (scene) in the gui AND the actual 3D scene. */

#ifndef SCENE_PANEL_H
#define SCENE_PANEL_H

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include"constant.h"
#include"typedef.h"
#include"solution.h"
#include"renderer3D.h"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class scene_panel
{
private:
    bvec plot_cart; //Buttons : [x, y, z, r] and [υx, υy, υz, υ].
    bvec plot_kep; //Buttons : [a, e, i, Ω, ω, M].
    bvec plot_rpy1, plot_rpy2; //Buttons : [roll 1, pitch 1, yaw 1, libration 1] and [roll 2, pitch 2, yaw 2, libration 1].
    bvec plot_w1i, plot_w1b; //Buttons : [ω1ix, ω1iy, ω1iz] and [ω1bx, ω1by, ω1bz].
    bvec plot_w2i, plot_w2b; //Buttons : [ω2ix, ω2iy, ω2iz] and [ω2bx, ω2by, ω2bz].
    bvec plot_ener_mom_rel_err; //Buttons : [energy, momentum].
    
    bool render_scene, play_pause_video;
    uint64_t zero_frame, current_frame, total_frames;

    int frame_rate; //Frame updates per second.
    float frame_accumulator; //Accumulates fractional frames between updates.

    bool reset_essential;

    solution sol, sol2D; //The 'sol' contains all the orbital data and is used to render the 3D scene. The 'sol2D' is used for the 2D plots.

    renderer3D rend3D;

public:
    scene_panel() : plot_cart({false,false,false,false, false,false,false,false}),
                    plot_kep({false,false,false,false,false,false}),
                    plot_rpy1({false,false,false,false}),
                    plot_rpy2({false,false,false,false}),
                    plot_w1i({false,false,false}),
                    plot_w1b({false,false,false}),
                    plot_w2i({false,false,false}),
                    plot_w2b({false,false,false}),
                    plot_ener_mom_rel_err({false,false}),
                    render_scene(false),
                    play_pause_video(false),
                    zero_frame(0),
                    current_frame(0),
                    total_frames(0),
                    frame_rate(60),
                    frame_accumulator(0.0f),
                    
    { }

    //(Re)set essential stuff upon a new simulation termination.
    void setup(const solution &sol)
    {
        //Obtain a whole solution copy for the 3D rendering. Then create a downsampled solution for the 2D plots.
        this->sol = sol;
        this->sol2D = sol.get_reduced_solution(PLOT_POINTS_2D);

        float binary_max_dist = *std::max_element(sol.dist.begin(), sol.dist.end());
        rend3D.sunlight.reset(sol.integr.brillouin1 + sol.integr.brillouin2 + binary_max_dist);
        rend3D.cam.reset(sol.integr.brillouin1 + sol.integr.brillouin2, binary_max_dist);

        current_frame = 0; //Set the frame slider to 0.
        play_pause_video = false; //Set the video at paused state ('true' means playing, 'false' means paused).
        reset_essential = true;
    }

    //This function controls the on/off logic of a clickable button in the gui.
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

    inline size_t map_frame_to_reduced_sol(const size_t frame, const size_t original_size, const size_t reduced_size)
    {
        //Edge cases.
        if (reduced_size == 0 || original_size <= 1 || reduced_size <= 1)
            return 0;
        
        double step = (original_size - 1.0)/(reduced_size - 1.0);
        double val = (double)frame/step;
        
        //Round to nearest integer.
        size_t i_reduced = (size_t)std::floor(val + 0.5);
    
        //Clamp to [0, reduced_size - 1].
        if (i_reduced >= reduced_size)
            i_reduced = reduced_size - 1;
    
        return i_reduced;
    }

    //This function plots the data {t,f(t)}, where t is time and f(t) is the plot_func.
    bool common_plot(const char *begin_id, const char *begin_plot_id, const char *yaxis_str, bool bool_plot_func, dvec &plot_func)
    {
        ImGui::SetNextWindowPos( ImVec2(0.6f*ImGui::GetIO().DisplaySize.x, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.25f*ImGui::GetIO().DisplaySize.x, 0.4f*ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin(begin_id, &bool_plot_func);
        ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        if (ImPlot::BeginPlot(begin_plot_id, plot_win_size))
        {
            ImPlot::SetupAxes("time [days]", yaxis_str);
            ImPlot::PlotLine("", &sol2D.t[0], &plot_func[0], sol2D.t.size());
            
            //Current frame marker logic :
            size_t i_reduced = map_frame_to_reduced_sol(current_frame, sol.t.size(), sol2D.t.size());
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6.0f, ImColor(0, 255, 0, 255), 1.0f, ImColor(0, 255, 0, 255));
            ImPlot::PlotScatter("Current frame", &sol2D.t[i_reduced], &plot_func[i_reduced], 1);

            //If there's a collision, highlight final point.
            if (sol2D.integr.collision)
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f, ImColor(255, 100, 0, 255), 1.0f, ImColor(255, 100, 0, 255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &plot_func.back(), 1); //Plot the final point as a scatter plot.
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
        return bool_plot_func;
    }
    
    //Draw the 2D plot buttons in the gui.
    void render_plot_buttons()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
        if (ImGui::TreeNodeEx("Mutual", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Position");
            plot_cart[0] = common_onoff_button("x##1", ImVec2(50.0f, 20.0f), plot_cart[0]); ImGui::SameLine();
            plot_cart[1] = common_onoff_button("y##2", ImVec2(50.0f, 20.0f), plot_cart[1]); ImGui::SameLine();
            plot_cart[2] = common_onoff_button("z##3", ImVec2(50.0f, 20.0f), plot_cart[2]); ImGui::SameLine();
            plot_cart[3] = common_onoff_button("r##4", ImVec2(50.0f, 20.0f), plot_cart[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Velocity");
            plot_cart[4] = common_onoff_button("υx##5", ImVec2(50.0f, 20.0f), plot_cart[4]); ImGui::SameLine();
            plot_cart[5] = common_onoff_button("υy##6", ImVec2(50.0f, 20.0f), plot_cart[5]); ImGui::SameLine();
            plot_cart[6] = common_onoff_button("υz##7", ImVec2(50.0f, 20.0f), plot_cart[6]); ImGui::SameLine();
            plot_cart[7] = common_onoff_button("υ##8" , ImVec2(50.0f, 20.0f), plot_cart[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Keplerian elements");
            plot_kep[0] = common_onoff_button("a##9",  ImVec2(35.0f, 20.0f), plot_kep[0]); ImGui::SameLine();
            plot_kep[1] = common_onoff_button("e##10", ImVec2(35.0f, 20.0f), plot_kep[1]); ImGui::SameLine();
            plot_kep[2] = common_onoff_button("i##11", ImVec2(35.0f, 20.0f), plot_kep[2]); ImGui::SameLine();
            plot_kep[3] = common_onoff_button("Ω##12", ImVec2(35.0f, 20.0f), plot_kep[3]); ImGui::SameLine();
            plot_kep[4] = common_onoff_button("ω##13", ImVec2(35.0f, 20.0f), plot_kep[4]); ImGui::SameLine();
            plot_kep[5] = common_onoff_button("M##14", ImVec2(35.0f, 20.0f), plot_kep[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Energy and momentum errors");
            plot_ener_mom_rel_err[0] = common_onoff_button("energy##15",   ImVec2(80.0f, 25.0f), plot_ener_mom_rel_err[0]); ImGui::SameLine();
            plot_ener_mom_rel_err[1] = common_onoff_button("momentum##16", ImVec2(80.0f, 25.0f), plot_ener_mom_rel_err[1]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Body 1", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Euler angles (XYZ)");
            plot_rpy1[0] = common_onoff_button("roll##17",        ImVec2(50.0f, 20.0f), plot_rpy1[0]); ImGui::SameLine();
            plot_rpy1[1] = common_onoff_button("pitch##18",       ImVec2(50.0f, 20.0f), plot_rpy1[1]); ImGui::SameLine();
            plot_rpy1[2] = common_onoff_button("yaw##19",         ImVec2(50.0f, 20.0f), plot_rpy1[2]); ImGui::SameLine();
            plot_rpy1[3] = common_onoff_button("libration##20",   ImVec2(60.0f, 20.0f), plot_rpy1[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Angular velocity (inertial frame)");
            plot_w1i[0] = common_onoff_button("ωx##21", ImVec2(50.0f, 20.0f), plot_w1i[0]); ImGui::SameLine();
            plot_w1i[1] = common_onoff_button("ωy##22", ImVec2(50.0f, 20.0f), plot_w1i[1]); ImGui::SameLine();
            plot_w1i[2] = common_onoff_button("ωz##23", ImVec2(50.0f, 20.0f), plot_w1i[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            
            ImGui::Text("Angular velocity (body frame)");
            plot_w1b[0] = common_onoff_button("ω1##24", ImVec2(50.0f, 20.0f), plot_w1b[0]); ImGui::SameLine();
            plot_w1b[1] = common_onoff_button("ω2##25", ImVec2(50.0f, 20.0f), plot_w1b[1]); ImGui::SameLine();
            plot_w1b[2] = common_onoff_button("ω3##26", ImVec2(50.0f, 20.0f), plot_w1b[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Body 2", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Euler angles (XYZ)");
            plot_rpy2[0] = common_onoff_button("roll##27",        ImVec2(50.0f, 20.0f), plot_rpy2[0]); ImGui::SameLine();
            plot_rpy2[1] = common_onoff_button("pitch##28",       ImVec2(50.0f, 20.0f), plot_rpy2[1]); ImGui::SameLine();
            plot_rpy2[2] = common_onoff_button("yaw##29",         ImVec2(50.0f, 20.0f), plot_rpy2[2]); ImGui::SameLine();
            plot_rpy2[3] = common_onoff_button("libration##30",   ImVec2(60.0f, 20.0f), plot_rpy2[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Angular velocity (inertial frame)");
            plot_w2i[0] = common_onoff_button("ωx##31", ImVec2(50.0f, 20.0f), plot_w2i[0]); ImGui::SameLine();
            plot_w2i[1] = common_onoff_button("ωy##32", ImVec2(50.0f, 20.0f), plot_w2i[1]); ImGui::SameLine();
            plot_w2i[2] = common_onoff_button("ωz##33", ImVec2(50.0f, 20.0f), plot_w2i[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            
            ImGui::Text("Angular velocity (body frame)");
            plot_w2b[0] = common_onoff_button("ω1##34", ImVec2(50.0f, 20.0f), plot_w2b[0]); ImGui::SameLine();
            plot_w2b[1] = common_onoff_button("ω2##35", ImVec2(50.0f, 20.0f), plot_w2b[1]); ImGui::SameLine();
            plot_w2b[2] = common_onoff_button("ω3##36", ImVec2(50.0f, 20.0f), plot_w2b[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::TreePop();
        }
        ImGui::PopStyleVar();
    }

    void render_scene_buttons()
    {
        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Text("Content state");
        render_scene = common_onoff_button("Render##37", ImVec2(80.0f, 25.0f), render_scene);
        ImGui::SameLine();
        total_frames = static_cast<uint64_t>(sol.t.size());
        uint64_t max_frame = (total_frames > 0) ? total_frames - 1 : 0;

        bool disabled = !render_scene;

        if (disabled)
        {
            ImGui::BeginDisabled();
            ImGui::Button("Play/Pause", ImVec2(80.0f, 25.0f));
            ImGui::EndDisabled();
        }
        else
        {
            if (ImGui::Button("Play/Pause", ImVec2(80.0f, 25.0f)))
                play_pause_video = !play_pause_video;
        }

        if (disabled)
            ImGui::BeginDisabled();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Text("Frame");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f);
        ImGui::SliderScalar("##38", ImGuiDataType_U64, &current_frame, &zero_frame, &max_frame, "%llu");

        ImGui::Text("Rate");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f);
        ImGui::SliderInt("[Hz]##39", &frame_rate, 0, 60, "%d");

        if (sol.t.empty())
            ImGui::Text("Time : 0.00  [days]");
        else
            ImGui::Text("Time : %.2f  [days]", (float)sol.t[current_frame]);

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Camera setup");

        ImGui::Text("Dist");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[km]##40", &rend3D.cam.dist, rend3D.cam.min_dist, rend3D.cam.max_dist, "%.3f", ImGuiSliderFlags_Logarithmic);

        ImGui::Text("Lon");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##41", &rend3D.cam.lon, 0.0f, 360.0f);

        ImGui::Text("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##42", &rend3D.cam.lat, 0.0f, 180.0f);

        ImGui::Text("FoV");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##43", &rend3D.cam.fov, 1.0f, 179.0f, "%.0f");

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Sun direction");

        ImGui::Text("Lon");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##44", &rend3D.sunlight.lon, 0.0f, 360.0f);

        ImGui::Text("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##45", &rend3D.sunlight.lat, 0.0f, 180.0f);

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Shadow map");

        ImGui::Text("Reso");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        if (ImGui::SliderInt("[pix]##46", &rend3D.depth_reso, 1024, 8192))
            rend3D.setup_depth_fbo();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Visible meshes");

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##47", &render_aster1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::Text("Axes 1");
        ImGui::SameLine();
        ImGui::Checkbox("##48", &render_axes1);

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##49", &render_aster2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::Text("Axes 2");
        ImGui::SameLine();
        ImGui::Checkbox("##50", &render_axes2);
        ImGui::Dummy(ImVec2(0.0f,0.6f));

        ImGui::Text("Orbits");

        uint64_t visible_last1 = (orb1.draw_count == 0) ? 0 : static_cast<uint64_t>(orb1.draw_count - 1);

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##51", &render_orb1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::SetNextItemWidth(100);
        ImGui::SliderScalar("##52", ImGuiDataType_U64, &visible_last1, &zero_frame, &max_frame, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb1_match = common_onoff_button("Match##53", ImVec2(50.0f, 18.0f), orb1_match);
        orb1.draw_count = static_cast<size_t>(visible_last1+1);
        if (orb1_match)
            orb1.draw_count = current_frame;
        
        uint64_t visible_last2 = (orb2.draw_count == 0) ? 0 : static_cast<uint64_t>(orb2.draw_count - 1);

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##54", &render_orb2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::SetNextItemWidth(100);
        ImGui::SliderScalar("##55", ImGuiDataType_U64, &visible_last2, &zero_frame, &max_frame, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb2.draw_count = static_cast<size_t>(visible_last2+1);
        orb2_match = common_onoff_button("Match##56", ImVec2(50.0f, 18.0f), orb2_match);
        if (orb2_match)
            orb2.draw_count = current_frame;

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Colors");

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::ColorEdit3("##57", glm::value_ptr(aster1_col), ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        ImGui::SetCursorPosX(120.0f);
        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::ColorEdit3("##58", glm::value_ptr(aster2_col), ImGuiColorEditFlags_NoInputs);
        ImGui::Text("Orbit 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::ColorEdit3("##59", glm::value_ptr(orb1_col), ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        ImGui::SetCursorPosX(120.0f);
        ImGui::Text("Orbit 2");
        ImGui::SameLine();
        ImGui::ColorEdit3("##60", glm::value_ptr(orb2_col), ImGuiColorEditFlags_NoInputs);

        if (disabled)
            ImGui::EndDisabled();

        if (!disabled)
            render_3D_content();


        if (play_pause_video && current_frame < total_frames - 1)
        {
            if (frame_rate == 0)
            {
                // The slider is set to 0 => paused
                // Do not increment current_frame
            }
            else if (frame_rate < 60)
            {
                // We do a time-based step to achieve the chosen frame_rate
                frame_accumulator += ImGui::GetIO().DeltaTime;
                float step = 1.0f / static_cast<float>(frame_rate);

                // In case dt is large (e.g., if the user drags the window), 
                // use a while() so we don’t “miss” increments:
                while (frame_accumulator >= step && current_frame < total_frames - 1)
                {
                    current_frame++;
                    frame_accumulator -= step;
                }
            }
            else
            {
                // frame_rate == 60 => let it play as fast as the machine can handle
                // i.e. increment every time we render:
                current_frame++;
            }
        }
    }
    
    void render(const int win_width, const int win_height)
    {
        //Copy the window's dimensions to the renderer3D's members. We need them at each frame to compute the camera's projection matrix.
        rend3D.win_width = win_width;
        rend3D.win_height = win_height;

        ImGui::SetNextWindowPos( ImVec2(0.85f*ImGui::GetIO().DisplaySize.x, 21.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 21.0f), ImGuiCond_FirstUseEver);
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

                if (plot_cart[0]) plot_cart[0] = common_plot("##101", "Relative x",        "x [km]",        plot_cart[0], sol2D.x);
                if (plot_cart[1]) plot_cart[1] = common_plot("##102", "Relative y",        "y [km]",        plot_cart[1], sol2D.y);
                if (plot_cart[2]) plot_cart[2] = common_plot("##103", "Relative z",        "z [km]",        plot_cart[2], sol2D.z);
                if (plot_cart[3]) plot_cart[3] = common_plot("##104", "Relative distance", "distance [km]", plot_cart[3], sol2D.dist);

                if (plot_cart[4]) plot_cart[4] = common_plot("##105", "Relative υx",          "υx [km/sec]",          plot_cart[4], sol2D.vx);
                if (plot_cart[5]) plot_cart[5] = common_plot("##106", "Relative υy",          "υy [km/sec]",          plot_cart[5], sol2D.vy);
                if (plot_cart[6]) plot_cart[6] = common_plot("##107", "Relative υz",          "υz [km/sec]",          plot_cart[6], sol2D.vz);
                if (plot_cart[7]) plot_cart[7] = common_plot("##108", "Relative υ magnitude", "υ magnitude [km/sec]", plot_cart[7], sol2D.vel);

                if (plot_kep[0]) plot_kep[0] = common_plot("##109",  "Semi-major axis",            "a [km]",  plot_kep[0], sol2D.sma);
                if (plot_kep[1]) plot_kep[1] = common_plot("##110", "Eccentricity",                "e [  ]",  plot_kep[1], sol2D.ecc);
                if (plot_kep[2]) plot_kep[2] = common_plot("##111", "Inclination",                 "i [deg]", plot_kep[2], sol2D.inc);
                if (plot_kep[3]) plot_kep[3] = common_plot("##112", "Longitude of ascending node", "Ω [deg]", plot_kep[3], sol2D.raan);
                if (plot_kep[4]) plot_kep[4] = common_plot("##113", "Argument of periapsis",       "ω [deg]", plot_kep[4], sol2D.argper);
                if (plot_kep[5]) plot_kep[5] = common_plot("##114", "Mean anomaly",                "M [deg]", plot_kep[5], sol2D.manom);

                if (plot_rpy1[0]) plot_rpy1[0] = common_plot("##115", "Body 1 roll",        "roll 1 [deg]", plot_rpy1[0], sol2D.roll1);
                if (plot_rpy1[1]) plot_rpy1[1] = common_plot("##116", "Body 1 pitch",       "pitch 1 [deg]", plot_rpy1[1], sol2D.pitch1);
                if (plot_rpy1[2]) plot_rpy1[2] = common_plot("##117", "Body 1 yaw",         "yaw 1 [deg]", plot_rpy1[2], sol2D.yaw1);
                if (plot_rpy1[3]) plot_rpy1[3] = common_plot("##118", "Body 1 libration",   "rel.  yaw 1 [deg]", plot_rpy1[3], sol2D.relyaw1);

                if (plot_rpy2[0]) plot_rpy2[0] = common_plot("##119", "Body 2 roll",        "roll 2  [deg]", plot_rpy2[0], sol2D.roll2);
                if (plot_rpy2[1]) plot_rpy2[1] = common_plot("##120", "Body 2 pitch",       "pitch 2 [deg]", plot_rpy2[1], sol2D.pitch2);
                if (plot_rpy2[2]) plot_rpy2[2] = common_plot("##121", "Body 2 yaw",         "yaw 2 [deg]", plot_rpy2[2], sol2D.yaw2);
                if (plot_rpy2[3]) plot_rpy2[3] = common_plot("##122", "Body 2 libration",   "rel.  yaw 2 [deg]", plot_rpy2[3], sol2D.relyaw2);

                if (plot_w1i[0]) plot_w1i[0] = common_plot("##123", "Body 1 ωx (inertial frame)", "ω1ix [rad/sec]", plot_w1i[0], sol2D.w1ix);
                if (plot_w1i[1]) plot_w1i[1] = common_plot("##124", "Body 1 ωy (inertial frame)", "ω1iy [rad/sec]", plot_w1i[1], sol2D.w1iy);
                if (plot_w1i[2]) plot_w1i[2] = common_plot("##125", "Body 1 ωz (inertial frame)", "ω1iz [rad/sec]", plot_w1i[2], sol2D.w1iz);

                if (plot_w1b[0]) plot_w1b[0] = common_plot("##126", "Body 1 ωx (body frame)", "ω1bx [rad/sec]", plot_w1b[0], sol2D.w1bx);
                if (plot_w1b[1]) plot_w1b[1] = common_plot("##127", "Body 1 ωy (body frame)", "ω1by [rad/sec]", plot_w1b[1], sol2D.w1by);
                if (plot_w1b[2]) plot_w1b[2] = common_plot("##128", "Body 1 ωz (body frame)", "ω1bz [rad/sec]", plot_w1b[2], sol2D.w1bz);

                if (plot_w2i[0]) plot_w2i[0] = common_plot("##129", "Body 2 ωx (inertial frame)", "ω2ix [rad/sec]", plot_w2i[0], sol2D.w2ix);
                if (plot_w2i[1]) plot_w2i[1] = common_plot("##130", "Body 2 ωy (inertial frame)", "ω2iy [rad/sec]", plot_w2i[1], sol2D.w2iy);
                if (plot_w2i[2]) plot_w2i[2] = common_plot("##131", "Body 2 ωz (inertial frame)", "ω2iz [rad/sec]", plot_w2i[2], sol2D.w2iz);

                if (plot_w2b[0]) plot_w2b[0] = common_plot("##132", "Body 2 ωx (body frame)", "ω2bx [rad/sec]", plot_w2b[0], sol2D.w2bx);
                if (plot_w2b[1]) plot_w2b[1] = common_plot("##133", "Body 2 ωy (body frame)", "ω2by [rad/sec]", plot_w2b[1], sol2D.w2by);
                if (plot_w2b[2]) plot_w2b[2] = common_plot("##134", "Body 2 ωz (body frame)", "ω2bz [rad/sec]", plot_w2b[2], sol2D.w2bz);

                if (plot_ener_mom_rel_err[0]) plot_ener_mom_rel_err[0] = common_plot("##135", "Energy relative error",             "| (E[i+1] - E[0])/E[0] |", plot_ener_mom_rel_err[0], sol2D.ener_rel_err);
                if (plot_ener_mom_rel_err[1]) plot_ener_mom_rel_err[1] = common_plot("##136", "Momentum magnitude relative error", "| (L[i+1] - L[0])/L[0] |", plot_ener_mom_rel_err[1], sol2D.mom_rel_err);

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
                render_scene_buttons();
        }
        ImGui::End();
    }
};

#endif
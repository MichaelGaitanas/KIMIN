/* This class handles the rendering logic of the right panel (scene) in the gui. */

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
#include"icons.h"

#include<algorithm>

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
    bvec plot_cart_sp; //Buttons : [xs, ys, zs].
    
    bool render_scene, play_pause_video, reset_gpu_essential, auto_replay, orb1_sync, orb2_sync, orb_sp_sync;
    uint64_t current_frame, total_frames;
    int frame_rate; //Frame updates per second.
    float frame_accumulator; //Accumulates fractional frames between updates.

    solution sol, sol2D; //The 'sol' contains all the orbital data and is used to render the 3D scene. The 'sol2D' is downsampled and used for the 2D plots.
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
                    plot_cart_sp({false,false,false}),
                    render_scene(false),
                    play_pause_video(false),
                    reset_gpu_essential(false),
                    auto_replay(false),
                    orb1_sync(false),
                    orb2_sync(false),
                    orb_sp_sync(false),
                    current_frame(0),
                    total_frames(0),
                    frame_rate(60),
                    frame_accumulator(0.0f)
    { }

    //Reset essential stuff upon a new simulation termination.
    //Note : apart from the following resets, we still have to reset OpenGL stuff. But the following setup() function is gonna run in
    //the 'task_thread' thread defined in gui.h, not in the main thread, where OpenGL runs. So any gl* commands that handle
    //graphics resets must not happen here. For that, we have the messenger variable 'reset_gpu_essential' (see function render_3D_content() in the renderer3D.h).
    void setup(const solution &sol)
    {
        this->sol = sol; //Obtain a solution copy for the 3D rendering.
        this->sol2D = sol.get_reduced_solution(PLOT_POINTS_2D); //Then create a downsampled solution for the 2D plots.

        float binary_max_dist = *std::max_element(sol.dist.begin(), sol.dist.end());
        rend3D.sunlight.reset(sol.integr.brillouin1 + sol.integr.brillouin2 + binary_max_dist);
        rend3D.cam.reset(sol.integr.brillouin1 + sol.integr.brillouin2, binary_max_dist);

        current_frame = 0;
        total_frames = static_cast<uint64_t>(sol.t.size());
        play_pause_video = false; //Set the video at paused state ('true' means playing, 'false' means paused).
        reset_gpu_essential = true;

        uint64_t init_orb_count = (total_frames > 0 ? 1 : 0);
        rend3D.orb1.draw_count = rend3D.orb2.draw_count = init_orb_count;
        //At every new simulation, if the user does not assume a 3rd body spacecraft, then any previous plots regarding the 3rd body shall disappear.
        if (!sol.integr.properties.spacecraft_checkbox)
        {
            plot_cart_sp = {false,false,false};
            rend3D.orb_sp.draw_count = 0;
            orb_sp_sync = false;
            rend3D.render_orb_sp = false;
        }
        else
            rend3D.orb_sp.draw_count = init_orb_count;
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

    //This function plots the data {t,f(t)}.
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
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f, ImColor(255,0,0,255), 1.0f, ImColor(255,0,0,255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &plot_func.back(), 1);
            }
            else if (sol2D.integr.collision_sp)
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f, ImColor(255, 100, 0, 255), 1.0f, ImColor(255, 100, 0, 255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &plot_func.back(), 1);
            }

            ImPlot::EndPlot();
        }
        ImGui::End();
        return bool_plot_func;
    }

    bool ImGuiSliderFloat2D(const char *label, ImVec2 *value, ImVec2 min, ImVec2 max, ImVec2 size = ImVec2(100.0f,100.0f))
    {
        ImGui::Text("%s", label);

        //First we set it and then we get it.
        ImGui::SetCursorPosX(0.15f*ImGui::GetIO().DisplaySize.x/2.0f - size.x/2.0f);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        //Use a framed button so hover/disabled visuals match other controls.
        ImGui::InvisibleButton(label, size);

        bool changed = false;
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0))
        {
            ImVec2 mouse = ImGui::GetIO().MousePos;
            float x = (mouse.x - pos.x)/size.x;
            float y = (mouse.y - pos.y)/size.y;
            value->x = min.x + x*(max.x - min.x);
            value->y = min.y + y*(max.y - min.y);
            changed = true;
        }

        //Clamp
        value->x = std::clamp(value->x, min.x, max.x);
        value->y = std::clamp(value->y, min.y, max.y);

        //Colors that respect BeginDisabled() dimming.
        ImU32 col_bg     = ImGui::GetColorU32(ImGuiCol_FrameBg);
        ImU32 col_border = ImGui::GetColorU32(ImGuiCol_Border);
        ImU32 col_handle = ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);

        //Draw background & border.
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), col_bg, 0.0f); //Interior.
        draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), col_border); //Border.

        //Handle.
        float tx = (value->x - min.x)/(max.x - min.x);
        float ty = (value->y - min.y)/(max.y - min.y);
        ImVec2 handle = ImVec2(pos.x + tx*size.x, pos.y + ty*size.y);
        draw_list->AddCircleFilled(handle, 5.0f, col_handle);

        return changed;
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

        if (ImGui::TreeNodeEx("Spacecraft orbiter", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (!sol.integr.properties.spacecraft_checkbox)
                ImGui::BeginDisabled();
            
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Position");
            plot_cart_sp[0] = common_onoff_button("xs##37", ImVec2(50.0f, 20.0f), plot_cart_sp[0]); ImGui::SameLine();
            plot_cart_sp[1] = common_onoff_button("ys##38", ImVec2(50.0f, 20.0f), plot_cart_sp[1]); ImGui::SameLine();
            plot_cart_sp[2] = common_onoff_button("zs##39", ImVec2(50.0f, 20.0f), plot_cart_sp[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            if (!sol.integr.properties.spacecraft_checkbox)
                ImGui::EndDisabled();

            ImGui::TreePop();
        }
        ImGui::PopStyleVar();
    }

    void render_scene_buttons()
    {
        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Text("Content state");
        render_scene = common_onoff_button("Render##40", ImVec2(80.0f, 25.0f), render_scene);
        ImGui::SameLine();

        if (!render_scene)
        {
            ImGui::BeginDisabled();
            ImGui::Button("Play/Pause", ImVec2(80.0f, 25.0f));
            ImGui::SameLine();
            auto_replay = common_onoff_button(ICON_FA_REDO " Auto", ImVec2(55.0f, 25.0f), auto_replay);
            ImGui::EndDisabled();
        }
        else
        {
            ImVec4 play_pause_col = play_pause_video ? ImVec4(0.0f, 0.7f, 0.0f, 1.0f) : ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button,        play_pause_col);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(play_pause_col.x+0.2f, play_pause_col.y+0.2f, play_pause_col.z+0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(play_pause_col.x*0.8f, play_pause_col.y*0.8f, play_pause_col.z*0.8f, 1.0f));
            if (ImGui::Button("Play/Pause", ImVec2(80.0f, 25.0f)))
                play_pause_video = !play_pause_video;
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            auto_replay = common_onoff_button(ICON_FA_REDO " Auto", ImVec2(55.0f, 25.0f), auto_replay);
            if (auto_replay && play_pause_video && current_frame == total_frames - 1)
                current_frame = 0;
        }

        if (!render_scene)
            ImGui::BeginDisabled();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Text("Frame");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f);
        uint64_t visible_min_frame = (total_frames > 0) ? 1 : 0;
        //We don't need a 'visible_max_frame' variable, as this is equal to 'total_frames';
        uint64_t visible_current_frame = (total_frames > 0) ? (current_frame + 1) : 0; //Show 1-based frame.
        ImGui::SliderScalar("##41", ImGuiDataType_U64, &visible_current_frame, &visible_min_frame, &total_frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        //Rule is : the above slider controls the frames and then the 'current_frame' is updated accordingly.
        current_frame = (visible_current_frame > 0) ? (visible_current_frame - 1) : 0; //Back to 0-based frame.

        ImGui::Text("Rate");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f);
        ImGui::SliderInt("[Hz]##42", &frame_rate, 0, 60, "%d");
        if (!sol.t.size())
            ImGui::Text("Time : 0.00  [days]");
        else
            ImGui::Text("Time : %.2f  [days]", (float)sol.t[current_frame]);

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Camera setup");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Barycentric frame view");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        if (rend3D.cam.mount_body1 || rend3D.cam.mount_body2)
            ImGui::BeginDisabled();

        ImGui::Text("Dist");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[km]##43", &rend3D.cam.dist, rend3D.cam.min_dist, rend3D.cam.max_dist, "%.3f", ImGuiSliderFlags_Logarithmic);

        ImGui::Text("Lon");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##44", &rend3D.cam.lon, 0.0f, 360.0f, "%.1f");

        ImGui::Text("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##45", &rend3D.cam.lat, 0.0f, 180.0f, "%.1f");

        if (rend3D.cam.mount_body1 || rend3D.cam.mount_body2)
            ImGui::EndDisabled();

        ImGui::Text("FoV");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##46", &rend3D.cam.fov, 1.0f, 179.0f, "%.0f");

        ImGui::Dummy(ImVec2(0.0f, 6.0f));
        ImGui::Text("Revolving frame view");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Mount Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        if (ImGui::Checkbox("##47", &rend3D.cam.mount_body1))
        {
            rend3D.cam.mount_body2 = false;
            rend3D.cam.voffset_ndc = glm::vec2(0.0f); //Recenter peek offset.
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(140.0f);
        ImGui::Text("Mount Body 2");
        ImGui::SameLine();
        if (ImGui::Checkbox("##48", &rend3D.cam.mount_body2))
        {
            rend3D.cam.mount_body1 = false;
            rend3D.cam.voffset_ndc = glm::vec2(0.0f); //Recenter peek offset.
        }
        ImGui::Dummy(ImVec2(0.0f, 3.0f));

        if (!rend3D.cam.mount_body1 && !rend3D.cam.mount_body2)
            ImGui::BeginDisabled();

        ImGui::Text("R - offset");
        ImGui::SameLine();
        ImGui::SetCursorPosX(70.0f);
        ImGui::SetNextItemWidth(130);
        ImGui::SliderFloat("[Brillouin]##49", &rend3D.cam.rscale, 2.0f, 10.0f, "%.1f");

        ImGui::Text("V - scale");
        ImGui::SameLine();
        ImGui::SetCursorPosX(70.0f);
        ImGui::SetNextItemWidth(130);
        ImGui::SliderFloat("[Brillouin]##50", &rend3D.cam.vscale, 0.0f, 5.0f, "%.1f");

        //2D joystick. Used to shift the mounted camera left-right-up-down from the radial direction so that the body in front does not block the view.
        ImGuiSliderFloat2D("V - offset", (ImVec2*)&rend3D.cam.voffset_ndc, ImVec2(-1.0f,-1.0f), ImVec2(1.0f,1.0f));

        if (!rend3D.cam.mount_body1 && !rend3D.cam.mount_body2)
            ImGui::EndDisabled();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Sun direction");

        ImGui::Text("Lon");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##51", &rend3D.sunlight.lon, 0.0f, 360.0f, "%.1f");

        ImGui::Text("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##52", &rend3D.sunlight.lat, 0.0f, 180.0f, "%.1f");

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Shadow map");

        ImGui::Text("Reso");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        if (ImGui::SliderInt("[pix]##53", &rend3D.depth_reso, 1024, 16384))
            rend3D.setup_depth_fbo();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        ImGui::Text("Visible meshes");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Asteroids");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##54", &rend3D.render_aster1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::Text("Axes 1");
        ImGui::SameLine();
        ImGui::Checkbox("##55", &rend3D.render_axes1);

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##56", &rend3D.render_aster2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::Text("Axes 2");
        ImGui::SameLine();
        ImGui::Checkbox("##57", &rend3D.render_axes2);
        ImGui::Dummy(ImVec2(0.0f,0.6f));

        ImGui::Text("Orbits");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##58", &rend3D.render_orb1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f);
        ImGui::SetNextItemWidth(100);
        uint64_t visible_orb1_frame = (total_frames > 0) ? static_cast<uint64_t>(rend3D.orb1.draw_count) : 0;
        ImGui::SliderScalar("##59", ImGuiDataType_U64, &visible_orb1_frame, &visible_min_frame, &total_frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        rend3D.orb1.draw_count = static_cast<size_t>(visible_orb1_frame);
        orb1_sync = common_onoff_button("Sync##60", ImVec2(50.0f, 18.0f), orb1_sync);
        if (orb1_sync)
            rend3D.orb1.draw_count = static_cast<size_t>(current_frame + 1);

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##61", &rend3D.render_orb2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f);
        ImGui::SetNextItemWidth(100);
        uint64_t visible_orb2_frame = (total_frames > 0) ? static_cast<uint64_t>(rend3D.orb2.draw_count) : 0;
        ImGui::SliderScalar("##62", ImGuiDataType_U64, &visible_orb2_frame, &visible_min_frame, &total_frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb2_sync = common_onoff_button("Sync##63", ImVec2(50.0f, 18.0f), orb2_sync);
        rend3D.orb2.draw_count = static_cast<size_t>(visible_orb2_frame);
        if (orb2_sync)
            rend3D.orb2.draw_count = static_cast<size_t>(current_frame + 1);


        if (!sol.integr.properties.spacecraft_checkbox)
                ImGui::BeginDisabled();

        ImGui::Text("Orbiter");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##64", &rend3D.render_orb_sp);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f);
        ImGui::SetNextItemWidth(100);
        uint64_t visible_orb_sp_frame = (total_frames > 0) ? static_cast<uint64_t>(rend3D.orb_sp.draw_count) : 0;
        ImGui::SliderScalar("##65", ImGuiDataType_U64, &visible_orb_sp_frame, &visible_min_frame, &total_frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb_sp_sync = common_onoff_button("Sync##66", ImVec2(50.0f, 18.0f), orb_sp_sync);
        rend3D.orb_sp.draw_count = static_cast<size_t>(visible_orb_sp_frame);
        if (orb_sp_sync)
            rend3D.orb_sp.draw_count = static_cast<size_t>(current_frame + 1);


        if (!sol.integr.properties.spacecraft_checkbox)
                ImGui::EndDisabled();

        ImGui::Dummy(ImVec2(0.0f,700.0f)); //Some extra y-space in order to be able to scroll down along scene panel.        

        if (!render_scene)
            ImGui::EndDisabled();

        ImGuiIO &io = ImGui::GetIO();
        if (render_scene && !io.WantCaptureMouse)
        {
            if (io.MouseWheel != 0.0f)
            {
                if (io.KeyCtrl)
                    rend3D.cam.scroll_fov(io.MouseWheel);
                else if (!rend3D.cam.mount_body1 && !rend3D.cam.mount_body2)
                    rend3D.cam.scroll_dist(io.MouseWheel);
                else if (rend3D.cam.mount_body1 || rend3D.cam.mount_body2)
                    rend3D.cam.scroll_dist_body(io.MouseWheel);
            }

            if (io.MouseDown[ImGuiMouseButton_Middle])
            {
                const ImVec2 d = io.MouseDelta;
                if (d.x != 0.0f || d.y != 0.0f)
                {
                    if (io.KeyCtrl)
                        rend3D.sunlight.rotate_lon_lat(d.x, d.y);
                    else if (!rend3D.cam.mount_body1 && !rend3D.cam.mount_body2)
                        rend3D.cam.rotate_lon_lat(d.x, d.y);
                    else if (rend3D.cam.mount_body1 || rend3D.cam.mount_body2)
                        rend3D.cam.translate_on_vplane(d.x, d.y, rend3D.win_width, rend3D.win_height);
                }
            }
        }

        if (play_pause_video && render_scene && current_frame < total_frames - 1)
        {
            if (frame_rate == 0) //The slider is set to 0 => paused. Do not increment current_frame.
            {
                //Pass.
            }
            else if (frame_rate < 60) //We do a time-based step to achieve the chosen frame_rate.
            {
                frame_accumulator += ImGui::GetIO().DeltaTime;
                float step = 1.0f/static_cast<float>(frame_rate);

                //In case dt is large (e.g. if the user drags the window), use a while() so we don't 'miss' increments.
                while (frame_accumulator >= step && current_frame < total_frames - 1)
                {
                    current_frame++;
                    frame_accumulator -= step;
                }
            }
            else //frame_rate == 60 => let it play as fast as the machine can handle, i.e. increment every time we render.
                current_frame++;
        }
        
        //As a final step, render the 3D content under the constraints implied by 'render_scene' variable.
        if (render_scene)
            rend3D.render_3D_content(sol, current_frame, reset_gpu_essential);
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
            if (!sol.t.size()) //Criterion that applies when the simulation hasn't been performed.
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

                if (sol.integr.properties.spacecraft_checkbox)
                {
                    if (plot_cart_sp[0]) plot_cart_sp[0] = common_plot("##137", "Spacecraft x", "xs [km]", plot_cart_sp[0], sol2D.x_sp);
                    if (plot_cart_sp[1]) plot_cart_sp[1] = common_plot("##138", "Spacecraft y", "ys [km]", plot_cart_sp[1], sol2D.y_sp);
                    if (plot_cart_sp[2]) plot_cart_sp[2] = common_plot("##139", "Spacecraft z", "zs [km]", plot_cart_sp[2], sol2D.z_sp);
                }

            }
            ImGui::PopStyleColor();
        }
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Video 3D"))
        {
            if (!sol.t.size()) //Criterion that applies when the simulation hasn't been performed.
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
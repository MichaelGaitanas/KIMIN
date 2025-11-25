/* This class handles the rendering logic of the right panel (scene) in the gui. */

#ifndef SCENE_PANEL_H
#define SCENE_PANEL_H

#include<algorithm>

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include"constants.h"
#include"typedef.h"
#include"solution.h"
#include"renderer3D.h"
#include"icons.h"

class scene_panel
{
private:
    bvec plot_cart; //Buttons : [x, y, z, r] and [υx, υy, υz, υ].
    bvec plot_kep; //Buttons : [a, e, i, Ω, ω, M].
    bvec plot_rpy1, plot_rpy2; //Buttons : [roll 1, pitch 1, yaw 1, libration 1] and [roll 2, pitch 2, yaw 2, libration 1].
    bvec plot_w1i, plot_w1b; //Buttons : [ω1ix, ω1iy, ω1iz] and [ω1bx, ω1by, ω1bz].
    bvec plot_w2i, plot_w2b; //Buttons : [ω2ix, ω2iy, ω2iz] and [ω2bx, ω2by, ω2bz].
    bvec plot_dener_dmom; //Buttons : [energy, momentum].
    bvec plot_rsp; //Buttons : [xsp, ysp, zsp].
    
    bool render_scene, play_video, reset_gpu_essential, auto_replay, orb1_sync, orb2_sync, orb_sp_sync;
    uint64_t iframe, frames;
    int framerate; //Frame updates per second.
    float frame_accumulator; //Accumulates fractional frames between updates.

    solution *sol; //This contains all the orbital data and is used to render the 3D scene (pointer to avoid huge copy).
    solution sol2D; //And this is the downsampled version of the sol, used only for the 2D plots.

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
                    plot_dener_dmom({false,false}),
                    plot_rsp({false,false,false}),
                    render_scene(false),
                    play_video(false),
                    reset_gpu_essential(false),
                    auto_replay(false),
                    orb1_sync(false),
                    orb2_sync(false),
                    orb_sp_sync(false),
                    iframe(0),
                    frames(0),
                    framerate(60),
                    frame_accumulator(0.0f),
                    sol(nullptr),
                    sol2D(),
                    rend3D()
    { }

    //Reset essential stuff upon a simulation termination.
    //Note : apart from the following resets, we still have to reset OpenGL stuff. But the following setup() function is gonna run in
    //the 'task_thread' thread defined in gui.h, not in the main thread, where OpenGL runs. So any gl* commands that handle
    //gpu resets must not happen here. For that, we have the messenger variable 'reset_gpu_essential' (see function render_3D_content() in the renderer3D.h).
    void setup(solution &s)
    {
        sol = &s; //Obtain a copy of the adress, not a full deep copy!
        sol2D = sol->get_reduced_solution(); //Then create a downsampled solution for the 2D plots.

        float mutual_max_dist = *std::max_element(sol->dist.begin(), sol->dist.end());
        rend3D.cam.reset(sol->integr.brillouin1 + sol->integr.brillouin2, mutual_max_dist);

        iframe = 0;
        frames = static_cast<uint64_t>(sol->t.size());
        play_video = false; //Set the video at paused state ('true' means play, 'false' means pause).
        reset_gpu_essential = true; //This will inform the renderer3D::reset_gpu_resources() to run, but only once.

        uint64_t init_orb_count = (frames > 0 ? 1 : 0);
        rend3D.orb1.draw_count = rend3D.orb2.draw_count = init_orb_count;
        //At every new simulation, if the user does not assume a 3rd body spacecraft, then any previous plots regarding the 3rd body shall disappear.
        if (!sol->integr.properties.spacecraft_checkbox)
        {
            plot_rsp = {false,false,false};
            rend3D.orb_sp.draw_count = 0;
            orb_sp_sync = false;
            rend3D.render_orb_sp = false;
        }
        else
            rend3D.orb_sp.draw_count = init_orb_count;
    }

private:
    //This function controls the on/off logic of a clickable button in the gui.
    bool onoff_button(const char *label, const ImVec2 &dimensions, bool state)
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

    //Because the 'sol2D' (the one used for 2D plotting) is reduced in size compared to the 'sol' (the one used for exporting or 3D rendering), we have to map
    //the 'iframe' index to another index 'jframe', so that the scatter point of the current frame corresponds to the correct time.
    //This function serves the aforementioned purpose. 
    inline size_t map_frame_to_reduced_sol(const size_t iframe, const size_t original_size, const size_t reduced_size)
    {
        //Edge cases :
        if (reduced_size == 0 || original_size <= 1 || reduced_size <= 1)
            return 0;
        
        double step = (original_size - 1.0)/(reduced_size - 1.0);
        size_t jframe = (size_t)std::floor(iframe/step + 0.5); //Round to nearest integer.
        if (jframe >= reduced_size)
            jframe = reduced_size - 1; //Clamp to avoid wrong access.
    
        return jframe;
    }

    //This function plots the data {t, data(t)}.
    bool plot(const char *imgui_id, const char *implot_id, const char *yaxis_str, bool plot_status, dvec &data)
    {
        ImGui::SetNextWindowPos( ImVec2(0.6f*ImGui::GetIO().DisplaySize.x, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.25f*ImGui::GetIO().DisplaySize.x, 0.4f*ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin(imgui_id, &plot_status);
        ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        if (ImPlot::BeginPlot(implot_id, plot_win_size))
        {
            //Line logic :
            ImPlot::SetupAxes("time [days]", yaxis_str);
            ImPlot::PlotLine("", &sol2D.t[0], &data[0], sol2D.t.size());
            
            //Current frame marker logic :
            size_t jframe = map_frame_to_reduced_sol(iframe, sol->t.size(), sol2D.t.size());
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6.0f, ImColor(0, 255, 0, 255), 1.0f, ImColor(0, 255, 0, 255));
            ImPlot::PlotScatter("Current frame", &sol2D.t[jframe], &data[jframe], 1);

            //Collision frame marker logic :
            if (sol2D.integr.collision) //Asteroid-asteroid collision.
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f, ImColor(255,0,0,255), 1.0f, ImColor(255,0,0,255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &data.back(), 1);
            }
            else if (sol2D.integr.collision_sp) //Asteroid-spacecraft collision.
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f, ImColor(255, 100, 0, 255), 1.0f, ImColor(255, 100, 0, 255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &data.back(), 1);
            }

            ImPlot::EndPlot();
        }
        ImGui::End();
        return plot_status;
    }

    //Since imgui does not provide a 2D slider - joystick, we emulate one ourselves.
    bool imgui_slider_float_2D(const char *label, const char *hash, ImVec2 *value, ImVec2 min, ImVec2 max, ImVec2 size = ImVec2(100.0f,100.0f))
    {
        ImGui::Text("%s", label);
        ImGui::SetCursorPosX(0.15f*ImGui::GetIO().DisplaySize.x/2.0f - size.x/2.0f);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(hash, size); //We use a framed button so hover/disabled visuals match other controls.

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

        //Circular handle logic :
        float tx = (value->x - min.x)/(max.x - min.x);
        float ty = (value->y - min.y)/(max.y - min.y);
        ImVec2 handle = ImVec2(pos.x + tx*size.x, pos.y + ty*size.y);
        draw_list->AddCircleFilled(handle, 5.0f, col_handle);

        return changed;
    }
    
    //Render on the gui the 2D plot buttons.
    void render_plot_buttons()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f); //Disable the indentation for what comes next.

        //Binary's (mutual) plots.
        if (ImGui::TreeNodeEx("Mutual"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Position");
            plot_cart[0] = onoff_button("x##plot_cart[0]", ImVec2(50.0f, 20.0f), plot_cart[0]); ImGui::SameLine();
            plot_cart[1] = onoff_button("y##plot_cart[1]", ImVec2(50.0f, 20.0f), plot_cart[1]); ImGui::SameLine();
            plot_cart[2] = onoff_button("z##plot_cart[2]", ImVec2(50.0f, 20.0f), plot_cart[2]); ImGui::SameLine();
            plot_cart[3] = onoff_button("r##plot_cart[3]", ImVec2(50.0f, 20.0f), plot_cart[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Velocity");
            plot_cart[4] = onoff_button("υx##plot_cart[4]", ImVec2(50.0f, 20.0f), plot_cart[4]); ImGui::SameLine();
            plot_cart[5] = onoff_button("υy##plot_cart[5]", ImVec2(50.0f, 20.0f), plot_cart[5]); ImGui::SameLine();
            plot_cart[6] = onoff_button("υz##plot_cart[6]", ImVec2(50.0f, 20.0f), plot_cart[6]); ImGui::SameLine();
            plot_cart[7] = onoff_button("υ##plot_cart[7]" , ImVec2(50.0f, 20.0f), plot_cart[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Keplerian elements");
            plot_kep[0] = onoff_button("a##plot_kep[0]", ImVec2(35.0f, 20.0f), plot_kep[0]); ImGui::SameLine();
            plot_kep[1] = onoff_button("e##plot_kep[1]", ImVec2(35.0f, 20.0f), plot_kep[1]); ImGui::SameLine();
            plot_kep[2] = onoff_button("i##plot_kep[2]", ImVec2(35.0f, 20.0f), plot_kep[2]); ImGui::SameLine();
            plot_kep[3] = onoff_button("Ω##plot_kep[3]", ImVec2(35.0f, 20.0f), plot_kep[3]); ImGui::SameLine();
            plot_kep[4] = onoff_button("ω##plot_kep[4]", ImVec2(35.0f, 20.0f), plot_kep[4]); ImGui::SameLine();
            plot_kep[5] = onoff_button("M##plot_kep[5]", ImVec2(35.0f, 20.0f), plot_kep[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Energy and momentum errors");
            plot_dener_dmom[0] = onoff_button("energy##plot_dener_dmom[0]",   ImVec2(80.0f, 25.0f), plot_dener_dmom[0]); ImGui::SameLine();
            plot_dener_dmom[1] = onoff_button("momentum##plot_dener_dmom[1]", ImVec2(80.0f, 25.0f), plot_dener_dmom[1]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::TreePop();
        }

        //Body 1 plots.
        if (ImGui::TreeNodeEx("Body 1"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Euler angles (XYZ)");
            plot_rpy1[0] = onoff_button("roll##plot_rpy1[0]",      ImVec2(50.0f, 20.0f), plot_rpy1[0]); ImGui::SameLine();
            plot_rpy1[1] = onoff_button("pitch##plot_rpy1[1]",     ImVec2(50.0f, 20.0f), plot_rpy1[1]); ImGui::SameLine();
            plot_rpy1[2] = onoff_button("yaw##plot_rpy1[2]",       ImVec2(50.0f, 20.0f), plot_rpy1[2]); ImGui::SameLine();
            plot_rpy1[3] = onoff_button("rel. yaw##plot_rpy1[3]",  ImVec2(60.0f, 20.0f), plot_rpy1[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Angular velocity (inertial frame)");
            plot_w1i[0] = onoff_button("ωx##plot_w1i[0]", ImVec2(50.0f, 20.0f), plot_w1i[0]); ImGui::SameLine();
            plot_w1i[1] = onoff_button("ωy##plot_w1i[1]", ImVec2(50.0f, 20.0f), plot_w1i[1]); ImGui::SameLine();
            plot_w1i[2] = onoff_button("ωz##plot_w1i[2]", ImVec2(50.0f, 20.0f), plot_w1i[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            
            ImGui::Text("Angular velocity (body frame)");
            plot_w1b[0] = onoff_button("ω1##plot_w1b[0]", ImVec2(50.0f, 20.0f), plot_w1b[0]); ImGui::SameLine();
            plot_w1b[1] = onoff_button("ω2##plot_w1b[1]", ImVec2(50.0f, 20.0f), plot_w1b[1]); ImGui::SameLine();
            plot_w1b[2] = onoff_button("ω3##plot_w1b[2]", ImVec2(50.0f, 20.0f), plot_w1b[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::TreePop();
        }

        //Body 2 plots.
        if (ImGui::TreeNodeEx("Body 2"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Euler angles (XYZ)");
            plot_rpy2[0] = onoff_button("roll##plot_rpy2[0]",      ImVec2(50.0f, 20.0f), plot_rpy2[0]); ImGui::SameLine();
            plot_rpy2[1] = onoff_button("pitch##plot_rpy2[1]",     ImVec2(50.0f, 20.0f), plot_rpy2[1]); ImGui::SameLine();
            plot_rpy2[2] = onoff_button("yaw##plot_rpy2[2]",       ImVec2(50.0f, 20.0f), plot_rpy2[2]); ImGui::SameLine();
            plot_rpy2[3] = onoff_button("rel. yaw##plot_rpy2[3]",  ImVec2(60.0f, 20.0f), plot_rpy2[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::Text("Angular velocity (inertial frame)");
            plot_w2i[0] = onoff_button("ωx##plot_w2i[0]", ImVec2(50.0f, 20.0f), plot_w2i[0]); ImGui::SameLine();
            plot_w2i[1] = onoff_button("ωy##plot_w2i[1]", ImVec2(50.0f, 20.0f), plot_w2i[1]); ImGui::SameLine();
            plot_w2i[2] = onoff_button("ωz##plot_w2i[2]", ImVec2(50.0f, 20.0f), plot_w2i[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            
            ImGui::Text("Angular velocity (body frame)");
            plot_w2b[0] = onoff_button("ω1##plot_w2b[0]", ImVec2(50.0f, 20.0f), plot_w2b[0]); ImGui::SameLine();
            plot_w2b[1] = onoff_button("ω2##plot_w2b[1]", ImVec2(50.0f, 20.0f), plot_w2b[1]); ImGui::SameLine();
            plot_w2b[2] = onoff_button("ω3##plot_w2b[2]", ImVec2(50.0f, 20.0f), plot_w2b[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));

            ImGui::TreePop();
        }

        //Spacecraft's plots.
        if (ImGui::TreeNodeEx("Spacecraft orbiter"))
        {
            if (!sol || sol->t.empty() || !sol->integr.properties.spacecraft_checkbox)
                ImGui::BeginDisabled();
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            ImGui::Text("Position (binary's C.O.M. frame)");
            plot_rsp[0] = onoff_button("x##plot_rsp[0]", ImVec2(50.0f, 20.0f), plot_rsp[0]); ImGui::SameLine();
            plot_rsp[1] = onoff_button("y##plot_rsp[1]", ImVec2(50.0f, 20.0f), plot_rsp[1]); ImGui::SameLine();
            plot_rsp[2] = onoff_button("z##plot_rsp[2]", ImVec2(50.0f, 20.0f), plot_rsp[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f));
            if (!sol || sol->t.empty() || !sol->integr.properties.spacecraft_checkbox)
                ImGui::EndDisabled();

            ImGui::TreePop();
        }

        ImGui::PopStyleVar();
    }

    //Render on the gui the buttons that correspond to the 3D scene.
    void render_scene_buttons()
    {
        //Content state logic :

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Text("Content state");
        render_scene = onoff_button("Render##render_scene", ImVec2(80.0f, 25.0f), render_scene);
        ImGui::SameLine();

        if (!render_scene)
        {
            ImGui::BeginDisabled();
            play_video = onoff_button("Play/Pause##play_video", ImVec2(80.0f, 25.0f), play_video);
            ImGui::SameLine();
            auto_replay = onoff_button(ICON_FA_REDO" Auto##auto_replay", ImVec2(55.0f, 25.0f), auto_replay);
            ImGui::EndDisabled();
        }
        else
        {
            ImVec4 play_pause_col = play_video ? ImVec4(0.0f, 0.7f, 0.0f, 1.0f) : ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button,        play_pause_col);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(play_pause_col.x+0.2f, play_pause_col.y+0.2f, play_pause_col.z+0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(play_pause_col.x*0.8f, play_pause_col.y*0.8f, play_pause_col.z*0.8f, 1.0f));
            play_video = onoff_button("Play/Pause##play_video", ImVec2(80.0f, 25.0f), play_video);
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            ImVec4 auto_replay_col = auto_replay ? ImVec4(0.0f, 0.7f, 0.0f, 1.0f) : ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button,        auto_replay_col);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(auto_replay_col.x+0.2f, auto_replay_col.y+0.2f, auto_replay_col.z+0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(auto_replay_col.x*0.8f, auto_replay_col.y*0.8f, auto_replay_col.z*0.8f, 1.0f));
            auto_replay = onoff_button(ICON_FA_REDO" Auto##auto_replay", ImVec2(55.0f, 25.0f), auto_replay);
            ImGui::PopStyleColor(3);
            if (auto_replay && play_video && iframe >= frames - 1)
                iframe = 0;
        }

        if (!render_scene)
            ImGui::BeginDisabled();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Text("Frame");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f);
        uint64_t visible_min_frame = (frames > 0) ? 1 : 0;
        uint64_t visible_iframe = (frames > 0) ? (iframe + 1) : 0; //Display in the gui 1-based frame (instead of 0-based, which is used in the arrays as index).
        ImGui::SliderScalar("##visible_iframe", ImGuiDataType_U64, &visible_iframe, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        //Rule is : the above slider controls the frames and then the 'iframe' is updated accordingly, but into 0-based frame, because it is an index.
        iframe = (visible_iframe > 0) ? (visible_iframe - 1) : 0;
        ImGui::Text("Rate");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f);
        ImGui::SliderInt("[Hz]##framerate", &framerate, 0, 60, "%d");
        if (!sol || sol->t.empty())
            ImGui::Text("Time : 0.00  [days]");
        else
            ImGui::Text("Time : %.2f  [days]", static_cast<float>(sol->t[iframe]));
        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        //Camera setup logic :

        ImGui::Text("Camera setup");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        //Frame view (World, Barycentric, Body 1, Body 2).
        ImGui::Text("Frame view");
        ImGui::PushItemWidth(250.0f);
        static const char *cam_frames[4] = {"World", "Center of mass", "Body 1", "Body 2"};
        ImGui::Combo("##rend3D.cam.frame_of_ref", (int*)(&rend3D.cam.frame_of_ref), cam_frames, IM_ARRAYSIZE(cam_frames));
        ImGui::PopItemWidth();

        if (rend3D.cam.frame_of_ref == camera::WORLD || rend3D.cam.frame_of_ref == camera::COM)
        {
            ImGui::Text("Dist");
            ImGui::SameLine();
            ImGui::SetCursorPosX(40.0f);
            ImGui::SliderFloat("[km]##rend3D.cam.dist_world_or_com", &rend3D.cam.get_active_dist(), rend3D.cam.min_dist, rend3D.cam.max_dist, "%.3f", ImGuiSliderFlags_Logarithmic);

            ImGui::Text("Lon");
            ImGui::SameLine();
            ImGui::SetCursorPosX(40.0f);
            ImGui::SliderFloat("[deg]##rend3D.cam.lon_world_or_com", &rend3D.cam.get_active_lon(), 0.0f, 360.0f, "%.1f");

            ImGui::Text("Lat");
            ImGui::SameLine();
            ImGui::SetCursorPosX(40.0f);
            ImGui::SliderFloat("[deg]##rend3D.cam.lat_world_or_com", &rend3D.cam.get_active_lat(), 0.0f, 180.0f, "%.1f");
        }
        else //camera::BODY1 or camera::BODY2
        {
            ImGui::Text("R - offset");
            ImGui::SameLine();
            ImGui::SetCursorPosX(70.0f);
            ImGui::SetNextItemWidth(130);
            ImGui::SliderFloat("[Brillouin]##rend3D.cam.rscale", &rend3D.cam.rscale, 3.0f, 10.0f, "%.1f");

            ImGui::Text("V - scale");
            ImGui::SameLine();
            ImGui::SetCursorPosX(70.0f);
            ImGui::SetNextItemWidth(130);
            ImGui::SliderFloat("[Brillouin]##rend3D.cam.vscale", &rend3D.cam.vscale, 0.0f, 5.0f, "%.1f");

            //This is a 2D joystick, used to shift the mounted camera left-right-up-down from the radial direction so that the body in front does not block the view.
            //rend3D.cam.voffset_ndc.x = rend3D.cam.voffset_ndc.y = 0.0f; //Recenter joystick.
            imgui_slider_float_2D("V - offset", "##rend3D.cam.voffset_ndc", (ImVec2*)&rend3D.cam.voffset_ndc, ImVec2(-1.0f,-1.0f), ImVec2(1.0f,1.0f));
        }
        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        ImGui::Text("FoV");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##rend3D.cam.fov", &rend3D.cam.fov, CAM_MIN_FOV, CAM_MAX_FOV, "%.0f");

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        //Sun setup logic :

        ImGui::Text("Sun direction");

        ImGui::Text("Lon");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##rend3D.sunlight.lon", &rend3D.sunlight.lon, 0.0f, 360.0f, "%.1f");

        ImGui::Text("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        ImGui::SliderFloat("[deg]##rend3D.sunlight.lat", &rend3D.sunlight.lat, 0.0f, 180.0f, "%.1f");

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        //Shadow setup logic :

        ImGui::Text("Shadow map");

        ImGui::Text("Reso");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f);
        if (ImGui::SliderInt("[pix]##rend3D.depth_reso", &rend3D.depth_reso, DEPTH_RESO_MIN, DEPTH_RESO_MAX))
            rend3D.setup_depth_fbo();

        ImGui::Dummy(ImVec2(0.0f, 7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f));

        //Meshes to render logic :

        ImGui::Text("Visible meshes");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Bodies");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##rend3D.render_body1", &rend3D.render_body1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::Text("Axes 1");
        ImGui::SameLine();
        ImGui::Checkbox("##rend3D.render_axes1", &rend3D.render_axes1);

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##rend3D.render_body2", &rend3D.render_body2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f);
        ImGui::Text("Axes 2");
        ImGui::SameLine();
        ImGui::Checkbox("##rend3D.render_axes2", &rend3D.render_axes2);
        ImGui::Dummy(ImVec2(0.0f,4.0f));

        ImGui::Text("Orbits");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.2f,1.0f)); //Make all the plot buttons' off state gray.

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##rend3D.render_orb1", &rend3D.render_orb1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f);
        ImGui::SetNextItemWidth(100);
        uint64_t visible_orb1_frame = (frames > 0) ? static_cast<uint64_t>(rend3D.orb1.draw_count) : 0;
        
        if (!rend3D.render_orb1)
        {
            ImGui::BeginDisabled();
            if (orb1_sync)
                orb1_sync = false;
        }
        
        ImGui::SliderScalar("##visible_orb1_frame", ImGuiDataType_U64, &visible_orb1_frame, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        rend3D.orb1.draw_count = static_cast<size_t>(visible_orb1_frame);
        orb1_sync = onoff_button("Sync##orb1_sync", ImVec2(50.0f, 18.0f), orb1_sync);
        if (orb1_sync)
            rend3D.orb1.draw_count = static_cast<size_t>(iframe + 1);

        if (!rend3D.render_orb1)
            ImGui::EndDisabled();

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##rend3D.render_orb2", &rend3D.render_orb2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f);
        ImGui::SetNextItemWidth(100);
        uint64_t visible_orb2_frame = (frames > 0) ? static_cast<uint64_t>(rend3D.orb2.draw_count) : 0;

        if (!rend3D.render_orb2)
        {
            ImGui::BeginDisabled();
            if (orb2_sync)
                orb2_sync = false;
        }

        ImGui::SliderScalar("##visible_orb2_frame", ImGuiDataType_U64, &visible_orb2_frame, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb2_sync = onoff_button("Sync##visible_orb2_frame", ImVec2(50.0f, 18.0f), orb2_sync);
        rend3D.orb2.draw_count = static_cast<size_t>(visible_orb2_frame);
        if (orb2_sync)
            rend3D.orb2.draw_count = static_cast<size_t>(iframe + 1);

        if (!rend3D.render_orb2)
            ImGui::EndDisabled();


        if (!sol || sol->t.empty() || !sol->integr.properties.spacecraft_checkbox)
            ImGui::BeginDisabled();

        ImGui::Text("Orbiter");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##rend3D.render_orb_sp", &rend3D.render_orb_sp);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f);
        ImGui::SetNextItemWidth(100);
        uint64_t visible_orb_sp_frame = (frames > 0) ? static_cast<uint64_t>(rend3D.orb_sp.draw_count) : 0;

        if (!rend3D.render_orb_sp)
        {
            ImGui::BeginDisabled();
            if (orb_sp_sync)
                orb_sp_sync = false;
        }

        ImGui::SliderScalar("##visible_orb_sp_frame", ImGuiDataType_U64, &visible_orb_sp_frame, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb_sp_sync = onoff_button("Sync##orb_sp_sync", ImVec2(50.0f, 18.0f), orb_sp_sync);
        rend3D.orb_sp.draw_count = static_cast<size_t>(visible_orb_sp_frame);
        if (orb_sp_sync)
            rend3D.orb_sp.draw_count = static_cast<size_t>(iframe + 1);

        if (!rend3D.render_orb_sp)
            ImGui::EndDisabled();

        if (!sol || sol->t.empty() || !sol->integr.properties.spacecraft_checkbox)
            ImGui::EndDisabled();

        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f,4.0f));

        ImGui::Text("Infinite grid");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::Text("Grid");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f);
        ImGui::Checkbox("##rend3D.render_grid", &rend3D.render_grid);

        ImGui::Dummy(ImVec2(0.0f,700.0f)); //Some extra y-space in order to be able to scroll down along scene panel.        

        if (!render_scene)
            ImGui::EndDisabled();

        //Hardware logic :

        ImGuiIO &io = ImGui::GetIO();
        if (render_scene && !io.WantCaptureMouse)
        {
            if (io.MouseWheel != 0.0f)
            {
                if (io.KeyCtrl)
                    rend3D.cam.scroll_fov(io.MouseWheel);
                else if (rend3D.cam.frame_of_ref == camera::WORLD || rend3D.cam.frame_of_ref == camera::COM)
                    rend3D.cam.scroll_dist_inertial(io.MouseWheel);
                else if (rend3D.cam.frame_of_ref == camera::BODY1 || rend3D.cam.frame_of_ref == camera::BODY2)
                    rend3D.cam.scroll_dist_body(io.MouseWheel);
            }

            if (io.MouseDown[ImGuiMouseButton_Middle])
            {
                const ImVec2 d = io.MouseDelta;
                if (d.x != 0.0f || d.y != 0.0f)
                {
                    if (io.KeyCtrl)
                        rend3D.sunlight.rotate_lon_lat(d.x, d.y);
                    else if (rend3D.cam.frame_of_ref == camera::WORLD || rend3D.cam.frame_of_ref == camera::COM)
                        rend3D.cam.rotate_lon_lat_inertial(d.x, d.y);
                    else if (rend3D.cam.frame_of_ref == camera::BODY1 || rend3D.cam.frame_of_ref == camera::BODY2)
                        rend3D.cam.move_upon_vplane(d.x, d.y, rend3D.win_width, rend3D.win_height);
                }
            }

            //Toggle play/pause state via spacebar key, but only when the cursor is in the 3D viewport region.
            if (ImGui::IsKeyReleased(ImGuiKey_Space))
                play_video = !play_video;
        }

        //Frame increment logic :

        if (play_video && render_scene && iframe < frames - 1)
        {
            if (framerate == 0) //The slider is set to 0 => paused. Do not increment iframe.
            {
                //Pass.
            }
            else if (framerate < 60) //We do a time-based step to achieve the chosen framerate.
            {
                frame_accumulator += ImGui::GetIO().DeltaTime;
                float step = 1.0f/static_cast<float>(framerate);
                //In case DeltaTime is large (e.g. if the user drags the window), use a while() so we don't 'miss' increments.
                while (frame_accumulator >= step && iframe < frames - 1)
                {
                    iframe++;
                    frame_accumulator -= step;
                }
            }
            else //framerate == 60 => let it play as fast as the machine can handle, i.e. increment every time we render.
                iframe++;
        }
        
        
        //Finally, render the 3D content.
        if (render_scene && sol && !sol->t.empty())
            rend3D.render_3D_content(*sol, iframe, reset_gpu_essential);
    }

public:
    //In this function, all the above *private* functions are called and render the scene panel gui. The *public* function reset() is only one called from gui.h and does not render anything.
    void render(const int win_width, const int win_height)
    {
        //Copy the window's dimensions to the renderer3D's members. We need them at each frame to compute the camera's projection matrix (see renderer3D.h).
        //NOTE : Do not forget to check if this is the right numbers vs the ImGui::GetIO().DisplaySize.
        rend3D.win_width  = win_width;
        rend3D.win_height = win_height;

        ImGui::SetNextWindowPos( ImVec2(0.85f*ImGui::GetIO().DisplaySize.x, 21.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 21.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene", nullptr);

        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Plots 2D"))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.2f,1.0f)); //Make all the plot buttons' off state gray.
            if (!sol || sol->t.empty())
            {
                ImGui::BeginDisabled();
                render_plot_buttons();
                ImGui::EndDisabled();   
            }
            else
            {
                render_plot_buttons();

                if (plot_cart[0]) plot_cart[0] = plot("##plot_cart[0]", "Mutual x",        "x [km]",        plot_cart[0], sol2D.x);
                if (plot_cart[1]) plot_cart[1] = plot("##plot_cart[1]", "Mutual y",        "y [km]",        plot_cart[1], sol2D.y);
                if (plot_cart[2]) plot_cart[2] = plot("##plot_cart[2]", "Mutual z",        "z [km]",        plot_cart[2], sol2D.z);
                if (plot_cart[3]) plot_cart[3] = plot("##plot_cart[3]", "Mutual distance", "distance [km]", plot_cart[3], sol2D.dist);

                if (plot_cart[4]) plot_cart[4] = plot("##plot_cart[4]", "Mutual υx",            "υx [km/sec]",  plot_cart[4], sol2D.vx);
                if (plot_cart[5]) plot_cart[5] = plot("##plot_cart[5]", "Mutual υy",            "υy [km/sec]",  plot_cart[5], sol2D.vy);
                if (plot_cart[6]) plot_cart[6] = plot("##plot_cart[6]", "Mutual υz",            "υz [km/sec]",  plot_cart[6], sol2D.vz);
                if (plot_cart[7]) plot_cart[7] = plot("##plot_cart[7]", "Mutual υ (magnitude)", "υ [km/sec]",   plot_cart[7], sol2D.vel);

                if (plot_kep[0]) plot_kep[0] = plot("##plot_kep[0]", "Semi - major axis",           "a [km]",  plot_kep[0], sol2D.sma);
                if (plot_kep[1]) plot_kep[1] = plot("##plot_kep[1]", "Eccentricity",                "e [  ]",  plot_kep[1], sol2D.ecc);
                if (plot_kep[2]) plot_kep[2] = plot("##plot_kep[2]", "Inclination",                 "i [deg]", plot_kep[2], sol2D.inc);
                if (plot_kep[3]) plot_kep[3] = plot("##plot_kep[3]", "Longitude of ascending node", "Ω [deg]", plot_kep[3], sol2D.raan);
                if (plot_kep[4]) plot_kep[4] = plot("##plot_kep[4]", "Argument of periapsis",       "ω [deg]", plot_kep[4], sol2D.argper);
                if (plot_kep[5]) plot_kep[5] = plot("##plot_kep[5]", "Mean anomaly",                "M [deg]", plot_kep[5], sol2D.manom);

                if (plot_rpy1[0]) plot_rpy1[0] = plot("##plot_rpy1[0]", "Body 1 roll",                 "roll 1 [deg]",      plot_rpy1[0], sol2D.roll1);
                if (plot_rpy1[1]) plot_rpy1[1] = plot("##plot_rpy1[1]", "Body 1 pitch",                "pitch 1 [deg]",     plot_rpy1[1], sol2D.pitch1);
                if (plot_rpy1[2]) plot_rpy1[2] = plot("##plot_rpy1[2]", "Body 1 yaw",                  "yaw 1 [deg]",       plot_rpy1[2], sol2D.yaw1);
                if (plot_rpy1[3]) plot_rpy1[3] = plot("##plot_rpy1[3]", "Body 1 rel. yaw (libration)", "rel.  yaw 1 [deg]", plot_rpy1[3], sol2D.relyaw1);

                if (plot_rpy2[0]) plot_rpy2[0] = plot("##plot_rpy2[0]", "Body 2 roll",                 "roll 2  [deg]",     plot_rpy2[0], sol2D.roll2);
                if (plot_rpy2[1]) plot_rpy2[1] = plot("##plot_rpy2[1]", "Body 2 pitch",                "pitch 2 [deg]",     plot_rpy2[1], sol2D.pitch2);
                if (plot_rpy2[2]) plot_rpy2[2] = plot("##plot_rpy2[2]", "Body 2 yaw",                  "yaw 2 [deg]",       plot_rpy2[2], sol2D.yaw2);
                if (plot_rpy2[3]) plot_rpy2[3] = plot("##plot_rpy2[3]", "Body 2 rel. yaw (libration)", "rel.  yaw 2 [deg]", plot_rpy2[3], sol2D.relyaw2);

                if (plot_w1i[0]) plot_w1i[0] = plot("##plot_w1i[0]", "Body 1 ωx (inertial frame)", "ω1x [rad/sec]", plot_w1i[0], sol2D.w1ix);
                if (plot_w1i[1]) plot_w1i[1] = plot("##plot_w1i[1]", "Body 1 ωy (inertial frame)", "ω1y [rad/sec]", plot_w1i[1], sol2D.w1iy);
                if (plot_w1i[2]) plot_w1i[2] = plot("##plot_w1i[2]", "Body 1 ωz (inertial frame)", "ω1z [rad/sec]", plot_w1i[2], sol2D.w1iz);

                if (plot_w1b[0]) plot_w1b[0] = plot("##plot_w1b[0]", "Body 1 ωx (body frame)", "ω11 [rad/sec]", plot_w1b[0], sol2D.w1bx);
                if (plot_w1b[1]) plot_w1b[1] = plot("##plot_w1b[1]", "Body 1 ωy (body frame)", "ω12 [rad/sec]", plot_w1b[1], sol2D.w1by);
                if (plot_w1b[2]) plot_w1b[2] = plot("##plot_w1b[2]", "Body 1 ωz (body frame)", "ω13 [rad/sec]", plot_w1b[2], sol2D.w1bz);

                if (plot_w2i[0]) plot_w2i[0] = plot("##plot_w2i[0]", "Body 2 ωx (inertial frame)", "ω2x [rad/sec]", plot_w2i[0], sol2D.w2ix);
                if (plot_w2i[1]) plot_w2i[1] = plot("##plot_w2i[1]", "Body 2 ωy (inertial frame)", "ω2y [rad/sec]", plot_w2i[1], sol2D.w2iy);
                if (plot_w2i[2]) plot_w2i[2] = plot("##plot_w2i[2]", "Body 2 ωz (inertial frame)", "ω2z [rad/sec]", plot_w2i[2], sol2D.w2iz);

                if (plot_w2b[0]) plot_w2b[0] = plot("##plot_w2b[0]", "Body 2 ωx (body frame)", "ω21 [rad/sec]", plot_w2b[0], sol2D.w2bx);
                if (plot_w2b[1]) plot_w2b[1] = plot("##plot_w2b[1]", "Body 2 ωy (body frame)", "ω22 [rad/sec]", plot_w2b[1], sol2D.w2by);
                if (plot_w2b[2]) plot_w2b[2] = plot("##plot_w2b[2]", "Body 2 ωz (body frame)", "ω23 [rad/sec]", plot_w2b[2], sol2D.w2bz);

                if (plot_dener_dmom[0]) plot_dener_dmom[0] = plot("##plot_dener_dmom[0]", "Energy relative error",             "| (E[i+1] - E[0])/E[0] |", plot_dener_dmom[0], sol2D.denergy);
                if (plot_dener_dmom[1]) plot_dener_dmom[1] = plot("##plot_dener_dmom[1]", "Momentum magnitude relative error", "| (L[i+1] - L[0])/L[0] |", plot_dener_dmom[1], sol2D.dmomentum);

                if (sol->integr.properties.spacecraft_checkbox)
                {
                    if (plot_rsp[0]) plot_rsp[0] = plot("##plot_rsp[0]", "Spacecraft x (binary's C.O.M. frame)", "sp.  x [km]", plot_rsp[0], sol2D.xsp);
                    if (plot_rsp[1]) plot_rsp[1] = plot("##plot_rsp[1]", "Spacecraft y (binary's C.O.M. frame)", "sp.  y [km]", plot_rsp[1], sol2D.ysp);
                    if (plot_rsp[2]) plot_rsp[2] = plot("##plot_rsp[2]", "Spacecraft z (binary's C.O.M. frame)", "sp.  z [km]", plot_rsp[2], sol2D.zsp);
                }

            }
            ImGui::PopStyleColor();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Video 3D"))
        {
            if (!sol || sol->t.empty())
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
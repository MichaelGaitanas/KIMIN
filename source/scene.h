/* This class handles the rendering logic of the right panel (scene) in the gui. */

#ifndef SCENE_H
#define SCENE_H

#include<algorithm>

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include"constants.h"
#include"typedef.h"
#include"solution.h"
#include"solution2D.h"
#include"renderer3D.h"
#include"icons.h"

class scene
{
private:
    bvec plot_cart_mut,       plot_kep_mut,   plot_dener_dmom;
    bvec plot_cart_com_helio, plot_kep_com_helio;
    bvec plot_rpy1, plot_rpy2;
    bvec plot_w1i, plot_w1b;
    bvec plot_w2i, plot_w2b;

    bvec plot_cart_sp_helio, plot_kep_sp_helio;
    bvec plot_cart_sp_com,   plot_kep_sp_com;
    bvec plot_cart_sp_com1,  plot_kep_sp_com1;
    bvec plot_cart_sp_com2,  plot_kep_sp_com2;
    
    bool render_scene, play_video, reset_gpu_essential, auto_replay, orb1_sync, orb2_sync, orb_sp_sync;

    uint64_t iframe, frames;
    int framerate; //Frame updates per second.
    float frame_accumulator; //Accumulates fractional frames between updates.

    solution *sol; //This contains all the orbital data and is used to render the 3D scene (pointer to avoid huge copy).
    solution2D sol2D; //This is the downsampled version of the sol, used only for the 2D plots.

    renderer3D rend3D;

public:
    scene() : plot_cart_mut({false,false,false,false, false,false,false,false}),
              plot_kep_mut({false,false,false,false,false,false}),
              plot_dener_dmom({false,false}),
              plot_cart_com_helio({false,false,false,false, false,false,false,false}),
              plot_kep_com_helio({false,false,false,false,false,false}),
              plot_rpy1({false,false,false,false}),
              plot_rpy2({false,false,false,false}),
              plot_w1i({false,false,false}),
              plot_w1b({false,false,false}),
              plot_w2i({false,false,false}),
              plot_w2b({false,false,false}),
              plot_cart_sp_helio({false,false,false,false, false,false,false,false}),
              plot_kep_sp_helio({false,false,false,false,false,false}),
              plot_cart_sp_com({false,false,false,false, false,false,false,false}),
              plot_kep_sp_com({false,false,false,false,false,false}),
              plot_cart_sp_com1({false,false,false,false, false,false,false,false}),
              plot_kep_sp_com1({false,false,false,false,false,false}),
              plot_cart_sp_com2({false,false,false,false, false,false,false,false}),
              plot_kep_sp_com2({false,false,false,false,false,false}),
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
        sol2D.construct(*sol); //Then create a downsampled solution for the 2D plots.
        sol->integr.orbit.clear();
        sol->integr.orbit.shrink_to_fit();

        rend3D.cam.reset(sol->integr.brillouin1 + sol->integr.brillouin2, *std::max_element(sol->dist_mut.begin(), sol->dist_mut.end()));

        iframe = 0;
        frames = static_cast<uint64_t>(sol->t.size());
        play_video = false; //Set the video at paused state ('true' means play, 'false' means pause).
        reset_gpu_essential = true; //This will inform the renderer3D::reset_gpu_resources() to run, but only once.

        uint64_t init_orb_count = (frames > 0 ? 1 : 0);
        rend3D.orb1.draw_count = rend3D.orb2.draw_count = init_orb_count;
        
        //At every new simulation, if the user hasn't assumed a spacecraft, then any previous plots regarding the spacecraft shall disappear.
        if (!sol->integr.props.spacecraft_checkbox)
        {
            for (size_t i = 0; i < plot_cart_sp_helio.size(); ++i)
                plot_cart_sp_helio[i] = plot_cart_sp_com[i] = plot_cart_sp_com1[i] = plot_cart_sp_com2[i] = false;
            for (size_t i = 0; i < plot_kep_sp_helio.size(); ++i)
                plot_kep_sp_helio[i] = plot_kep_sp_com[i] = plot_kep_sp_com1[i] = plot_kep_sp_com2[i] = false;

            rend3D.orb_sp.draw_count = 0;
            orb_sp_sync = false;
            rend3D.render_orb_sp = false;
        }
        else
            rend3D.orb_sp.draw_count = init_orb_count;
    }

private:
    //This function controls the on/off logic of a clickable labeled button in the gui.
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

    //Because the 'sol2D' (the one used for 2D plotting) is reduced in size compared to the 'sol' (the one used for exporting and 3D rendering), we have to map
    //the 'iframe' index to another index 'jframe', so that the scatter point of the current frame corresponds to the correct time.
    //This function serves the aforementioned purpose. 
    inline size_t map_frame_to_reduced_sol(const size_t iframe, const size_t original_size, const size_t reduced_size)
    {
        //Edge cases :
        if (original_size <= 1 || reduced_size <= 1)
            return 0;
        
        double step = (original_size - 1.0)/(reduced_size - 1.0);
        size_t jframe = (size_t)std::floor(iframe/step + 0.5); //Round to nearest integer.
        if (jframe >= reduced_size)
            jframe = reduced_size - 1; //Clamp to avoid wrong access.
    
        return jframe;
    }

    //This function plots the data [t, f(t)].
    bool plot(const char *imgui_id, const char *implot_id, const char *yaxis_str, bool plot_status, dvec &data)
    {
        float sx = ImGui::GetIO().DisplaySize.x;
        float sy = ImGui::GetIO().DisplaySize.y;

        ImGui::SetNextWindowPos(ImVec2(0.6f*sx, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.25f*sx, 0.4f*sy), ImGuiCond_FirstUseEver);
        ImGui::Begin(imgui_id, &plot_status);
        if (ImPlot::BeginPlot(implot_id, ImVec2(ImGui::GetWindowSize().x - 20.0f*SCX, ImGui::GetWindowSize().y - 40.0f*SCY)))
        {
            //Line logic :
            ImPlot::SetupAxes("time [days]", yaxis_str);
            ImPlot::PlotLine("", &sol2D.t[0], &data[0], sol2D.t.size());
            
            //Current frame marker logic :
            size_t jframe = map_frame_to_reduced_sol(iframe, sol->t.size(), sol2D.t.size());
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6.0f*SCX, ImColor(0,255,0,255), 1.0f, ImColor(0,255,0,255));
            ImPlot::PlotScatter("Current frame", &sol2D.t[jframe], &data[jframe], 1);

            //Collision frame marker logic :
            if (sol->integr.collision_mut) //Asteroid-asteroid collision.
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f*SCX, ImColor(255,0,0,255), 1.0f, ImColor(255,0,0,255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &data.back(), 1);
            }
            else if (sol->integr.collision_sp) //Asteroid-spacecraft collision.
            {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Down, 6.0f*SCX, ImColor(255,100,0,255), 1.0f, ImColor(255,100,0,255));
                ImPlot::PlotScatter("Collision frame", &sol2D.t.back(), &data.back(), 1);
            }

            ImPlot::EndPlot();
        }
        ImGui::End();
        return plot_status;
    }
    
    //Render on the gui the 2D plot buttons.
    void render_plot_buttons()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f); //Disable the indentation for what comes next.

        //Binary's mutual state plots.
        if (ImGui::TreeNodeEx("Mutual"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            ImGui::Text("Position and velocity");
            plot_cart_mut[0] = onoff_button("x##plot_cart_mut[0]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[0]); ImGui::SameLine();
            plot_cart_mut[1] = onoff_button("y##plot_cart_mut[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[1]); ImGui::SameLine();
            plot_cart_mut[2] = onoff_button("z##plot_cart_mut[2]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[2]); ImGui::SameLine();
            plot_cart_mut[3] = onoff_button("dist##plot_cart_mut[3]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[3]);
            plot_cart_mut[4] = onoff_button("υx##plot_cart_mut[4]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[4]); ImGui::SameLine();
            plot_cart_mut[5] = onoff_button("υy##plot_cart_mut[5]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[5]); ImGui::SameLine();
            plot_cart_mut[6] = onoff_button("υz##plot_cart_mut[6]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[6]); ImGui::SameLine();
            plot_cart_mut[7] = onoff_button("υel##plot_cart_mut[7]" , ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_mut[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Keplerian elements");
            plot_kep_mut[0] = onoff_button("a##plot_kep_mut[0]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_mut[0]); ImGui::SameLine();
            plot_kep_mut[1] = onoff_button("e##plot_kep_mut[1]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_mut[1]); ImGui::SameLine();
            plot_kep_mut[2] = onoff_button("i##plot_kep_mut[2]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_mut[2]); ImGui::SameLine();
            plot_kep_mut[3] = onoff_button("Ω##plot_kep_mut[3]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_mut[3]); ImGui::SameLine();
            plot_kep_mut[4] = onoff_button("ω##plot_kep_mut[4]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_mut[4]); ImGui::SameLine();
            plot_kep_mut[5] = onoff_button("M##plot_kep_mut[5]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_mut[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Energy and momentum errors");
            plot_dener_dmom[0] = onoff_button("energy##plot_dener_dmom[0]",   ImVec2(80.0f*SCX, 25.0f*SCY), plot_dener_dmom[0]); ImGui::SameLine();
            plot_dener_dmom[1] = onoff_button("momentum##plot_dener_dmom[1]", ImVec2(80.0f*SCX, 25.0f*SCY), plot_dener_dmom[1]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::TreePop();
        }

        //Binary's COM plots.
        if (ImGui::TreeNodeEx("COM (Heliocentric)"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            ImGui::Text("Position and velocity");
            plot_cart_com_helio[0] = onoff_button("x##plot_cart_com_helio[0]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[0]); ImGui::SameLine();
            plot_cart_com_helio[1] = onoff_button("y##plot_cart_com_helio[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[1]); ImGui::SameLine();
            plot_cart_com_helio[2] = onoff_button("z##plot_cart_com_helio[2]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[2]); ImGui::SameLine();
            plot_cart_com_helio[3] = onoff_button("dist##plot_cart_com_helio[3]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[3]);
            plot_cart_com_helio[4] = onoff_button("υx##plot_cart_com_helio[4]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[4]); ImGui::SameLine();
            plot_cart_com_helio[5] = onoff_button("υy##plot_cart_com_helio[5]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[5]); ImGui::SameLine();
            plot_cart_com_helio[6] = onoff_button("υz##plot_cart_com_helio[6]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[6]); ImGui::SameLine();
            plot_cart_com_helio[7] = onoff_button("υel##plot_cart_com_helio[7]",  ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_com_helio[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Keplerian elements");
            plot_kep_com_helio[0] = onoff_button("a##plot_kep_com_helio[0]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_com_helio[0]); ImGui::SameLine();
            plot_kep_com_helio[1] = onoff_button("e##plot_kep_com_helio[1]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_com_helio[1]); ImGui::SameLine();
            plot_kep_com_helio[2] = onoff_button("i##plot_kep_com_helio[2]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_com_helio[2]); ImGui::SameLine();
            plot_kep_com_helio[3] = onoff_button("Ω##plot_kep_com_helio[3]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_com_helio[3]); ImGui::SameLine();
            plot_kep_com_helio[4] = onoff_button("ω##plot_kep_com_helio[4]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_com_helio[4]); ImGui::SameLine();
            plot_kep_com_helio[5] = onoff_button("M##plot_kep_com_helio[5]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_com_helio[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::TreePop();
        }

        //Body 1 plots.
        if (ImGui::TreeNodeEx("Body 1"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            ImGui::Text("Euler angles (XYZ)");
            plot_rpy1[0] = onoff_button("roll##plot_rpy1[0]",     ImVec2(50.0f*SCX, 20.0f*SCY), plot_rpy1[0]); ImGui::SameLine();
            plot_rpy1[1] = onoff_button("pitch##plot_rpy1[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_rpy1[1]); ImGui::SameLine();
            plot_rpy1[2] = onoff_button("yaw##plot_rpy1[2]",      ImVec2(50.0f*SCX, 20.0f*SCY), plot_rpy1[2]); ImGui::SameLine();
            plot_rpy1[3] = onoff_button("rel. yaw##plot_rpy1[3]", ImVec2(60.0f*SCX, 20.0f*SCY), plot_rpy1[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Angular velocity (inertial frame)");
            plot_w1i[0] = onoff_button("ωx##plot_w1i[0]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w1i[0]); ImGui::SameLine();
            plot_w1i[1] = onoff_button("ωy##plot_w1i[1]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w1i[1]); ImGui::SameLine();
            plot_w1i[2] = onoff_button("ωz##plot_w1i[2]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w1i[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            
            ImGui::Text("Angular velocity (body frame)");
            plot_w1b[0] = onoff_button("ω1##plot_w1b[0]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w1b[0]); ImGui::SameLine();
            plot_w1b[1] = onoff_button("ω2##plot_w1b[1]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w1b[1]); ImGui::SameLine();
            plot_w1b[2] = onoff_button("ω3##plot_w1b[2]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w1b[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::TreePop();
        }

        //Body 2 plots.
        if (ImGui::TreeNodeEx("Body 2"))
        {
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            ImGui::Text("Euler angles (XYZ)");
            plot_rpy2[0] = onoff_button("roll##plot_rpy2[0]",      ImVec2(50.0f*SCX, 20.0f*SCY), plot_rpy2[0]); ImGui::SameLine();
            plot_rpy2[1] = onoff_button("pitch##plot_rpy2[1]",     ImVec2(50.0f*SCX, 20.0f*SCY), plot_rpy2[1]); ImGui::SameLine();
            plot_rpy2[2] = onoff_button("yaw##plot_rpy2[2]",       ImVec2(50.0f*SCX, 20.0f*SCY), plot_rpy2[2]); ImGui::SameLine();
            plot_rpy2[3] = onoff_button("rel. yaw##plot_rpy2[3]",  ImVec2(60.0f*SCX, 20.0f*SCY), plot_rpy2[3]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Angular velocity (inertial frame)");
            plot_w2i[0] = onoff_button("ωx##plot_w2i[0]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w2i[0]); ImGui::SameLine();
            plot_w2i[1] = onoff_button("ωy##plot_w2i[1]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w2i[1]); ImGui::SameLine();
            plot_w2i[2] = onoff_button("ωz##plot_w2i[2]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w2i[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            
            ImGui::Text("Angular velocity (body frame)");
            plot_w2b[0] = onoff_button("ω1##plot_w2b[0]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w2b[0]); ImGui::SameLine();
            plot_w2b[1] = onoff_button("ω2##plot_w2b[1]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w2b[1]); ImGui::SameLine();
            plot_w2b[2] = onoff_button("ω3##plot_w2b[2]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_w2b[2]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::TreePop();
        }

        //Spacecraft's plots.
        if (ImGui::TreeNodeEx("Spacecraft"))
        {
            if (!sol || sol->t.empty() || !sol->integr.props.spacecraft_checkbox)
                ImGui::BeginDisabled();
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Position and velocity (Heliocentric)");
            plot_cart_sp_helio[0] = onoff_button("x##plot_cart_sp_helio[0]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[0]); ImGui::SameLine();
            plot_cart_sp_helio[1] = onoff_button("y##plot_cart_sp_helio[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[1]); ImGui::SameLine();
            plot_cart_sp_helio[2] = onoff_button("z##plot_cart_sp_helio[2]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[2]); ImGui::SameLine();
            plot_cart_sp_helio[3] = onoff_button("dist##plot_cart_sp_helio[3]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[3]);
            plot_cart_sp_helio[4] = onoff_button("υx##plot_cart_sp_helio[4]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[4]); ImGui::SameLine();
            plot_cart_sp_helio[5] = onoff_button("υy##plot_cart_sp_helio[5]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[5]); ImGui::SameLine();
            plot_cart_sp_helio[6] = onoff_button("υz##plot_cart_sp_helio[6]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[6]); ImGui::SameLine();
            plot_cart_sp_helio[7] = onoff_button("υel##plot_cart_sp_helio[7]",  ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_helio[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Keplerian elements (Heliocentric)");
            plot_kep_sp_helio[0] = onoff_button("a##plot_kep_sp_helio[0]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_helio[0]); ImGui::SameLine();
            plot_kep_sp_helio[1] = onoff_button("e##plot_kep_sp_helio[1]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_helio[1]); ImGui::SameLine();
            plot_kep_sp_helio[2] = onoff_button("i##plot_kep_sp_helio[2]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_helio[2]); ImGui::SameLine();
            plot_kep_sp_helio[3] = onoff_button("Ω##plot_kep_sp_helio[3]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_helio[3]); ImGui::SameLine();
            plot_kep_sp_helio[4] = onoff_button("ω##plot_kep_sp_helio[4]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_helio[4]); ImGui::SameLine();
            plot_kep_sp_helio[5] = onoff_button("M##plot_kep_sp_helio[5]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_helio[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            
            ImGui::Text("Position and velocity (binary COM)");
            plot_cart_sp_com[0] = onoff_button("x##plot_cart_sp_com[0]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[0]); ImGui::SameLine();
            plot_cart_sp_com[1] = onoff_button("y##plot_cart_sp_com[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[1]); ImGui::SameLine();
            plot_cart_sp_com[2] = onoff_button("z##plot_cart_sp_com[2]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[2]); ImGui::SameLine();
            plot_cart_sp_com[3] = onoff_button("dist##plot_cart_sp_com[3]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[3]);
            plot_cart_sp_com[4] = onoff_button("υx##plot_cart_sp_com[4]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[4]); ImGui::SameLine();
            plot_cart_sp_com[5] = onoff_button("υy##plot_cart_sp_com[5]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[5]); ImGui::SameLine();
            plot_cart_sp_com[6] = onoff_button("υz##plot_cart_sp_com[6]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[6]); ImGui::SameLine();
            plot_cart_sp_com[7] = onoff_button("υel##plot_cart_sp_com[7]",  ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Keplerian elements (binary COM)");
            plot_kep_sp_com[0] = onoff_button("a##plot_kep_sp_com[0]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com[0]); ImGui::SameLine();
            plot_kep_sp_com[1] = onoff_button("e##plot_kep_sp_com[1]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com[1]); ImGui::SameLine();
            plot_kep_sp_com[2] = onoff_button("i##plot_kep_sp_com[2]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com[2]); ImGui::SameLine();
            plot_kep_sp_com[3] = onoff_button("Ω##plot_kep_sp_com[3]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com[3]); ImGui::SameLine();
            plot_kep_sp_com[4] = onoff_button("ω##plot_kep_sp_com[4]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com[4]); ImGui::SameLine();
            plot_kep_sp_com[5] = onoff_button("M##plot_kep_sp_com[5]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Position and velocity (body 1)");
            plot_cart_sp_com1[0] = onoff_button("x##plot_cart_sp_com1[0]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[0]); ImGui::SameLine();
            plot_cart_sp_com1[1] = onoff_button("y##plot_cart_sp_com1[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[1]); ImGui::SameLine();
            plot_cart_sp_com1[2] = onoff_button("z##plot_cart_sp_com1[2]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[2]); ImGui::SameLine();
            plot_cart_sp_com1[3] = onoff_button("dist##plot_cart_sp_com1[3]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[3]);
            plot_cart_sp_com1[4] = onoff_button("υx##plot_cart_sp_com1[4]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[4]); ImGui::SameLine();
            plot_cart_sp_com1[5] = onoff_button("υy##plot_cart_sp_com1[5]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[5]); ImGui::SameLine();
            plot_cart_sp_com1[6] = onoff_button("υz##plot_cart_sp_com1[6]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[6]); ImGui::SameLine();
            plot_cart_sp_com1[7] = onoff_button("υel##plot_cart_sp_com1[7]",  ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com1[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Keplerian elements (body 1)");
            plot_kep_sp_com1[0] = onoff_button("a##plot_kep_sp_com1[0]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com1[0]); ImGui::SameLine();
            plot_kep_sp_com1[1] = onoff_button("e##plot_kep_sp_com1[1]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com1[1]); ImGui::SameLine();
            plot_kep_sp_com1[2] = onoff_button("i##plot_kep_sp_com1[2]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com1[2]); ImGui::SameLine();
            plot_kep_sp_com1[3] = onoff_button("Ω##plot_kep_sp_com1[3]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com1[3]); ImGui::SameLine();
            plot_kep_sp_com1[4] = onoff_button("ω##plot_kep_sp_com1[4]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com1[4]); ImGui::SameLine();
            plot_kep_sp_com1[5] = onoff_button("M##plot_kep_sp_com1[5]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com1[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Position and velocity (body 2)");
            plot_cart_sp_com2[0] = onoff_button("x##plot_cart_sp_com2[0]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[0]); ImGui::SameLine();
            plot_cart_sp_com2[1] = onoff_button("y##plot_cart_sp_com2[1]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[1]); ImGui::SameLine();
            plot_cart_sp_com2[2] = onoff_button("z##plot_cart_sp_com2[2]",    ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[2]); ImGui::SameLine();
            plot_cart_sp_com2[3] = onoff_button("dist##plot_cart_sp_com2[3]", ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[3]);
            plot_cart_sp_com2[4] = onoff_button("υx##plot_cart_sp_com2[4]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[4]); ImGui::SameLine();
            plot_cart_sp_com2[5] = onoff_button("υy##plot_cart_sp_com2[5]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[5]); ImGui::SameLine();
            plot_cart_sp_com2[6] = onoff_button("υz##plot_cart_sp_com2[6]",   ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[6]); ImGui::SameLine();
            plot_cart_sp_com2[7] = onoff_button("υel##plot_cart_sp_com2[7]",  ImVec2(50.0f*SCX, 20.0f*SCY), plot_cart_sp_com2[7]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            ImGui::Text("Keplerian elements (body 2)");
            plot_kep_sp_com2[0] = onoff_button("a##plot_kep_sp_com2[0]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com2[0]); ImGui::SameLine();
            plot_kep_sp_com2[1] = onoff_button("e##plot_kep_sp_com2[1]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com2[1]); ImGui::SameLine();
            plot_kep_sp_com2[2] = onoff_button("i##plot_kep_sp_com2[2]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com2[2]); ImGui::SameLine();
            plot_kep_sp_com2[3] = onoff_button("Ω##plot_kep_sp_com2[3]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com2[3]); ImGui::SameLine();
            plot_kep_sp_com2[4] = onoff_button("ω##plot_kep_sp_com2[4]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com2[4]); ImGui::SameLine();
            plot_kep_sp_com2[5] = onoff_button("M##plot_kep_sp_com2[5]", ImVec2(35.0f*SCX, 20.0f*SCY), plot_kep_sp_com2[5]);
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));

            if (!sol || sol->t.empty() || !sol->integr.props.spacecraft_checkbox)
                ImGui::EndDisabled();

            ImGui::TreePop();
        }
        ImGui::PopStyleVar();
    }

    //Render on the gui the buttons that correspond to the 3D scene.
    void render_scene_buttons()
    {
        //Content state logic :

        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));
        ImGui::Text("Content state");
        render_scene = onoff_button("Render##render_scene", ImVec2(80.0f*SCX, 25.0f*SCY), render_scene);
        ImGui::SameLine();

        if (!render_scene)
        {
            ImGui::BeginDisabled();
            play_video = onoff_button("Play/Pause##play_video", ImVec2(80.0f*SCX, 25.0f*SCY), play_video);
            ImGui::SameLine();
            auto_replay = onoff_button(ICON_FA_REDO" Auto##auto_replay", ImVec2(55.0f*SCX, 25.0f*SCY), auto_replay);
            ImGui::EndDisabled();
        }
        else
        {
            play_video = onoff_button("Play/Pause##play_video", ImVec2(80.0f*SCX, 25.0f*SCY), play_video);
            ImGui::SameLine();
            auto_replay = onoff_button(ICON_FA_REDO" Auto##auto_replay", ImVec2(55.0f*SCX, 25.0f*SCY), auto_replay);
            if (auto_replay && play_video && iframe >= frames - 1)
                iframe = 0;
        }

        if (!render_scene)
            ImGui::BeginDisabled();

        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));
        ImGui::Text("Frame");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f*SCX);
        uint64_t visible_min_frame = (frames > 0) ? 1 : 0;
        uint64_t visible_iframe = (frames > 0) ? (iframe + 1) : 0; //Display in the gui 1-based frame (instead of 0-based, which is used in the arrays as index).
        ImGui::SliderScalar("##visible_iframe", ImGuiDataType_U64, &visible_iframe, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        //Rule is : the above slider controls the frames and then the 'iframe' is updated accordingly, but into 0-based frame, because it is an index.
        iframe = (visible_iframe > 0) ? (visible_iframe - 1) : 0;
        ImGui::Text("Rate");
        ImGui::SameLine();
        ImGui::SetCursorPosX(50.0f*SCX);
        ImGui::SliderInt("[Hz]##framerate", &framerate, 0, 60, "%d");
        if (!sol || sol->t.empty())
            ImGui::Text("Time : 0.00  [days]");
        else
            ImGui::Text("Time : %.2f  [days]", static_cast<float>(sol->t[iframe]));
        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));

        //Camera setup logic :

        ImGui::Text("Camera setup");
        ImGui::Dummy(ImVec2(0.0f, 4.0f*SCY));

        ImGui::Text("Dist");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f*SCX);
        ImGui::SliderFloat("[km]##rend3D.cam.dist", &rend3D.cam.dist, rend3D.cam.min_dist, rend3D.cam.max_dist, "%.3f", ImGuiSliderFlags_Logarithmic);

        ImGui::Text("Lon");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f*SCX);
        ImGui::SliderFloat("[deg]##rend3D.cam.lon", &rend3D.cam.lon, 0.0f, 360.0f, "%.1f");

        ImGui::Text("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f*SCX);
        ImGui::SliderFloat("[deg]##rend3D.cam.lat", &rend3D.cam.lat, 0.0f, 180.0f, "%.1f");
        ImGui::Dummy(ImVec2(0.0f, 8.0f*SCY));

        ImGui::Text("FoV");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f*SCX);
        ImGui::SliderFloat("[deg]##rend3D.cam.fov", &rend3D.cam.fov, CAM_MIN_FOV, CAM_MAX_FOV, "%.0f");

        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));

        //Shadow setup logic :

        ImGui::Text("Shadow map");

        ImGui::Text("Reso");
        ImGui::SameLine();
        ImGui::SetCursorPosX(40.0f*SCX);
        if (ImGui::SliderInt("[pix]##rend3D.depth_reso", &rend3D.depth_reso, DEPTH_RESO_MIN, DEPTH_RESO_MAX))
            rend3D.setup_depth_fbo();

        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 7.5f*SCY));

        //Meshes to render logic :

        ImGui::Text("Visible meshes");
        ImGui::Dummy(ImVec2(0.0f, 4.0f*SCY));

        ImGui::Text("Bodies");
        ImGui::Dummy(ImVec2(0.0f, 4.0f*SCY));

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f*SCX);
        ImGui::Checkbox("##rend3D.render_body1", &rend3D.render_body1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f*SCX);
        ImGui::Text("Axes 1");
        ImGui::SameLine();
        ImGui::Checkbox("##rend3D.render_axes1", &rend3D.render_axes1);

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f*SCX);
        ImGui::Checkbox("##rend3D.render_body2", &rend3D.render_body2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f*SCX);
        ImGui::Text("Axes 2");
        ImGui::SameLine();
        ImGui::Checkbox("##rend3D.render_axes2", &rend3D.render_axes2);
        ImGui::Dummy(ImVec2(0.0f,4.0f*SCY));

        ImGui::Text("Orbits");
        ImGui::Dummy(ImVec2(0.0f, 4.0f*SCY));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.2f,1.0f)); //Make all the plot buttons' off state gray.

        ImGui::Text("Body 1");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f*SCX);
        ImGui::Checkbox("##rend3D.render_orb1", &rend3D.render_orb1);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f*SCX);
        ImGui::SetNextItemWidth(100.0f*SCX);
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
        orb1_sync = onoff_button("Sync##orb1_sync", ImVec2(50.0f*SCX, 18.0f*SCY), orb1_sync);
        if (orb1_sync)
            rend3D.orb1.draw_count = static_cast<size_t>(iframe + 1);

        if (!rend3D.render_orb1)
            ImGui::EndDisabled();

        ImGui::Text("Body 2");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f*SCX);
        ImGui::Checkbox("##rend3D.render_orb2", &rend3D.render_orb2);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f*SCX);
        ImGui::SetNextItemWidth(100.0f*SCX);
        uint64_t visible_orb2_frame = (frames > 0) ? static_cast<uint64_t>(rend3D.orb2.draw_count) : 0;

        if (!rend3D.render_orb2)
        {
            ImGui::BeginDisabled();
            if (orb2_sync)
                orb2_sync = false;
        }

        ImGui::SliderScalar("##visible_orb2_frame", ImGuiDataType_U64, &visible_orb2_frame, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb2_sync = onoff_button("Sync##visible_orb2_frame", ImVec2(50.0f*SCX, 18.0f*SCY), orb2_sync);
        rend3D.orb2.draw_count = static_cast<size_t>(visible_orb2_frame);
        if (orb2_sync)
            rend3D.orb2.draw_count = static_cast<size_t>(iframe + 1);

        if (!rend3D.render_orb2)
            ImGui::EndDisabled();

        if (!sol || sol->t.empty() || !sol->integr.props.spacecraft_checkbox)
            ImGui::BeginDisabled();

        ImGui::Text("Orbiter");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f*SCX);
        ImGui::Checkbox("##rend3D.render_orb_sp", &rend3D.render_orb_sp);
        ImGui::SameLine();
        ImGui::SetCursorPosX(90.0f*SCX);
        ImGui::SetNextItemWidth(100.0f*SCX);
        uint64_t visible_orb_sp_frame = (frames > 0) ? static_cast<uint64_t>(rend3D.orb_sp.draw_count) : 0;

        if (!rend3D.render_orb_sp)
        {
            ImGui::BeginDisabled();
            if (orb_sp_sync)
                orb_sp_sync = false;
        }

        ImGui::SliderScalar("##visible_orb_sp_frame", ImGuiDataType_U64, &visible_orb_sp_frame, &visible_min_frame, &frames, "%" PRIu64, ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        orb_sp_sync = onoff_button("Sync##orb_sp_sync", ImVec2(50.0f*SCX, 18.0f*SCY), orb_sp_sync);
        rend3D.orb_sp.draw_count = static_cast<size_t>(visible_orb_sp_frame);
        if (orb_sp_sync)
            rend3D.orb_sp.draw_count = static_cast<size_t>(iframe + 1);

        if (!rend3D.render_orb_sp)
            ImGui::EndDisabled();

        if (!sol || sol->t.empty() || !sol->integr.props.spacecraft_checkbox)
            ImGui::EndDisabled();

        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f,4.0f*SCY));

        ImGui::Text("Infinite grid");
        ImGui::Dummy(ImVec2(0.0f, 4.0f*SCY));
        ImGui::Text("Grid");
        ImGui::SameLine();
        ImGui::SetCursorPosX(60.0f*SCX);
        ImGui::Checkbox("##rend3D.render_grid", &rend3D.render_grid);

        ImGui::Dummy(ImVec2(0.0f,700.0f*SCY)); //Some extra y-space in order to be able to scroll down along scene panel.        

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
                else
                    rend3D.cam.scroll_dist(io.MouseWheel);
            }
            if (io.MouseDown[ImGuiMouseButton_Middle])
            {
                const ImVec2 d = io.MouseDelta;
                if (d.x != 0.0f || d.y != 0.0f)
                    rend3D.cam.rotate_lon_lat(d.x, d.y);
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
                float step = 1.0f/framerate;
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

        float sx = ImGui::GetIO().DisplaySize.x;
        float sy = ImGui::GetIO().DisplaySize.y;

        ImGui::SetNextWindowPos( ImVec2(0.85f*sx, ImGui::GetFrameHeight()), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.15f*sx, sy - ImGui::GetFrameHeight()), ImGuiCond_FirstUseEver);
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

                if (plot_cart_mut[0]) plot_cart_mut[0] = plot("##plot_cart_mut[0]", "Mutual x",             "x [km]",      plot_cart_mut[0], sol2D.xmut);
                if (plot_cart_mut[1]) plot_cart_mut[1] = plot("##plot_cart_mut[1]", "Mutual y",             "y [km]",      plot_cart_mut[1], sol2D.ymut);
                if (plot_cart_mut[2]) plot_cart_mut[2] = plot("##plot_cart_mut[2]", "Mutual z",             "z [km]",      plot_cart_mut[2], sol2D.zmut);
                if (plot_cart_mut[3]) plot_cart_mut[3] = plot("##plot_cart_mut[3]", "Mutual distance",      "d [km]",      plot_cart_mut[3], sol2D.dist_mut);
                if (plot_cart_mut[4]) plot_cart_mut[4] = plot("##plot_cart_mut[4]", "Mutual υx",            "υx [km/sec]", plot_cart_mut[4], sol2D.vxmut);
                if (plot_cart_mut[5]) plot_cart_mut[5] = plot("##plot_cart_mut[5]", "Mutual υy",            "υy [km/sec]", plot_cart_mut[5], sol2D.vymut);
                if (plot_cart_mut[6]) plot_cart_mut[6] = plot("##plot_cart_mut[6]", "Mutual υz",            "υz [km/sec]", plot_cart_mut[6], sol2D.vzmut);
                if (plot_cart_mut[7]) plot_cart_mut[7] = plot("##plot_cart_mut[7]", "Mutual υ (magnitude)", "υ [km/sec]",  plot_cart_mut[7], sol2D.vel_mut);

                if (plot_kep_mut[0]) plot_kep_mut[0] = plot("##plot_kep_mut[0]", "Mutual semi - major axis",           "a [km]",  plot_kep_mut[0], sol2D.sma_mut);
                if (plot_kep_mut[1]) plot_kep_mut[1] = plot("##plot_kep_mut[1]", "Mutual eccentricity",                "e [  ]",  plot_kep_mut[1], sol2D.ecc_mut);
                if (plot_kep_mut[2]) plot_kep_mut[2] = plot("##plot_kep_mut[2]", "Mutual inclination",                 "i [deg]", plot_kep_mut[2], sol2D.inc_mut);
                if (plot_kep_mut[3]) plot_kep_mut[3] = plot("##plot_kep_mut[3]", "Mutual longitude of ascending node", "Ω [deg]", plot_kep_mut[3], sol2D.raan_mut);
                if (plot_kep_mut[4]) plot_kep_mut[4] = plot("##plot_kep_mut[4]", "Mutual argument of periapsis",       "ω [deg]", plot_kep_mut[4], sol2D.argper_mut);
                if (plot_kep_mut[5]) plot_kep_mut[5] = plot("##plot_kep_mut[5]", "Mutual mean anomaly",                "M [deg]", plot_kep_mut[5], sol2D.manom_mut);

                if (plot_dener_dmom[0]) plot_dener_dmom[0] = plot("##plot_dener_dmom[0]", "Energy relative error",             "| (E[i+1] - E[0])/E[0] |", plot_dener_dmom[0], sol2D.denergy);
                if (plot_dener_dmom[1]) plot_dener_dmom[1] = plot("##plot_dener_dmom[1]", "Momentum magnitude relative error", "| (L[i+1] - L[0])/L[0] |", plot_dener_dmom[1], sol2D.dmomentum);

                if (plot_cart_com_helio[0]) plot_cart_com_helio[0] = plot("##plot_cart_com_helio[0]", "Heliocentric COM x",             "x [AU]",      plot_cart_com_helio[0], sol2D.xcom_helio);
                if (plot_cart_com_helio[1]) plot_cart_com_helio[1] = plot("##plot_cart_com_helio[1]", "Heliocentric COM y",             "y [AU]",      plot_cart_com_helio[1], sol2D.ycom_helio);
                if (plot_cart_com_helio[2]) plot_cart_com_helio[2] = plot("##plot_cart_com_helio[2]", "Heliocentric COM z",             "z [AU]",      plot_cart_com_helio[2], sol2D.zcom_helio);
                if (plot_cart_com_helio[3]) plot_cart_com_helio[3] = plot("##plot_cart_com_helio[3]", "Heliocentric COM distance",      "d [AU]",      plot_cart_com_helio[3], sol2D.dist_com_helio);
                if (plot_cart_com_helio[4]) plot_cart_com_helio[4] = plot("##plot_cart_com_helio[4]", "Heliocentric COM υx",            "υx [km/sec]", plot_cart_com_helio[4], sol2D.vxcom_helio);
                if (plot_cart_com_helio[5]) plot_cart_com_helio[5] = plot("##plot_cart_com_helio[5]", "Heliocentric COM υy",            "υy [km/sec]", plot_cart_com_helio[5], sol2D.vycom_helio);
                if (plot_cart_com_helio[6]) plot_cart_com_helio[6] = plot("##plot_cart_com_helio[6]", "Heliocentric COM υz",            "υz [km/sec]", plot_cart_com_helio[6], sol2D.vzcom_helio);
                if (plot_cart_com_helio[7]) plot_cart_com_helio[7] = plot("##plot_cart_com_helio[7]", "Heliocentric COM υ (magnitude)", "υ [km/sec]",  plot_cart_com_helio[7], sol2D.vel_com_helio);

                if (plot_kep_com_helio[0]) plot_kep_com_helio[0] = plot("##plot_kep_com_helio[0]", "Heliocentric COM semi - major axis",           "a [AU]",  plot_kep_com_helio[0], sol2D.sma_com_helio);
                if (plot_kep_com_helio[1]) plot_kep_com_helio[1] = plot("##plot_kep_com_helio[1]", "Heliocentric COM eccentricity",                "e [  ]",  plot_kep_com_helio[1], sol2D.ecc_com_helio);
                if (plot_kep_com_helio[2]) plot_kep_com_helio[2] = plot("##plot_kep_com_helio[2]", "Heliocentric COM inclination",                 "i [deg]", plot_kep_com_helio[2], sol2D.inc_com_helio);
                if (plot_kep_com_helio[3]) plot_kep_com_helio[3] = plot("##plot_kep_com_helio[3]", "Heliocentric COM Longitude of ascending node", "Ω [deg]", plot_kep_com_helio[3], sol2D.raan_com_helio);
                if (plot_kep_com_helio[4]) plot_kep_com_helio[4] = plot("##plot_kep_com_helio[4]", "Heliocentric COM argument of periapsis",       "ω [deg]", plot_kep_com_helio[4], sol2D.argper_com_helio);
                if (plot_kep_com_helio[5]) plot_kep_com_helio[5] = plot("##plot_kep_com_helio[5]", "Heliocentric COM mean anomaly",                "M [deg]", plot_kep_com_helio[5], sol2D.manom_com_helio);

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

                if (sol->integr.props.spacecraft_checkbox)
                {
                    if (plot_cart_sp_helio[0]) plot_cart_sp_helio[0] = plot("##plot_cart_sp_helio[0]", "Heliocentric spacecraft x",             "x [AU]",       plot_cart_sp_helio[0], sol2D.xsp_helio);
                    if (plot_cart_sp_helio[1]) plot_cart_sp_helio[1] = plot("##plot_cart_sp_helio[1]", "Heliocentric spacecraft y",             "y [AU]",       plot_cart_sp_helio[1], sol2D.ysp_helio);
                    if (plot_cart_sp_helio[2]) plot_cart_sp_helio[2] = plot("##plot_cart_sp_helio[2]", "Heliocentric spacecraft z",             "z [AU]",       plot_cart_sp_helio[2], sol2D.zsp_helio);
                    if (plot_cart_sp_helio[3]) plot_cart_sp_helio[3] = plot("##plot_cart_sp_helio[3]", "Heliocentric spacecraft distance",      "d [AU]",       plot_cart_sp_helio[3], sol2D.dist_sp_helio);
                    if (plot_cart_sp_helio[4]) plot_cart_sp_helio[4] = plot("##plot_cart_sp_helio[4]", "Heliocentric spacecraft υx",            "υx [km/sec]",  plot_cart_sp_helio[4], sol2D.vxsp_helio);
                    if (plot_cart_sp_helio[5]) plot_cart_sp_helio[5] = plot("##plot_cart_sp_helio[5]", "Heliocentric spacecraft υy",            "υy [km/sec]",  plot_cart_sp_helio[5], sol2D.vysp_helio);
                    if (plot_cart_sp_helio[6]) plot_cart_sp_helio[6] = plot("##plot_cart_sp_helio[6]", "Heliocentric spacecraft υz",            "υz [km/sec]",  plot_cart_sp_helio[6], sol2D.vzsp_helio);
                    if (plot_cart_sp_helio[7]) plot_cart_sp_helio[7] = plot("##plot_cart_sp_helio[7]", "Heliocentric spacecraft υ (magnitude)", "υ [km/sec]",   plot_cart_sp_helio[7], sol2D.vel_sp_helio);

                    if (plot_kep_sp_helio[0]) plot_kep_sp_helio[0] = plot("##plot_kep_sp_helio[0]", "Heliocentric spacecraft semi - major axis",           "a [AU]",  plot_kep_sp_helio[0], sol2D.sma_sp_helio);
                    if (plot_kep_sp_helio[1]) plot_kep_sp_helio[1] = plot("##plot_kep_sp_helio[1]", "Heliocentric spacecraft eccentricity",                "e [  ]",  plot_kep_sp_helio[1], sol2D.ecc_sp_helio);
                    if (plot_kep_sp_helio[2]) plot_kep_sp_helio[2] = plot("##plot_kep_sp_helio[2]", "Heliocentric spacecraft inclination",                 "i [deg]", plot_kep_sp_helio[2], sol2D.inc_sp_helio);
                    if (plot_kep_sp_helio[3]) plot_kep_sp_helio[3] = plot("##plot_kep_sp_helio[3]", "Heliocentric spacecraft longitude of ascending node", "Ω [deg]", plot_kep_sp_helio[3], sol2D.raan_sp_helio);
                    if (plot_kep_sp_helio[4]) plot_kep_sp_helio[4] = plot("##plot_kep_sp_helio[4]", "Heliocentric spacecraft argument of periapsis",       "ω [deg]", plot_kep_sp_helio[4], sol2D.argper_sp_helio);
                    if (plot_kep_sp_helio[5]) plot_kep_sp_helio[5] = plot("##plot_kep_sp_helio[5]", "Heliocentric spacecraft mean anomaly",                "M [deg]", plot_kep_sp_helio[5], sol2D.manom_sp_helio);

                    if (plot_cart_sp_com[0]) plot_cart_sp_com[0] = plot("##plot_cart_sp_com[0]", "Spacecraft x (binary COM)",             "x [km]",       plot_cart_sp_com[0], sol2D.xsp_com);
                    if (plot_cart_sp_com[1]) plot_cart_sp_com[1] = plot("##plot_cart_sp_com[1]", "Spacecraft y (binary COM)",             "y [km]",       plot_cart_sp_com[1], sol2D.ysp_com);
                    if (plot_cart_sp_com[2]) plot_cart_sp_com[2] = plot("##plot_cart_sp_com[2]", "Spacecraft z (binary COM)",             "z [km]",       plot_cart_sp_com[2], sol2D.zsp_com);
                    if (plot_cart_sp_com[3]) plot_cart_sp_com[3] = plot("##plot_cart_sp_com[3]", "Spacecraft distance (binary COM)",      "d [km]",       plot_cart_sp_com[3], sol2D.dist_sp_com);
                    if (plot_cart_sp_com[4]) plot_cart_sp_com[4] = plot("##plot_cart_sp_com[4]", "Spacecraft υx (binary COM)",            "υx [km/sec]",  plot_cart_sp_com[4], sol2D.vxsp_com);
                    if (plot_cart_sp_com[5]) plot_cart_sp_com[5] = plot("##plot_cart_sp_com[5]", "Spacecraft υy (binary COM)",            "υy [km/sec]",  plot_cart_sp_com[5], sol2D.vysp_com);
                    if (plot_cart_sp_com[6]) plot_cart_sp_com[6] = plot("##plot_cart_sp_com[6]", "Spacecraft υz (binary COM)",            "υz [km/sec]",  plot_cart_sp_com[6], sol2D.vzsp_com);
                    if (plot_cart_sp_com[7]) plot_cart_sp_com[7] = plot("##plot_cart_sp_com[7]", "Spacecraft υ (magnitude) (binary COM)", "υ [km/sec]",   plot_cart_sp_com[7], sol2D.vel_sp_com);

                    if (plot_kep_sp_com[0]) plot_kep_sp_com[0] = plot("##plot_kep_sp_com[0]", "Spacecraft semi - major axis (binary COM)",           "a [km]",  plot_kep_sp_com[0], sol2D.sma_sp_com);
                    if (plot_kep_sp_com[1]) plot_kep_sp_com[1] = plot("##plot_kep_sp_com[1]", "Spacecraft eccentricity (binary COM)",                "e [  ]",  plot_kep_sp_com[1], sol2D.ecc_sp_com);
                    if (plot_kep_sp_com[2]) plot_kep_sp_com[2] = plot("##plot_kep_sp_com[2]", "Spacecraft inclination (binary COM)",                 "i [deg]", plot_kep_sp_com[2], sol2D.inc_sp_com);
                    if (plot_kep_sp_com[3]) plot_kep_sp_com[3] = plot("##plot_kep_sp_com[3]", "Spacecraft longitude of ascending node (binary COM)", "Ω [deg]", plot_kep_sp_com[3], sol2D.raan_sp_com);
                    if (plot_kep_sp_com[4]) plot_kep_sp_com[4] = plot("##plot_kep_sp_com[4]", "Spacecraft argument of periapsis (binary COM)",       "ω [deg]", plot_kep_sp_com[4], sol2D.argper_sp_com);
                    if (plot_kep_sp_com[5]) plot_kep_sp_com[5] = plot("##plot_kep_sp_com[5]", "Spacecraft mean anomaly (binary COM)",                "M [deg]", plot_kep_sp_com[5], sol2D.manom_sp_com);

                    if (plot_cart_sp_com1[0]) plot_cart_sp_com1[0] = plot("##plot_cart_sp_com1[0]", "Spacecraft x (body 1)",             "x [km]",       plot_cart_sp_com1[0], sol2D.xsp_com1);
                    if (plot_cart_sp_com1[1]) plot_cart_sp_com1[1] = plot("##plot_cart_sp_com1[1]", "Spacecraft y (body 1)",             "y [km]",       plot_cart_sp_com1[1], sol2D.ysp_com1);
                    if (plot_cart_sp_com1[2]) plot_cart_sp_com1[2] = plot("##plot_cart_sp_com1[2]", "Spacecraft z (body 1)",             "z [km]",       plot_cart_sp_com1[2], sol2D.zsp_com1);
                    if (plot_cart_sp_com1[3]) plot_cart_sp_com1[3] = plot("##plot_cart_sp_com1[3]", "Spacecraft distance (body 1)",      "d [km]",       plot_cart_sp_com1[3], sol2D.dist_sp_com1);
                    if (plot_cart_sp_com1[4]) plot_cart_sp_com1[4] = plot("##plot_cart_sp_com1[4]", "Spacecraft υx (body 1)",            "υx [km/sec]",  plot_cart_sp_com1[4], sol2D.vxsp_com1);
                    if (plot_cart_sp_com1[5]) plot_cart_sp_com1[5] = plot("##plot_cart_sp_com1[5]", "Spacecraft υy (body 1)",            "υy [km/sec]",  plot_cart_sp_com1[5], sol2D.vysp_com1);
                    if (plot_cart_sp_com1[6]) plot_cart_sp_com1[6] = plot("##plot_cart_sp_com1[6]", "Spacecraft υz (body 1)",            "υz [km/sec]",  plot_cart_sp_com1[6], sol2D.vzsp_com1);
                    if (plot_cart_sp_com1[7]) plot_cart_sp_com1[7] = plot("##plot_cart_sp_com1[7]", "Spacecraft υ (magnitude) (body 1)", "υ [km/sec]",   plot_cart_sp_com1[7], sol2D.vel_sp_com1);

                    if (plot_kep_sp_com1[0]) plot_kep_sp_com1[0] = plot("##plot_kep_sp_com1[0]", "Spacecraft semi - major axis (body 1)",           "a [km]",  plot_kep_sp_com1[0], sol2D.sma_sp_com1);
                    if (plot_kep_sp_com1[1]) plot_kep_sp_com1[1] = plot("##plot_kep_sp_com1[1]", "Spacecraft eccentricity (body 1)",                "e [  ]",  plot_kep_sp_com1[1], sol2D.ecc_sp_com1);
                    if (plot_kep_sp_com1[2]) plot_kep_sp_com1[2] = plot("##plot_kep_sp_com1[2]", "Spacecraft inclination (body 1)",                 "i [deg]", plot_kep_sp_com1[2], sol2D.inc_sp_com1);
                    if (plot_kep_sp_com1[3]) plot_kep_sp_com1[3] = plot("##plot_kep_sp_com1[3]", "Spacecraft longitude of ascending node (body 1)", "Ω [deg]", plot_kep_sp_com1[3], sol2D.raan_sp_com1);
                    if (plot_kep_sp_com1[4]) plot_kep_sp_com1[4] = plot("##plot_kep_sp_com1[4]", "Spacecraft argument of periapsis (body 1)",       "ω [deg]", plot_kep_sp_com1[4], sol2D.argper_sp_com1);
                    if (plot_kep_sp_com1[5]) plot_kep_sp_com1[5] = plot("##plot_kep_sp_com1[5]", "Spacecraft mean anomaly (body 1)",                "M [deg]", plot_kep_sp_com1[5], sol2D.manom_sp_com1);

                    if (plot_cart_sp_com2[0]) plot_cart_sp_com2[0] = plot("##plot_cart_sp_com2[0]", "Spacecraft x (body 2)",             "x [km]",       plot_cart_sp_com2[0], sol2D.xsp_com2);
                    if (plot_cart_sp_com2[1]) plot_cart_sp_com2[1] = plot("##plot_cart_sp_com2[1]", "Spacecraft y (body 2)",             "y [km]",       plot_cart_sp_com2[1], sol2D.ysp_com2);
                    if (plot_cart_sp_com2[2]) plot_cart_sp_com2[2] = plot("##plot_cart_sp_com2[2]", "Spacecraft z (body 2)",             "z [km]",       plot_cart_sp_com2[2], sol2D.zsp_com2);
                    if (plot_cart_sp_com2[3]) plot_cart_sp_com2[3] = plot("##plot_cart_sp_com2[3]", "Spacecraft distance (body 2)",      "d [km]",       plot_cart_sp_com2[3], sol2D.dist_sp_com2);
                    if (plot_cart_sp_com2[4]) plot_cart_sp_com2[4] = plot("##plot_cart_sp_com2[4]", "Spacecraft υx (body 2)",            "υx [km/sec]",  plot_cart_sp_com2[4], sol2D.vxsp_com2);
                    if (plot_cart_sp_com2[5]) plot_cart_sp_com2[5] = plot("##plot_cart_sp_com2[5]", "Spacecraft υy (body 2)",            "υy [km/sec]",  plot_cart_sp_com2[5], sol2D.vysp_com2);
                    if (plot_cart_sp_com2[6]) plot_cart_sp_com2[6] = plot("##plot_cart_sp_com2[6]", "Spacecraft υz (body 2)",            "υz [km/sec]",  plot_cart_sp_com2[6], sol2D.vzsp_com2);
                    if (plot_cart_sp_com2[7]) plot_cart_sp_com2[7] = plot("##plot_cart_sp_com2[7]", "Spacecraft υ (magnitude) (body 2)", "υ [km/sec]",   plot_cart_sp_com2[7], sol2D.vel_sp_com2);

                    if (plot_kep_sp_com2[0]) plot_kep_sp_com2[0] = plot("##plot_kep_sp_com2[0]", "Spacecraft semi - major axis (body 2)",           "a [km]",  plot_kep_sp_com2[0], sol2D.sma_sp_com2);
                    if (plot_kep_sp_com2[1]) plot_kep_sp_com2[1] = plot("##plot_kep_sp_com2[1]", "Spacecraft eccentricity (body 2)",                "e [  ]",  plot_kep_sp_com2[1], sol2D.ecc_sp_com2);
                    if (plot_kep_sp_com2[2]) plot_kep_sp_com2[2] = plot("##plot_kep_sp_com2[2]", "Spacecraft inclination (body 2)",                 "i [deg]", plot_kep_sp_com2[2], sol2D.inc_sp_com2);
                    if (plot_kep_sp_com2[3]) plot_kep_sp_com2[3] = plot("##plot_kep_sp_com2[3]", "Spacecraft longitude of ascending node (body 2)", "Ω [deg]", plot_kep_sp_com2[3], sol2D.raan_sp_com2);
                    if (plot_kep_sp_com2[4]) plot_kep_sp_com2[4] = plot("##plot_kep_sp_com2[4]", "Spacecraft argument of periapsis (body 2)",       "ω [deg]", plot_kep_sp_com2[4], sol2D.argper_sp_com2);
                    if (plot_kep_sp_com2[5]) plot_kep_sp_com2[5] = plot("##plot_kep_sp_com2[5]", "Spacecraft mean anomaly (body 2)",                "M [deg]", plot_kep_sp_com2[5], sol2D.manom_sp_com2);
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
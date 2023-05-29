#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"obj.hpp"
#include"conversion.hpp"

#include"solution.hpp"
#include"meshvfn.hpp"
#include"shader.hpp"
#include"skybox.hpp"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class Graphics
{
    
public:

    //x(t), y(t), z(t), r(t) magnitude (distance), υx(t), υy(t),υz(t), υ(t) magnitude
    bvec plot_cart;

    //a(t), e(t), i(t), Ω(t), ω(t), M(t)
    bvec plot_kep;

    //roll1(t), pitch1(t), yaw1(t), roll2(t), pitch2(t), yaw2(t)
    bvec plot_rpy1, plot_rpy2;
    //q10(t), q11(t), q12(t), q13(t), q20(t), q21(t), q22(t), q23(t)
    bvec plot_q1, plot_q2;

    //ω1ix(t), ω1iy(t), ω1iz(t),  ω1bx(t), ω1by(t), ω1bz(t)
    bvec plot_w1i, plot_w1b;
    //ω2ix(t), ω2ix(t), ω2ix(t),  ω2bx(t), ω2by(t), ω2bz(t)
    bvec plot_w2i, plot_w2b;

    //energy and momentum (magnitude) relative errors
    bvec plot_ener_mom_rel_err;

    bool view_panom, view_r, view_l, view_f, view_b, view_t, view_d;

    // video setup

    bool play_video;

    int current_frame;

    int solution_frame_rate;

    bool isVideoPaused;

    double previous_time;

    float camera_distance;

    //Properties properties;

    //orbit data
    Solution solution;

    Graphics() : plot_cart({false,false,false,false, false,false,false,false}),
                 plot_kep({false,false,false,false,false,false}),
                 plot_rpy1({false,false,false}),
                 plot_rpy2({false,false,false}),
                 plot_q1({false,false,false,false}),
                 plot_q2({false,false,false,false}),
                 plot_w1i({false,false,false}),
                 plot_w1b({false,false,false}),
                 plot_w2i({false,false,false}),
                 plot_w2b({false,false,false}),
                 plot_ener_mom_rel_err({false,false}),
                 view_panom(true),
                 view_r(false),
                 view_l(false),
                 view_f(false),
                 view_b(false),
                 view_t(false),
                 view_d(false),
                 play_video(false),
                 isVideoPaused(false),
                 current_frame(0),
                 solution_frame_rate(60),
                 previous_time(0.0),
                 camera_distance(4.0f)
    { }

    //void yield_properties(const Properties &properties)
    //{
    //    this->properties = properties;
    //    return;
    //}

    void yield_solution(const Solution &solution)
    {
        this->solution = solution;
        return;
    }

    bool common_plot_button(const char *label, bool plot_func)
    {
        if (plot_func)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        if (ImGui::Button(label)) plot_func = !plot_func;
        ImGui::PopStyleColor();
        return plot_func;
    }
    
    void plot_buttons()
    {
        ImGui::Text("Relative position");
        plot_cart[0] = common_plot_button("    x    ", plot_cart[0]); ImGui::SameLine();
        plot_cart[1] = common_plot_button("    y    ", plot_cart[1]); ImGui::SameLine();
        plot_cart[2] = common_plot_button("    z    ", plot_cart[2]); ImGui::SameLine();
        plot_cart[3] = common_plot_button("distance",  plot_cart[3]);
        ImGui::Separator();

        ImGui::Text("Relative velocity");
        plot_cart[4] = common_plot_button("   υx   ", plot_cart[4]); ImGui::SameLine();
        plot_cart[5] = common_plot_button("   υy   ", plot_cart[5]); ImGui::SameLine();
        plot_cart[6] = common_plot_button("   υz   ", plot_cart[6]); ImGui::SameLine();
        plot_cart[7] = common_plot_button("υ mag",    plot_cart[7]);
        ImGui::Separator();

        ImGui::Text("Relative Keplerian elements");
        plot_kep[0] = common_plot_button("   a   ", plot_kep[0]); ImGui::SameLine();
        plot_kep[1] = common_plot_button("   e   ", plot_kep[1]); ImGui::SameLine();
        plot_kep[2] = common_plot_button("   i   ", plot_kep[2]); ImGui::SameLine();
        plot_kep[3] = common_plot_button("   Ω   ", plot_kep[3]); ImGui::SameLine();
        plot_kep[4] = common_plot_button("   ω   ", plot_kep[4]); ImGui::SameLine();
        plot_kep[5] = common_plot_button("   M   ", plot_kep[5]);
        ImGui::Separator();

        ImGui::Text("Euler angles (Body 1)");
        plot_rpy1[0] = common_plot_button("roll 1",  plot_rpy1[0]); ImGui::SameLine();
        plot_rpy1[1] = common_plot_button("pitch 1", plot_rpy1[1]); ImGui::SameLine();
        plot_rpy1[2] = common_plot_button("yaw 1",   plot_rpy1[2]);
        ImGui::Separator();

        ImGui::Text("Euler angles (Body 2)");
        plot_rpy2[0] = common_plot_button("roll 2",  plot_rpy2[0]); ImGui::SameLine();
        plot_rpy2[1] = common_plot_button("pitch 2", plot_rpy2[1]); ImGui::SameLine();
        plot_rpy2[2] = common_plot_button("yaw 2",   plot_rpy2[2]);
        ImGui::Separator();
        
        ImGui::Text("Quaternion (Body 1)");
        plot_q1[0] = common_plot_button("q10", plot_q1[0]); ImGui::SameLine();
        plot_q1[1] = common_plot_button("q11", plot_q1[1]); ImGui::SameLine();
        plot_q1[2] = common_plot_button("q12", plot_q1[2]); ImGui::SameLine();
        plot_q1[3] = common_plot_button("q13", plot_q1[3]);
        ImGui::Separator();
        
        ImGui::Text("Quaternion (Body 2)");
        plot_q2[0] = common_plot_button("q20", plot_q2[0]); ImGui::SameLine();
        plot_q2[1] = common_plot_button("q21", plot_q2[1]); ImGui::SameLine();
        plot_q2[2] = common_plot_button("q22", plot_q2[2]); ImGui::SameLine();
        plot_q2[3] = common_plot_button("q23", plot_q2[3]);
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 1, Inertial)");
        plot_w1i[0] = common_plot_button(" ω1ix ", plot_w1i[0]); ImGui::SameLine();
        plot_w1i[1] = common_plot_button(" ω1iy ", plot_w1i[1]); ImGui::SameLine();
        plot_w1i[2] = common_plot_button(" ω1iz ", plot_w1i[2]);
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 1, Body)");
        plot_w1b[0] = common_plot_button(" ω1bx ", plot_w1b[0]); ImGui::SameLine();
        plot_w1b[1] = common_plot_button(" ω1by ", plot_w1b[1]); ImGui::SameLine();
        plot_w1b[2] = common_plot_button(" ω1bz ", plot_w1b[2]);
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 2, Inertial)");
        plot_w2i[0] = common_plot_button(" ω2ix ", plot_w2i[0]); ImGui::SameLine();
        plot_w2i[1] = common_plot_button(" ω2iy ", plot_w2i[1]); ImGui::SameLine();
        plot_w2i[2] = common_plot_button(" ω2iz ", plot_w2i[2]);
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 2, Body)");
        plot_w2b[0] = common_plot_button(" ω2bx ", plot_w2b[0]); ImGui::SameLine();
        plot_w2b[1] = common_plot_button(" ω2by ", plot_w2b[1]); ImGui::SameLine();
        plot_w2b[2] = common_plot_button(" ω2bz ", plot_w2b[2]);
        ImGui::Separator();

        ImGui::Text("Constants of motion");
        plot_ener_mom_rel_err[0] = common_plot_button("Energy",       plot_ener_mom_rel_err[0]); ImGui::SameLine();
        plot_ener_mom_rel_err[1] = common_plot_button("Momentum mag", plot_ener_mom_rel_err[1]);
        ImGui::Separator();

        return;
    }

    bool common_plot(const char *begin_id, const char *begin_plot_id, const char *yaxis_str, bool bool_plot_func, dvec &plot_func)
    {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.8f*ImGui::GetIO().DisplaySize.x/3.0f, 0.8f*3.0f*ImGui::GetIO().DisplaySize.x/12.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin(begin_id, &bool_plot_func);
        ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        if (ImPlot::BeginPlot(begin_plot_id, plot_win_size))
        {
            ImPlot::SetupAxes("time [days]", yaxis_str);
            ImPlot::PlotLine("", &(solution.t[0]), &plot_func[0], solution.t.size());
            ImVec4 ForestGreen(0.2,0.6,0.2,1.0);
            ImVec4 DarkRed(0.6,0.2,0.2,1.0);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 3.5, ForestGreen, -1.0, ForestGreen);
            ImPlot::PlotScatter("Current Frame", &(solution.t[current_frame]), &(plot_func[current_frame]), 1, 2.0);
            if (solution.collision){
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 3.5, DarkRed, -1.0, DarkRed);
                ImPlot::PlotScatter("Collision", &(solution.t[solution.t.size()-1]), &(plot_func[solution.t.size()-1]), 1, 2.0);
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
        return bool_plot_func;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool video_content(const bool play_video)
    {
        glClear(GL_DEPTH_BUFFER_BIT);

        bool temp_play_video = play_video;
     
        static Meshvfn aster1(solution.obj_path1.c_str());
        static Meshvfn aster2(solution.obj_path2.c_str());
        static shader aster_shad("../shaders/vertex/trans_mvpn.vert", "../shaders/fragment/dir_light_ad.frag");

        glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f);
        glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f/1080.0f, 0.01f,100.0f);
        glm::mat4 view;
        if (view_panom)
           view = glm::lookAt(glm::vec3(camera_distance,camera_distance,camera_distance), cam_aim, cam_up);
        else if (view_r)
           view = glm::lookAt(glm::vec3(camera_distance,0.0f,0.0f), cam_aim, cam_up);
        else if (view_l)
           view = glm::lookAt(glm::vec3(-camera_distance,0.0f,0.0f), cam_aim, cam_up);
        else if (view_f)
           view = glm::lookAt(glm::vec3(0.0f,camera_distance,0.0f), cam_aim, cam_up);
        else if (view_b)
           view = glm::lookAt(glm::vec3(0.0f,-camera_distance,0.0f), cam_aim, cam_up);
        else if (view_t)
           view = glm::lookAt(glm::vec3(0.0f,-0.01f,camera_distance), cam_aim, cam_up);
        else if (view_d)
           view = glm::lookAt(glm::vec3(0.0f,-0.01f,-camera_distance), cam_aim, cam_up);
        
        aster_shad.use();

        aster_shad.set_mat4_uniform("projection", projection);
        aster_shad.set_mat4_uniform("view", view);

        double cm1fac = -solution.M2/(solution.M1 + solution.M2);
        double cm2fac =  solution.M1/(solution.M1 + solution.M2);
        
        int max_frame = solution.x.size()-1;
        
        double now_time = glfwGetTime();
        double delta_time = now_time - previous_time;
        if( delta_time > 1.0f/solution_frame_rate){

            if(current_frame == max_frame || isVideoPaused ){

            } else{

                current_frame++;
            }
            previous_time = now_time;
        }           
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((float)cm1fac*solution.x[current_frame],(float)cm1fac*solution.y[current_frame],(float)cm1fac* solution.z[current_frame]));
        model = glm::rotate(model, (float)solution.yaw1[current_frame]  , glm::vec3(0.0f,0.0f,1.0f));
        model = glm::rotate(model, (float)solution.pitch1[current_frame], glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, (float)solution.roll1[current_frame] , glm::vec3(1.0f,0.0f,0.0f));
        if(solution.ell_checkbox){
            model = glm::scale(model,glm::vec3(solution.semiaxes1[0],solution.semiaxes1[1],solution.semiaxes1[2]));
        }
        aster_shad.set_mat4_uniform("model", model);
        aster1.draw();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((float)cm2fac*solution.x[current_frame],(float)cm2fac*solution.y[current_frame],(float)cm2fac* solution.z[current_frame]));
        model = glm::rotate(model, (float)solution.yaw2[current_frame]   , glm::vec3(0.0f,0.0f,1.0f));
        model = glm::rotate(model, (float)solution.pitch2[current_frame] , glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, (float)solution.roll2[current_frame]  , glm::vec3(1.0f,0.0f,0.0f));
        if(solution.ell_checkbox){
            model = glm::scale(model,glm::vec3(solution.semiaxes2[0],solution.semiaxes2[1],solution.semiaxes2[2]));
        }
        aster_shad.set_mat4_uniform("model", model);
        aster2.draw();


        // Skybox
        static Skybox sky;
        static shader sky_shad("../shaders/vertex/skybox.vert" , "../shaders/fragment/skybox.frag");
        glDepthFunc(GL_LEQUAL);
            sky_shad.use();
            projection = glm::perspective(glm::radians(45.0f), 1920.0f/1080.0f, 0.01f, 100.0f);
            if (view_panom)
               view = glm::mat4(glm::mat3(glm::lookAt(glm::vec3(camera_distance,camera_distance,camera_distance), cam_aim, cam_up)));
            
            sky_shad.set_mat4_uniform("projection", projection);
            sky_shad.set_mat4_uniform("view", view);
            sky.draw();
        glDepthFunc(GL_LESS);

        return temp_play_video;
    }

    void video_buttons()
    {
        if (ImGui::TreeNodeEx("Camera"))
        {
            if (ImGui::Checkbox("Panoramic", &view_panom))
            {
                if (!view_r && !view_l && !view_f && !view_b && !view_t && !view_d)
                    view_panom = true;
                else
                    view_r = view_l = view_f = view_b = view_t = view_d = false;
            }
            if (ImGui::Checkbox("Right ( + x )", &view_r))
            {
                if (!view_panom && !view_l && !view_f && !view_b && !view_t && !view_d)
                    view_r = true;
                else
                    view_panom = view_l = view_f = view_b = view_t = view_d = false;
            }
            if (ImGui::Checkbox("Left ( - x )", &view_l))
            {
                if (!view_panom && !view_r && !view_f && !view_b && !view_t && !view_d)
                    view_l = true;
                else
                    view_panom = view_r = view_f = view_b = view_t = view_d = false;
            }
            if (ImGui::Checkbox("Front ( + y )", &view_f))
            {
                if (!view_panom && !view_r && !view_l && !view_b && !view_t && !view_d)
                    view_f = true;
                else
                    view_panom = view_r = view_l = view_b = view_t = view_d = false;
            }
            if (ImGui::Checkbox("Back ( - y )", &view_b))
            {
                if (!view_panom && !view_r && !view_l && !view_f && !view_t && !view_d)
                    view_b = true;
                else
                    view_panom = view_r = view_l = view_f = view_t = view_d = false;
            }
            if (ImGui::Checkbox("Top ( + z )", &view_t))
            {
                if (!view_panom && !view_r && !view_l && !view_f && !view_b && !view_d)
                    view_t = true;
                else
                    view_panom = view_r = view_l = view_f = view_b = view_d = false;
            }
            if (ImGui::Checkbox("Down ( - z )", &view_d))
            {
                if (!view_panom && !view_r && !view_l && !view_f && !view_b && !view_t)
                    view_d = true;
                else
                    view_panom = view_r = view_l = view_f = view_b = view_t = false;
            }
            

            ImGui::TreePop();
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        if (ImGui::Button("Show")) play_video = !play_video;
        ImGui::SameLine();
        if (isVideoPaused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        if (ImGui::Button("Pause"))  isVideoPaused = !isVideoPaused;
        ImGui::PopStyleColor();
         
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            current_frame = 0;
        };
        ImGui::Separator();
        ImGui::LabelText("##","Current Frame");
        ImGui::SliderInt("[# Frame]", &current_frame, 0, solution.x.size()-1);
        ImGui::Separator();
        ImGui::LabelText("##","Video Speed");
        ImGui::SliderInt("[FPS]", &solution_frame_rate, 1, 60);
        ImGui::LabelText("##","Camera Distance");
        ImGui::SliderFloat("[km]", &camera_distance, 0.5 * solution.max_dist, 5.0*solution.max_dist);
    }

    void render()
    {
        ImGui::SetNextWindowPos( ImVec2(6.0f*ImGui::GetIO().DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(     ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graphics", nullptr);

        if (ImGui::CollapsingHeader("Plots"))
        {
            if (!solution.t.size())
            {
                ImGui::BeginDisabled();
                    plot_buttons();
                ImGui::EndDisabled();
            }
            else
            {
                plot_buttons();

                if (plot_cart[0]) plot_cart[0] = common_plot("##1",  "##2",  "x [km]",               plot_cart[0], solution.x);
                if (plot_cart[1]) plot_cart[1] = common_plot("##3",  "##4",  "y [km]",               plot_cart[1], solution.y);
                if (plot_cart[2]) plot_cart[2] = common_plot("##5",  "##6",  "z [km]",               plot_cart[2], solution.z);
                if (plot_cart[3]) plot_cart[3] = common_plot("##7",  "##8",  "distance [km]",        plot_cart[3], solution.dist);
                if (plot_cart[4]) plot_cart[4] = common_plot("##9",  "##10", "υx [km/sec]",          plot_cart[4], solution.vx);
                if (plot_cart[5]) plot_cart[5] = common_plot("##11", "##12", "υy [km/sec]",          plot_cart[5], solution.vy);
                if (plot_cart[6]) plot_cart[6] = common_plot("##13", "##14", "υz [km/sec]",          plot_cart[6], solution.vz);
                if (plot_cart[7]) plot_cart[7] = common_plot("##15", "##16", "υ magnitude [km/sec]", plot_cart[7], solution.vmag);

                if (plot_kep[0]) plot_kep[0] = common_plot("##17", "##18", "a [km]",  plot_kep[0], solution.a);
                if (plot_kep[1]) plot_kep[1] = common_plot("##19", "##20", "e [  ]",  plot_kep[1], solution.e);
                if (plot_kep[2]) plot_kep[2] = common_plot("##21", "##22", "i [deg]", plot_kep[2], solution.inc);
                if (plot_kep[3]) plot_kep[3] = common_plot("##23", "##24", "Ω [deg]", plot_kep[3], solution.Om);
                if (plot_kep[4]) plot_kep[4] = common_plot("##25", "##26", "ω [deg]", plot_kep[4], solution.w);
                if (plot_kep[5]) plot_kep[5] = common_plot("##27", "##28", "M [deg]", plot_kep[5], solution.M);

                if (plot_rpy1[0]) plot_rpy1[0] = common_plot("##29", "##30", "roll 1 [deg]",  plot_rpy1[0], solution.roll1);
                if (plot_rpy1[1]) plot_rpy1[1] = common_plot("##31", "##32", "pitch 1 [deg]", plot_rpy1[1], solution.pitch1);
                if (plot_rpy1[2]) plot_rpy1[2] = common_plot("##33", "##34", "yaw 1 [deg]",   plot_rpy1[2], solution.yaw1);

                if (plot_rpy2[0]) plot_rpy2[0] = common_plot("##35", "##36", "roll 2 [deg]",  plot_rpy2[0], solution.roll2);
                if (plot_rpy2[1]) plot_rpy2[1] = common_plot("##37", "##38", "pitch 2 [deg]", plot_rpy2[1], solution.pitch2);
                if (plot_rpy2[2]) plot_rpy2[2] = common_plot("##38", "##40", "yaw 2 [deg]",   plot_rpy2[2], solution.yaw2);

                if (plot_q1[0]) plot_q1[0] = common_plot("##41", "##42", "q10 [  ]", plot_q1[0], solution.q10);
                if (plot_q1[1]) plot_q1[1] = common_plot("##43", "##44", "q11 [  ]", plot_q1[1], solution.q11);
                if (plot_q1[2]) plot_q1[2] = common_plot("##45", "##46", "q12 [  ]", plot_q1[2], solution.q12);
                if (plot_q1[3]) plot_q1[3] = common_plot("##47", "##48", "q13 [  ]", plot_q1[3], solution.q13);

                if (plot_q2[0]) plot_q2[0] = common_plot("##49", "##50", "q20 [  ]", plot_q2[0], solution.q20);
                if (plot_q2[1]) plot_q2[1] = common_plot("##51", "##52", "q21 [  ]", plot_q2[1], solution.q21);
                if (plot_q2[2]) plot_q2[2] = common_plot("##53", "##54", "q22 [  ]", plot_q2[2], solution.q22);
                if (plot_q2[3]) plot_q2[3] = common_plot("##55", "##56", "q23 [  ]", plot_q2[3], solution.q23);

                if (plot_w1i[0]) plot_w1i[0] = common_plot("##57", "##58", "ω1ix [rad/sec]", plot_w1i[0], solution.w1ix);
                if (plot_w1i[1]) plot_w1i[1] = common_plot("##59", "##60", "ω1iy [rad/sec]", plot_w1i[1], solution.w1iy);
                if (plot_w1i[2]) plot_w1i[2] = common_plot("##61", "##62", "ω1iz [rad/sec]", plot_w1i[2], solution.w1iz);

                if (plot_w1b[0]) plot_w1b[0] = common_plot("##63", "##64", "ω1bx [rad/sec]", plot_w1b[0], solution.w1bx);
                if (plot_w1b[1]) plot_w1b[1] = common_plot("##65", "##66", "ω1by [rad/sec]", plot_w1b[1], solution.w1by);
                if (plot_w1b[2]) plot_w1b[2] = common_plot("##67", "##68", "ω1bz [rad/sec]", plot_w1b[2], solution.w1bz);

                if (plot_w2i[0]) plot_w2i[0] = common_plot("##69", "##70", "ω2ix [rad/sec]", plot_w2i[0], solution.w2ix);
                if (plot_w2i[1]) plot_w2i[1] = common_plot("##71", "##72", "ω2iy [rad/sec]", plot_w2i[1], solution.w2iy);
                if (plot_w2i[2]) plot_w2i[2] = common_plot("##73", "##74", "ω2iz [rad/sec]", plot_w2i[2], solution.w2iz);

                if (plot_w2b[0]) plot_w2b[0] = common_plot("##75", "##76", "ω2bx [rad/sec]", plot_w2b[0], solution.w2bx);
                if (plot_w2b[1]) plot_w2b[1] = common_plot("##77", "##78", "ω2by [rad/sec]", plot_w2b[1], solution.w2by);
                if (plot_w2b[2]) plot_w2b[2] = common_plot("##79", "##80", "ω2bz [rad/sec]", plot_w2b[2], solution.w2bz);

                if (plot_ener_mom_rel_err[0]) plot_ener_mom_rel_err[0] = common_plot("##81", "##82", "energy error [  ]",   plot_ener_mom_rel_err[0], solution.ener_rel_err);
                if (plot_ener_mom_rel_err[1]) plot_ener_mom_rel_err[1] = common_plot("##83", "##84", "momentum error [  ]", plot_ener_mom_rel_err[1], solution.mom_rel_err);
            }
        }

        if (ImGui::CollapsingHeader("Video"))
        {
            if (!solution.t.size())
            {
                ImGui::BeginDisabled();
                    video_buttons();
                ImGui::EndDisabled();
            }
            else
            {
                video_buttons();
                if (play_video) play_video = video_content(play_video);

            }
        }








        ImGui::End();
    }



};

#endif
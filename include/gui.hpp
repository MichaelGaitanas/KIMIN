#ifndef GUI_HPP
#define GUI_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

#include"io.hpp"

class gui
{
public:
    ImGuiIO &io; //this reference will be (and must be) initialized in the constructor gui(...)

    //We ought to call ImGui::CreateContext() before ImGui::GetIO(). But ImGui::GetIO() will be called once an instance of the class is created.
    //The following static member function serves the aforementioned purpose : To be able to call ImGui::CreateContext() without having an instance of the class.
    static void create_imgui_context()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
    }

    //Constructor and correct initialization of &io.
    gui(GLFWwindow *pointer) : io(ImGui::GetIO())
    {
        io.IniFilename = NULL;
        io.Fonts->AddFontFromFileTTF("../font/arial.ttf", 15.0f);
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    //Destructor
    ~gui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    //Create an new imgui frame.
    void new_frame()
    {
        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    //Render the gui stuff.
    void render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    //KIMIN's left gui panel (mainly simulation inputs).
    void left_panel(inputs &ins, GLFWwindow *pointer)
    {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x/7.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties ",&ins.xclose, ImGuiWindowFlags_MenuBar);
        if (!ins.xclose)
            glfwSetWindowShouldClose(pointer, true);

        //classical menu bar section
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Import"))
            {
                if (ImGui::MenuItem("Properties"))
                {
                    //Import ONE file (json or txt) which contains a standard (example-like) set of all the member variables of the 'inputs' class.
                }
                if (ImGui::MenuItem("State vector"))
                {
                    //Import a bunch of files (jsin or txt) that enhold state vector of one simulation run.
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("Simulation name");
        ImGui::PushItemWidth(200);
            ImGui::InputText(" ", ins.simname, IM_ARRAYSIZE(ins.simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Shape models");
        if (ImGui::Checkbox("Ellipsoids", &ins.ell_checkbox) && ins.ell_checkbox)
        {
            ins.clicked_ell_ok = false;
        }
        if (ins.ell_checkbox && !ins.clicked_ell_ok)
        {
            ins.obj_checkbox = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(300,300), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters");
                
            //ellipsoid semiaxes submenu
            ImGui::Text("Semiaxes");
            //keyboard input : a1 semiaxis
            ImGui::Text("a1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(0);
                    ImGui::InputDouble("[km]", &ins.semiaxes1[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : b1 semiaxis
            ImGui::Text("b1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(1);
                    ImGui::InputDouble("[km]", &ins.semiaxes1[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : c1 semiaxis
            ImGui::Text("c1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(2);
                    ImGui::InputDouble("[km]", &ins.semiaxes1[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            //keyboard input : a2 semiaxis
            ImGui::Text("a2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(3);
                    ImGui::InputDouble("[km]", &ins.semiaxes2[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : b2 semiaxis
            ImGui::Text("b2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(4);
                    ImGui::InputDouble("[km]", &ins.semiaxes2[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : c2 semiaxis
            ImGui::Text("c2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(5);
                    ImGui::InputDouble("[km]", &ins.semiaxes2[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //mouse input : ellipsoid submenu "OK" button
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
            {
                ins.clicked_ell_ok = true;
            }
            ImGui::End();
        }

        //mouse input : .obj shape model (either mascons or polyhedra)
        if (ImGui::Checkbox(".obj files", &ins.obj_checkbox) && ins.obj_checkbox)
        {
            ins.clicked_obj_ok = false;
        }
        if (ins.obj_checkbox && !ins.clicked_obj_ok)
        {
            ins.ell_checkbox = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); //display position of the obj files menu 
            ImGui::SetNextWindowSize(ImVec2(300,300), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj parameters");
            ImGui::Text("KIMIN's .obj database\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            //mouse input : body1 or body2 via radio buttons
            if (ImGui::RadioButton("Body 1", ins.obj_refer_to_body == 1))
                ins.obj_refer_to_body = 1;
            if (ImGui::RadioButton("Body 2", ins.obj_refer_to_body == 2))
                ins.obj_refer_to_body = 2;
            
            if (ImGui::TreeNodeEx("Mascon models"))
            {
                for (int i = 0; i < ins.path_to_masc_obj.size(); ++i)
                {
                    if (ins.obj_refer_to_body == 1)
                    {
                        //mouse input : which .obj model path for body1
                        if (ImGui::Selectable(ins.path_to_masc_obj[i].string().c_str(), (ins.clicked_masc1_index == i)))
                        {
                            ins.clicked_poly1 = false;
                            ins.clicked_masc1_index = i;
                            ins.clicked_poly1_index = -1;
                            ins.obj_path1 = ins.path_to_masc_obj[i].string();
                        }
                    }
                    else
                    {
                        //mouse input : which .obj model path for body2
                        if (ImGui::Selectable(ins.path_to_masc_obj[i].string().c_str(), (ins.clicked_masc2_index == i)))
                        {
                            ins.clicked_poly2 = false;
                            ins.clicked_masc2_index = i;
                            ins.clicked_poly2_index = -1;
                            ins.obj_path2 = ins.path_to_masc_obj[i].string();
                        }
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Polyhedral models"))
            {
                for (int i = 0; i < ins.path_to_poly_obj.size(); ++i)
                {
                    //mouse input : which .obj model path for body1
                    if (ins.obj_refer_to_body == 1)
                    {
                        if (ImGui::Selectable(ins.path_to_poly_obj[i].string().c_str(), (ins.clicked_poly1_index == i)))
                        {
                            ins.clicked_poly1 = true;
                            ins.clicked_masc1_index = -1;
                            ins.clicked_poly1_index = i;
                            ins.obj_path1 = ins.path_to_poly_obj[i].string();
                        }
                    }
                    else
                    {
                        //mouse input : which .obj model path for body2
                        if (ImGui::Selectable(ins.path_to_poly_obj[i].string().c_str(), (ins.clicked_poly2_index == i)))
                        {
                            ins.clicked_poly2 = true;
                            ins.clicked_masc2_index = -1;
                            ins.clicked_poly2_index = i;
                            ins.obj_path2 = ins.path_to_poly_obj[i].string();
                        }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,5.0f));

            ImGui::Indent();
            ImGui::Text("Grid resolution ");

            if ((ins.obj_refer_to_body == 1 && !ins.clicked_poly1) ||
                (ins.obj_refer_to_body == 2 && !ins.clicked_poly2) ||
                (!ins.clicked_poly1 && !ins.clicked_poly2))
            {
                //pseudo mouse or keyboard inputs (basically you cannot input anything if the following commands are executed)

                ImGui::BeginDisabled();
                    ImGui::Text("x axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(6);
                            ImGui::InputInt(" [ > 1 ]", &ins.grid_reso_null[0]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::Text("y axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(7);
                            ImGui::InputInt(" [ > 1 ]", &ins.grid_reso_null[1]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::Text("z axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(8);
                            ImGui::InputInt(" [ > 1 ]", &ins.grid_reso_null[2]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                ImGui::EndDisabled();
            }
            else if (ins.obj_refer_to_body == 1 && ins.clicked_poly1)
            {
                //mouse or keyboard input : body1 x grid resolution for the raycast
                ImGui::Text("x axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(9);
                        ImGui::InputInt(" [ > 1 ]", &ins.grid_reso1[0]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body1 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(10);
                        ImGui::InputInt(" [ > 1 ]", &ins.grid_reso1[1]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body1 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(11);
                        ImGui::InputInt(" [ > 1 ]", &ins.grid_reso1[2]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
            else if (ins.obj_refer_to_body == 2 && ins.clicked_poly2)
            {
                //mouse or keyboard input : body2 x grid resolution for the raycast
                ImGui::Text("x axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(12);
                        ImGui::InputInt(" [ > 1 ]", &ins.grid_reso2[0]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body2 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(13);
                        ImGui::InputInt(" [ > 1 ]", &ins.grid_reso2[1]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body2 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(14);
                        ImGui::InputInt(" [ > 1 ]", &ins.grid_reso2[2]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
            ImGui::Unindent();

            ImGui::Dummy(ImVec2(0.0f, 15.0f));

            //mouse input : .obj submenu "OK" button
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))   
            {
                ins.clicked_obj_ok = true;
            }

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        //mouse input : physics theory
        ImGui::Text("Theory");
        if (ImGui::Checkbox("Order 2", &ins.ord2_checkbox))
        {
            ins.ord3_checkbox = false;
            ins.ord4_checkbox = false;
            ins.mascons_checkbox = false;
        }
        if (ImGui::Checkbox("Order 3", &ins.ord3_checkbox))
        {
            ins.ord2_checkbox = false;
            ins.ord4_checkbox = false;
            ins.mascons_checkbox = false;
        }
        if (ImGui::Checkbox("Order 4", &ins.ord4_checkbox))
        {
            ins.ord2_checkbox = false;
            ins.ord3_checkbox = false;
            ins.mascons_checkbox = false;
        }
        if (ImGui::Checkbox("Mascons", &ins.mascons_checkbox))
        {
            ins.ord2_checkbox = false;
            ins.ord3_checkbox = false;
            ins.ord4_checkbox = false;
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Mass");

        //keyboard input : M1
        ImGui::Text("M1 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(15);
                ImGui::InputDouble("[kg]", &ins.M1, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        
        //keyboard input : M2
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(16);
                ImGui::InputDouble("[kg]", &ins.M2, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Text("Integration time");

        //keyboard input : Epoch
        ImGui::Text("Epoch     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(17);
                ImGui::InputDouble(" [days]", &ins.epoch, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();

        //keyboard input : Duration
        ImGui::Text("Duration  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(18);
                ImGui::InputDouble("[days]", &ins.dur, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();

        //keyboard input : Step
        ImGui::Text("Step        ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(19);
                ImGui::InputDouble("[days]", &ins.step, 0.0, 0.0,"%.9lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::Text("Initial state");
        ImGui::Indent();
        ImGui::Text("Relative position and velocity");

        //mouse input : position/velocity variables (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(20);
                ImGui::Combo("  ", &ins.cart_kep_var_choice, ins.cart_kep_var, IM_ARRAYSIZE(ins.cart_kep_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (ins.cart_kep_var_choice == 0)
        {
            //keyboard input : relative position x
            ImGui::Text("x    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(21);
                    ImGui::InputDouble("[km]", &ins.relcart[0], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative position y
            ImGui::Text("y    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(22);
                    ImGui::InputDouble("[km]", &ins.relcart[1], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative position z
            ImGui::Text("z    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(23);
                    ImGui::InputDouble("[km]", &ins.relcart[2], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative velocity vx
            ImGui::Text("vx  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(24);
                    ImGui::InputDouble("[km/sec]", &ins.relcart[3], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : relative velocity vy
            ImGui::Text("vy  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(25);
                    ImGui::InputDouble("[km/sec]", &ins.relcart[4], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative velocity vz
            ImGui::Text("vz  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(26);
                    ImGui::InputDouble("[km/sec]", &ins.relcart[5], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

        }
        else if (ins.cart_kep_var_choice == 1)
        {
            //keyboard input : relative semi-major axis a
            ImGui::Text("a       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(27);
                    ImGui::InputDouble("[km]", &ins.relkep[0], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
                        
            //keyboard input : relative eccentricity e
            ImGui::Text("e       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(28);
                    ImGui::InputDouble("[  ]", &ins.relkep[1], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative inclination i
            ImGui::Text("i        ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(29);
                    ImGui::InputDouble("[deg]", &ins.relkep[2], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative longitude of ascending node raan
            ImGui::Text("raan  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(30);
                    ImGui::InputDouble("[deg]", &ins.relkep[3], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative argument of periapsis w
            ImGui::Text("w       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(31);
                    ImGui::InputDouble("[deg]", &ins.relkep[4], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative mean anomaly M
            ImGui::Text("M      ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(32);
                    ImGui::InputDouble("[deg]", &ins.relkep[5], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }

        ImGui::Text("Orientations");

        //mouse input : orientation variables (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(33);
                ImGui::Combo("  ", &ins.orient_var_choice, ins.orient_var, IM_ARRAYSIZE(ins.orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (ins.orient_var_choice == 0)
        {
            //keyboard input : roll 1 angle
            ImGui::Text("roll 1    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(34);
                    ImGui::InputDouble("[deg]", &ins.rpy1[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : pitch 1 angle
            ImGui::Text("pitch 1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0);
                ImGui::PushID(35);
                    ImGui::InputDouble("[deg]", &ins.rpy1[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : yaw 1 angle
            ImGui::Text("yaw 1  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(36);
                    ImGui::InputDouble("[deg]", &ins.rpy1[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : roll 2 angle
            ImGui::Text("roll 2    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(37);
                    ImGui::InputDouble("[deg]", &ins.rpy2[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : pitch 2 angle
            ImGui::Text("pitch 2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(38);
                    ImGui::InputDouble("[deg]", &ins.rpy2[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : yaw 2 angle
            ImGui::Text("yaw 2  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(39);
                    ImGui::InputDouble("[deg]", &ins.rpy2[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }
        else if (ins.orient_var_choice == 1)
        {
            //keyboard input : q10 quaternion component
            ImGui::Text("q10   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(40);
                    ImGui::InputDouble("[  ]", &ins.q1[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q11 quaternion component
            ImGui::Text("q11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(41);
                    ImGui::InputDouble("[  ]", &ins.q1[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q12 quaternion component
            ImGui::Text("q12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(42);
                    ImGui::InputDouble("[  ]", &ins.q1[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : q13 quaternion component
            ImGui::Text("q13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(43);
                    ImGui::InputDouble("[  ]", &ins.q1[3], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q20 quaternion component
            ImGui::Text("q20   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(44);
                    ImGui::InputDouble("[  ]", &ins.q2[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q21 quaternion component
            ImGui::Text("q21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(45);
                    ImGui::InputDouble("[  ]", &ins.q2[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q22 quaternion component
            ImGui::Text("q22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(46);
                    ImGui::InputDouble("[  ]", &ins.q2[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //field : q23 quaternion component
            ImGui::Text("q23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(47);
                    ImGui::InputDouble("[  ]", &ins.q2[3], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }

        ImGui::Text("Angular velocity");

        //mouse input : frame choice (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(48);
                ImGui::Combo("  ", &ins.frame_type_choice, ins.frame_type, IM_ARRAYSIZE(ins.frame_type));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (ins.frame_type_choice == 0)
        {
            //keyboard input : w1x (inertial) angular velocity
            ImGui::Text("w1x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(49);
                    ImGui::InputDouble("[rad/sec]", &ins.w1i[0], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w1y (inertial) angular velocity
            ImGui::Text("w1y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(50);
                    ImGui::InputDouble("[rad/sec]", &ins.w1i[1], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : w1z (inertial) angular velocity
            ImGui::Text("w1z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(51);
                    ImGui::InputDouble("[rad/sec]", &ins.w1i[2], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w2x (inertial) angular velocity
            ImGui::Text("w2x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(52);
                    ImGui::InputDouble("[rad/sec]", &ins.w2i[0], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w2y (inertial) angular velocity
            ImGui::Text("w2y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(53);
                    ImGui::InputDouble("[rad/sec]", &ins.w2i[1], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : w2z (inertial) angular velocity
            ImGui::Text("w2z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(54);
                    ImGui::InputDouble("[rad/sec]", &ins.w2i[2], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

        }
        else if (ins.frame_type_choice == 1)
        {
            //keyboard input : w11 (body) angular velocity
            ImGui::Text("w11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(55);
                    ImGui::InputDouble("[rad/sec]", &ins.w1b[0], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w12 (body) angular velocity
            ImGui::Text("w12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(56);
                    ImGui::InputDouble("[rad/sec]", &ins.w1b[1], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w13 (body) angular velocity
            ImGui::Text("w13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(57);
                    ImGui::InputDouble("[rad/sec]", &ins.w1b[2], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w21 (body) angular velocity
            ImGui::Text("w21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(58);
                    ImGui::InputDouble("[rad/sec]", &ins.w2b[0], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w22 (body) angular velocity
            ImGui::Text("w22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(59);
                    ImGui::InputDouble("[rad/sec]", &ins.w2b[1], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w23 (body) angular velocity
            ImGui::Text("w23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(60);
                    ImGui::InputDouble("[rad/sec]", &ins.w2b[2], 0.0, 0.0,"%.10lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Unindent();

        ImGui::Text("Integrator");

        //mouse input : integration method
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(61);
                ImGui::Combo("  ", &ins.integ_method_choice, ins.integ_method, IM_ARRAYSIZE(ins.integ_method));
            ImGui::PopID();
        ImGui::PopItemWidth();

        //keyboard input : integration method tolerance
        ImGui::Text("tolerance ");
        ImGui::SameLine();
        ImGui::PushItemWidth(200.0);
            ImGui::PushID(62);
                ImGui::InputDouble("", &ins.tol, 0.0, 0.0,"%e");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        //mouse input : whether to render OpenGL video or not
        ImGui::Checkbox("Playback video", &ins.playback_video);
        ImGui::Dummy(ImVec2(0.0f,20.0f));

        //mouse input : run button
        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            ins.clicked_run = true;

        ImGui::End();
    }

    //KIMIN's right gui panel (3D graphics controls during the video).
    void right_panel()
    {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - io.DisplaySize.x/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x/7.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graphics", NULL);
        if (ImGui::CollapsingHeader("Plots"))
        {
            if (ImGui::Checkbox("Ellipsoids", &ins.ell_checkbox));
        }
        if (ImGui::CollapsingHeader("Simulation video"))
        {
            ImGui::Text("Integrator");
        }
        ImGui::End();
    }
};

#endif
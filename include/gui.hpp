#ifndef GUI_HPP
#define GUI_HPP

#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

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
        ImGui::Begin("Inputs ",&ins.xclose);
        if (!ins.xclose)
            glfwSetWindowShouldClose(pointer, true);

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
                    ImGui::InputDouble("", &ins.semiaxes1[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : b1 semiaxis
            ImGui::Text("b1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(1);
                    ImGui::InputDouble("", &ins.semiaxes1[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : c1 semiaxis
            ImGui::Text("c1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(2);
                    ImGui::InputDouble("", &ins.semiaxes1[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            //keyboard input : a2 semiaxis
            ImGui::Text("a2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(3);
                    ImGui::InputDouble("", &ins.semiaxes2[0], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : b2 semiaxis
            ImGui::Text("b2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(4);
                    ImGui::InputDouble("", &ins.semiaxes2[1], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : c2 semiaxis
            ImGui::Text("c2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(5);
                    ImGui::InputDouble("", &ins.semiaxes2[2], 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
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
            ImGui::SetNextWindowSize(ImVec2(300,300),ImGuiCond_FirstUseEver); 
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
                            ImGui::InputInt("", &ins.grid_reso_null[0]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::Text(" [ > 1 ]");

                    ImGui::Text("y axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(7);
                            ImGui::InputInt("", &ins.grid_reso_null[1]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::Text(" [ > 1 ]");

                    ImGui::Text("z axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(8);
                            ImGui::InputInt("", &ins.grid_reso_null[2]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::Text(" [ > 1 ]");
                ImGui::EndDisabled();
            }
            else if (ins.obj_refer_to_body == 1 && ins.clicked_poly1)
            {
                //mouse or keyboard input : body1 x grid resolution for the raycast
                ImGui::Text("x axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(9);
                        ImGui::InputInt("", &ins.grid_reso1[0]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body1 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(10);
                        ImGui::InputInt("", &ins.grid_reso1[1]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body1 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(11);
                        ImGui::InputInt("", &ins.grid_reso1[2]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");
            }
            else if (ins.obj_refer_to_body == 2 && ins.clicked_poly2)
            {
                //mouse or keyboard input : body2 x grid resolution for the raycast
                ImGui::Text("x axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(12);
                        ImGui::InputInt("", &ins.grid_reso2[0]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body2 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(13);
                        ImGui::InputInt("", &ins.grid_reso2[1]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body2 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(14);
                        ImGui::InputInt("", &ins.grid_reso2[2]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");
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
        ImGui::End();
    }
};

#endif
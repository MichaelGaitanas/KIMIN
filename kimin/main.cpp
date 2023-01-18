#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>
#include<filesystem>

#include"../include/typedef.hpp"
#include"../include/directory.hpp"
#include"../include/logger.hpp"
#include"../include/inputs.hpp"

void raw_hardware_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    return;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0,width,height);
    return;
}

int main()
{
    //glfw initialization and OpenGL window creation
    //////////////////////////////////////////////////////////////////////////////////////
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_REFRESH_RATE, 60);
    GLFWwindow *window = glfwCreateWindow(800,600, "KIMIN", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    //////////////////////////////////////////////////////////////////////////////////////

    //imgui initialization and basic setup
    //////////////////////////////////////////////////////////////////////////////////////
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../font/arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    //////////////////////////////////////////////////////////////////////////////////////

    std::vector<std::filesystem::path> path_to_masc_obj = lsobj("../resources/obj/mascons/");
    std::vector<std::filesystem::path> path_to_poly_obj = lsobj("../resources/obj/polyhedra/");

    inputs ins;
    logger lg;

    //game loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //remember to throw this outside the game loop
		glClear(GL_COLOR_BUFFER_BIT);

        //setup imgui 'root' frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const float win_width  = io.DisplaySize.x;
        const float win_height = io.DisplaySize.y;
 
        //create the imgui window at the top left of the glfw window
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width/7.0f, win_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inputs ",&ins.xclose);
        if (!ins.xclose)
        {
            glfwSetWindowShouldClose(window, true);
        }

        //keyboard input : simulation name
        ImGui::Text("Simulation name");
        ImGui::PushItemWidth(200); //field box width (height is auto)
            ImGui::InputText(" ", ins.simname, IM_ARRAYSIZE(ins.simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        //mouse input : ellipsoid shape model checkbox
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
                    ImGui::InputDouble("", &ins.a1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : b1 semiaxis
            ImGui::Text("b1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(1);
                    ImGui::InputDouble("", &ins.b1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : c1 semiaxis
            ImGui::Text("c1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(2);
                    ImGui::InputDouble("", &ins.c1, 0.0, 0.0,"%.5lf");
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
                    ImGui::InputDouble("", &ins.a2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : b2 semiaxis
            ImGui::Text("b2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(4);
                    ImGui::InputDouble("", &ins.b2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            //keyboard input : c2 semiaxis
            ImGui::Text("c2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(5);
                    ImGui::InputDouble("", &ins.c2, 0.0, 0.0,"%.5lf");
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
                for (int i = 0; i < path_to_masc_obj.size(); ++i)
                {
                    if (ins.obj_refer_to_body == 1)
                    {
                        //mouse input : which .obj model path for body1
                        if (ImGui::Selectable(path_to_masc_obj[i].string().c_str(), (ins.clicked_masc1_index == i)))
                        {
                            ins.clicked_poly1 = false;
                            ins.clicked_masc1_index = i;
                            ins.clicked_poly1_index = -1;
                            ins.obj_path1 = path_to_masc_obj[i].string();
                        }
                    }
                    else
                    {
                        //mouse input : which .obj model path for body2
                        if (ImGui::Selectable(path_to_masc_obj[i].string().c_str(), (ins.clicked_masc2_index == i)))
                        {
                            ins.clicked_poly2 = false;
                            ins.clicked_masc2_index = i;
                            ins.clicked_poly2_index = -1;
                            ins.obj_path2 = path_to_masc_obj[i].string();
                        }
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Polyhedral models"))
            {
                for (int i = 0; i < path_to_poly_obj.size(); ++i)
                {
                    //mouse input : which .obj model path for body1
                    if (ins.obj_refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_to_poly_obj[i].string().c_str(), (ins.clicked_poly1_index == i)))
                        {
                            ins.clicked_poly1 = true;
                            ins.clicked_masc1_index = -1;
                            ins.clicked_poly1_index = i;
                            ins.obj_path1 = path_to_poly_obj[i].string();
                        }
                    }
                    else
                    {
                        //mouse input : which .obj model path for body2
                        if (ImGui::Selectable(path_to_poly_obj[i].string().c_str(), (ins.clicked_poly2_index == i)))
                        {
                            ins.clicked_poly2 = true;
                            ins.clicked_masc2_index = -1;
                            ins.clicked_poly2_index = i;
                            ins.obj_path2 = path_to_poly_obj[i].string();
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
                            ImGui::InputInt("", &ins.grid_reso_x_null);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::Text(" [ > 1 ]");

                    ImGui::Text("y axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(7);
                            ImGui::InputInt("", &ins.grid_reso_y_null);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::Text(" [ > 1 ]");

                    ImGui::Text("z axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(8);
                            ImGui::InputInt("", &ins.grid_reso_z_null);
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
                        ImGui::InputInt("", &ins.grid_reso_x1);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body1 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(10);
                        ImGui::InputInt("", &ins.grid_reso_y1);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body1 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(11);
                        ImGui::InputInt("", &ins.grid_reso_z1);
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
                        ImGui::InputInt("", &ins.grid_reso_x2);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body2 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(13);
                        ImGui::InputInt("", &ins.grid_reso_y2);
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text(" [ > 1 ]");

                //mouse or keyboard input : body2 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(14);
                        ImGui::InputInt("", &ins.grid_reso_z2);
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
                ImGui::InputDouble("", &ins.M1, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");
        
        //keyboard input : M2
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(16);
                ImGui::InputDouble("", &ins.M2, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Integration time");

        //keyboard input : Epoch
        ImGui::Text("Epoch     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(17);
                ImGui::InputDouble("", &ins.epoch, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");

        //keyboard input : Duration
        ImGui::Text("Duration  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(18);
                ImGui::InputDouble("", &ins.dur, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");
        
        //keyboard input : Step
        ImGui::Text("Step        ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(19);
                ImGui::InputDouble("", &ins.step, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::Dummy(ImVec2(0.0f,20.0f));

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
                    ImGui::InputDouble("", &ins.relx, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            
            //keyboard input : relative position y
            ImGui::Text("y    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(22);
                    ImGui::InputDouble("", &ins.rely, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            //keyboard input : relative position z
            ImGui::Text("z    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(23);
                    ImGui::InputDouble("", &ins.relz, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            //keyboard input : relative velocity vx
            ImGui::Text("vx  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(24);
                    ImGui::InputDouble("", &ins.relvx, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km/sec]");
            
            //keyboard input : relative velocity vy
            ImGui::Text("vy  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(25);
                    ImGui::InputDouble("", &ins.relvy, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km/sec]");

            //keyboard input : relative velocity vz
            ImGui::Text("vz  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(26);
                    ImGui::InputDouble("", &ins.relvz, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km/sec]");
        }
        else if (ins.cart_kep_var_choice == 1)
        {
            //keyboard input : relative semi-major axis a
            ImGui::Text("a       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(27);
                    ImGui::InputDouble("", &ins.rela, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");
            
            //keyboard input : relative eccentricity e
            ImGui::Text("e       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(28);
                    ImGui::InputDouble("", &ins.rele, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : relative inclination i
            ImGui::Text("i        ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(29);
                    ImGui::InputDouble("", &ins.reli, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            //keyboard input : relative longitude of ascending node raan
            ImGui::Text("raan  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(30);
                    ImGui::InputDouble("", &ins.relraan, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            //keyboard input : relative argument of periapsis w
            ImGui::Text("w       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(31);
                    ImGui::InputDouble("", &ins.relw, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            //keyboard input : relative mean anomaly M
            ImGui::Text("M      ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(32);
                    ImGui::InputDouble("", &ins.relM, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
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
                    ImGui::InputDouble("", &ins.roll1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            //keyboard input : pitch 1 angle
            ImGui::Text("pitch 1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0);
                ImGui::PushID(35);
                    ImGui::InputDouble("", &ins.pitch1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            //keyboard input : yaw 1 angle
            ImGui::Text("yaw 1  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(36);
                    ImGui::InputDouble("", &ins.yaw1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            //keyboard input : roll 2 angle
            ImGui::Text("roll 2    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(37);
                    ImGui::InputDouble("", &ins.roll2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            //keyboard input : pitch 2 angle
            ImGui::Text("pitch 2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(38);
                    ImGui::InputDouble("", &ins.pitch2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            //keyboard input : yaw 2 angle
            ImGui::Text("yaw 2  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(39);
                    ImGui::InputDouble("", &ins.yaw2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
        }
        else if (ins.orient_var_choice == 1)
        {
            //keyboard input : q10 quaternion component
            ImGui::Text("q10   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(40);
                    ImGui::InputDouble("", &ins.q10, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : q11 quaternion component
            ImGui::Text("q11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(41);
                    ImGui::InputDouble("", &ins.q11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : q12 quaternion component
            ImGui::Text("q12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(42);
                    ImGui::InputDouble("", &ins.q12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : q13 quaternion component
            ImGui::Text("q13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(43);
                    ImGui::InputDouble("", &ins.q13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : q20 quaternion component
            ImGui::Text("q20   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(44);
                    ImGui::InputDouble("", &ins.q20, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : q21 quaternion component
            ImGui::Text("q21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(45);
                    ImGui::InputDouble("", &ins.q21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //keyboard input : q22 quaternion component
            ImGui::Text("q22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(46);
                    ImGui::InputDouble("", &ins.q22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            //field : q23 quaternion component
            ImGui::Text("q23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(47);
                    ImGui::InputDouble("", &ins.q23, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");
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
                    ImGui::InputDouble("", &ins.w1x, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w1y (inertial) angular velocity
            ImGui::Text("w1y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(50);
                    ImGui::InputDouble("", &ins.w1y, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            
            //keyboard input : w1z (inertial) angular velocity
            ImGui::Text("w1z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(51);
                    ImGui::InputDouble("", &ins.w1z, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w2x (inertial) angular velocity
            ImGui::Text("w2x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(52);
                    ImGui::InputDouble("", &ins.w2x, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w2y (inertial) angular velocity
            ImGui::Text("w2y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(53);
                    ImGui::InputDouble("", &ins.w2y, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            
            //keyboard input : w2z (inertial) angular velocity
            ImGui::Text("w2z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(54);
                    ImGui::InputDouble("", &ins.w2z, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

        }
        else if (ins.frame_type_choice == 1)
        {
            //keyboard input : w11 (body) angular velocity
            ImGui::Text("w11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(55);
                    ImGui::InputDouble("", &ins.w11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w12 (body) angular velocity
            ImGui::Text("w12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(56);
                    ImGui::InputDouble("", &ins.w12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w13 (body) angular velocity
            ImGui::Text("w13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(57);
                    ImGui::InputDouble("", &ins.w13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w21 (body) angular velocity
            ImGui::Text("w21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(58);
                    ImGui::InputDouble("", &ins.w21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w22 (body) angular velocity
            ImGui::Text("w22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(59);
                    ImGui::InputDouble("", &ins.w22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            //keyboard input : w23 (body) angular velocity
            ImGui::Text("w23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(60);
                    ImGui::InputDouble("", &ins.w23, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
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

        //mouse input : whether to assume a collision detection criterion or not
        ImGui::Checkbox("Detect binary collision", &ins.detect_binary_collision);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Files to export");

        //mouse input : which files to export after the simulation
        ImGui::Checkbox("Time", &ins.export_time);
        ImGui::Checkbox("Relative position/velocity", &ins.export_rel_pos_vel);
        ImGui::Checkbox("Euler angles", &ins.export_ang);
        ImGui::Checkbox("Quaternions", &ins.export_quat);
        ImGui::Checkbox("Angular velocities (inertial frame)", &ins.export_wi);
        ImGui::Checkbox("Angular velocities (body frames)", &ins.export_wb);
        ImGui::Checkbox("Energy & momentum", &ins.export_ener_mom);
        ImGui::Checkbox("Relative Keplerian elements", &ins.export_rel_kep);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        //mouse input : whether to render OpenGL video or not
        ImGui::Checkbox("Playback video", &ins.playback_video);
        ImGui::Dummy(ImVec2(0.0f,20.0f));

        //mouse input : run button
        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            ins.clicked_run = true;

        ImGui::End();

        if (ins.clicked_run)
        {
            strvec errors = ins.validate();
            if (!errors.size())
            {
                lg.add("Running physics ...  "); lg.add("\n");
                ///outputs outs = ins.propagate();
            }
            else
            {
                for (int i = 0; i < errors.size(); ++i)
                {
                    lg.add(errors[i].c_str()); lg.add("\n");
                }
                lg.add("\n");
            }
            
        }
        lg.draw("Log ", NULL, mode->width, mode->height, (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));

        ins.clicked_run = false;

        ImGui::SetNextWindowPos(ImVec2(win_width - win_width/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width/7.0f, win_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graphics", NULL);
        ImGui::Text("Content...");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        raw_hardware_input(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}
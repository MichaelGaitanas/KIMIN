#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<filesystem>

#include"../include/directory.hpp"
#include"../include/log.hpp"
#include"../include/inputs.hpp"

//remember to store these functions somewehre...
//////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////

int main()
{
    //glfw initialization and OpenGL window creation
    //////////////////////////////////////////////////////////////////////////////////////
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    GLFWwindow *window = glfwCreateWindow(800,600, "KIMIN", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }
    glfwMakeContextCurrent(window);
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

    std::string finpath1, finpath2;
    std::vector<std::filesystem::path> path_v = lsobj("../resources/obj/v/");
    std::vector<std::filesystem::path> path_vf = lsobj("../resources/obj/vf/");
    std::vector<std::filesystem::path> path_vfn = lsobj("../resources/obj/vfn/");

    inputs ins;
    log lg;

    //game loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //remember to throw this outside the game loop
		glClear(GL_COLOR_BUFFER_BIT);

        //setup imgui 'root' frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
 
        //create the imgui window at the top left of the glfw window
        ImGui::SetNextWindowPos(ImVec2(0.0f,0.0f), ImGuiCond_FirstUseEver); //extremely careful with the FirstUseEver flag 
        ImGui::Begin("Controls (under development)");

        /* field : simulation name */
        ImGui::Text("Simulation name");
        
        ImGui::PushItemWidth(200); //field box width (height is auto)
            ImGui::InputText(" ", ins.simname, IM_ARRAYSIZE(ins.simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        /* field : ellipsoid shape model */
        ImGui::Text("Shape model");
        if (ImGui::Checkbox("Ellipsoids", &ins.ell_checkbox) && ins.ell_checkbox)
        {
            ins.pressed_ell_ok = false;
        }
        if (ins.ell_checkbox && !ins.pressed_ell_ok)
        {
            ins.obj_checkbox = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y)); 
            ImGui::Begin("  ");
                
            //ellipsoid semiaxes menu

            ImGui::Text("Semiaxes");

            /* field : a1 semiaxis */
            ImGui::Text("a1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(0);
                    ImGui::InputDouble("", &ins.a1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : b1 semiaxis */
            ImGui::Text("b1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(1);
                    ImGui::InputDouble("", &ins.b1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : c1 semiaxis */
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

            /* field : a2 semiaxis */
            ImGui::Text("a2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(3);
                    ImGui::InputDouble("", &ins.a2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : b2 semiaxis */
            ImGui::Text("b2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(4);
                    ImGui::InputDouble("", &ins.b2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : c2 semiaxis */
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

            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
            {
                ins.pressed_ell_ok = true;
            }
            ImGui::End();
        }

        /* field : .obj shape model */
        if (ImGui::Checkbox(".obj files", &ins.obj_checkbox) && ins.obj_checkbox)
        {
            ins.pressed_obj_ok = false;
        }
        if (ins.obj_checkbox && !ins.pressed_obj_ok)
        {
            ins.ell_checkbox = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y)); //display position of the obj files menu 
            ImGui::Begin("  ");
            ImGui::Text("KIMIN's .obj database\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            if (ImGui::RadioButton("Body 1", ins.refer_to_body == 1))
                ins.refer_to_body = 1;
            if (ImGui::RadioButton("Body 2", ins.refer_to_body == 2))
                ins.refer_to_body = 2;

            
            if (ImGui::TreeNodeEx("v"))
            {
                for (int i = 0; i < path_v.size(); ++i)
                {
                    if (ins.refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_v[i].string().c_str(), (ins.current_v1 == i)))
                            {ins.current_v1 = i; ins.current_vf1 = -1; ins.current_vfn1 = -1; finpath1 = path_v[i].string();}
                    }
                    else
                    {
                        if (ImGui::Selectable(path_v[i].string().c_str(), (ins.current_v2 == i)))
                            {ins.current_v2 = i; ins.current_vf2 = -1; ins.current_vfn2 = -1; finpath2 = path_v[i].string();}
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("vf"))
            {
                for (int i = 0; i < path_vf.size(); ++i)
                {
                    if (ins.refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_vf[i].string().c_str(), (ins.current_vf1 == i)))
                            {ins.current_v1 = -1; ins.current_vf1 = i; ins.current_vfn1 = -1; finpath1 = path_vf[i].string();}
                    }
                    else
                    {
                        if (ImGui::Selectable(path_vf[i].string().c_str(), (ins.current_vf2 == i)))
                            {ins.current_v2 = -1; ins.current_vf2 = i; ins.current_vfn2 = -1; finpath2 = path_vf[i].string();}
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("vfn"))
            {
                for (int i = 0; i < path_vfn.size(); ++i)
                {
                    if (ins.refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_vfn[i].string().c_str(), (ins.current_vfn1 == i)))
                            {ins.current_v1 = -1; ins.current_vf1 = -1; ins.current_vfn1 = i; finpath1 = path_vfn[i].string();}
                    }
                    else
                    {
                        if (ImGui::Selectable(path_vfn[i].string().c_str(), (ins.current_vfn2 == i)))
                            {ins.current_v2 = -1; ins.current_vf2 = -1; ins.current_vfn2 = i; finpath2 = path_vfn[i].string();}
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))   
            {
                ins.pressed_obj_ok = true;
            }

            ImGui::End();
        }

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Mass");

        /* field : M1 */
        ImGui::Text("M1 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(6);
                ImGui::InputDouble("", &ins.M1, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");
        
        /* field : M2 */
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(7);
                ImGui::InputDouble("", &ins.M2, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Integration time");

        /* field : t0 */
        ImGui::Text("t0              ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(8);
                ImGui::InputDouble("", &ins.t0, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");

        /* field : tmax */
        ImGui::Text("tmax         ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(9);
                ImGui::InputDouble("", &ins.tmax, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");
        
        /* field : print step */
        ImGui::Text("Print step ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(10);
                ImGui::InputDouble("", &ins.print_step, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");

        ImGui::Dummy(ImVec2(0.0f,20.0f));

        ImGui::Text("Initial state");
        ImGui::Indent();

        ImGui::Text("Relative position");

        /* field : relative position x */
        ImGui::Text("x   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(11);
                ImGui::InputDouble("", &ins.relx, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");
        
        /* field : relative position y */
        ImGui::Text("y   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(12);
                ImGui::InputDouble("", &ins.rely, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");

        /* field : relative position z */
        ImGui::Text("z   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(13);
                ImGui::InputDouble("", &ins.relz, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");

        ImGui::Text("Relative velocity");

        /* field : relative velocity x */
        ImGui::Text("vx  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(14);
                ImGui::InputDouble("", &ins.relvx, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        
        /* field : relative velocity y */
        ImGui::Text("vy  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(15);
                ImGui::InputDouble("", &ins.relvy, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");

        /* field : relative velocity z */
        ImGui::Text("vz  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(16);
                ImGui::InputDouble("", &ins.relvz, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");

        ImGui::Text("Orientations");

        /* field : orientation variables (combo) */
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(17);
                ImGui::Combo("  ", &ins.orient_var_choice, ins.orient_var, IM_ARRAYSIZE(ins.orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (ins.orient_var_choice == 0)
        {
            /* field : roll 1 angle */
            ImGui::Text("roll 1    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(18);
                    ImGui::InputDouble("", &ins.roll1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : pitch 1 angle */
            ImGui::Text("pitch 1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0);
                ImGui::PushID(19);
                    ImGui::InputDouble("", &ins.pitch1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            /* field : yaw 1 angle */
            ImGui::Text("yaw 1  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(20);
                    ImGui::InputDouble("", &ins.yaw1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : roll 2 angle */
            ImGui::Text("roll 2    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(21);
                    ImGui::InputDouble("", &ins.roll2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : pitch 2 angle */
            ImGui::Text("pitch 2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(22);
                    ImGui::InputDouble("", &ins.pitch2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            /* field : yaw 2 angle */
            ImGui::Text("yaw 2  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(23);
                    ImGui::InputDouble("", &ins.yaw2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
        }
        else if (ins.orient_var_choice == 1)
        {
            /* field : q10 quaternion component */
            ImGui::Text("q10   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(24);
                    ImGui::InputDouble("", &ins.q10, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q11 quaternion component */
            ImGui::Text("q11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(25);
                    ImGui::InputDouble("", &ins.q11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q12 quaternion component */
            ImGui::Text("q12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(26);
                    ImGui::InputDouble("", &ins.q12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q13 quaternion component */
            ImGui::Text("q13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(27);
                    ImGui::InputDouble("", &ins.q13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q20 quaternion component */
            ImGui::Text("q20   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(28);
                    ImGui::InputDouble("", &ins.q20, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q21 quaternion component */
            ImGui::Text("q21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(29);
                    ImGui::InputDouble("", &ins.q21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q22 quaternion component */
            ImGui::Text("q22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(30);
                    ImGui::InputDouble("", &ins.q22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q23 quaternion component */
            ImGui::Text("q23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(31);
                    ImGui::InputDouble("", &ins.q23, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");
        }

        ImGui::Text("Angular velocity");

        /* field : frame choice (combo) */
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(32);
                ImGui::Combo("  ", &ins.frame_type_choice, ins.frame_type, IM_ARRAYSIZE(ins.frame_type));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (ins.frame_type_choice == 0)
        {
            /* field : w1x (inertial) angular velocity */
            ImGui::Text("w1x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(33);
                    ImGui::InputDouble("", &ins.w1x, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w1y (inertial) angular velocity */
            ImGui::Text("w1y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(34);
                    ImGui::InputDouble("", &ins.w1y, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            
            /* field : w1z (inertial) angular velocity */
            ImGui::Text("w1z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(35);
                    ImGui::InputDouble("", &ins.w1z, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");



            /* field : w2x (inertial) angular velocity */
            ImGui::Text("w2x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(36);
                    ImGui::InputDouble("", &ins.w2x, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w2y (inertial) angular velocity */
            ImGui::Text("w2y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(37);
                    ImGui::InputDouble("", &ins.w2y, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            
            /* field : w2z (inertial) angular velocity */
            ImGui::Text("w2z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(38);
                    ImGui::InputDouble("", &ins.w2z, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

        }
        else if (ins.frame_type_choice == 1)
        {
            /* field : w11 (body) angular velocity */
            ImGui::Text("w11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(39);
                    ImGui::InputDouble("", &ins.w11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w12 (body) angular velocity */
            ImGui::Text("w12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(40);
                    ImGui::InputDouble("", &ins.w12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w13 (body) angular velocity */
            ImGui::Text("w13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(41);
                    ImGui::InputDouble("", &ins.w13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");





            /* field : w21 (body) angular velocity */
            ImGui::Text("w21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(42);
                    ImGui::InputDouble("", &ins.w21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w22 (body) angular velocity */
            ImGui::Text("w22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(43);
                    ImGui::InputDouble("", &ins.w22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w23 (body) angular velocity */
            ImGui::Text("w23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(44);
                    ImGui::InputDouble("", &ins.w23, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
        }

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Unindent();

        ImGui::Text("Integrator");

        /* field : integration method */
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(45);
                ImGui::Combo("  ", &ins.integ_method_choice, ins.integ_method, IM_ARRAYSIZE(ins.integ_method));
            ImGui::PopID();
        ImGui::PopItemWidth();

        /* field : integration method tolerance */
        ImGui::Text("tolerance ");
        ImGui::SameLine();
        ImGui::PushItemWidth(200.0);
            ImGui::PushID(46);
                ImGui::InputDouble("", &ins.tol, 0.0, 0.0,"%e");
            ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        /* field : whether to assume a collision detection criterion or not */
        ImGui::Checkbox("Detect binary collision", &ins.detect_binary_collision);

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Files to export");

        /* fields : which files to export after the simulation */
        ImGui::Checkbox("Time", &ins.export_time);
        ImGui::Checkbox("Relative position/velocity", &ins.export_rel_pos_vel);
        ImGui::Checkbox("Euler angles", &ins.export_ang);
        ImGui::Checkbox("Quaternions", &ins.export_quat);
        ImGui::Checkbox("Angular velocities (inertial frame)", &ins.export_wi);
        ImGui::Checkbox("Angular velocities (body frames)", &ins.export_wb);
        ImGui::Checkbox("Energy & momentum", &ins.export_ener_mom);
        ImGui::Checkbox("Relative Keplerian elements", &ins.export_rel_kep);

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        /* field : whether to render OpenGL playback video or not */
        ImGui::Checkbox("Playback video", &ins.playback_video);

        ImGui::Dummy(ImVec2(0.0f,20.0f));

        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            ins.pressed_run = true;

        ImGui::End();

        if (ins.pressed_run)
        {
            lg.add("Supposed to run..."); lg.add("\n");
        }
        lg.draw("Log (under development)", NULL, mode->width, mode->height);

        ins.pressed_run = false;

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
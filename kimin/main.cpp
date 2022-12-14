#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<filesystem>

#include"../include/directory.hpp"
#include"../include/log.hpp"

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

    bool ell_checkbox = false;
    bool pressed_ell_ok = false;
    bool obj_checkbox = false;
    bool pressed_obj_ok = false;
    bool detect_binary_collision = true;
    bool playback_video = false;
    bool pressed_run = false;

    //20 characters (+ '\0') available for the simulation name directory (don't forget to write name validity checks)
    static char simname[21] = "";

    static double a1 = 0.0;
    static double b1 = 0.0;
    static double c1 = 0.0;
    static double a2 = 0.0;
    static double b2 = 0.0;
    static double c2 = 0.0;

    static int refer_to_body = 1; //initial body choice for the .obj file

    static int current_v1 = -1;
    static int current_vf1 = -1;
    static int current_vfn1 = -1;

    static int current_v2 = -1;
    static int current_vf2 = -1;
    static int current_vfn2 = -1;

    std::vector<std::filesystem::path> path_v = lsobj("../resources/obj/v/");
    std::vector<std::filesystem::path> path_vf = lsobj("../resources/obj/vf/");
    std::vector<std::filesystem::path> path_vfn = lsobj("../resources/obj/vfn/");

    static double M1 = 0.0;
    static double M2 = 0.0;

    static double t0 = 0.0;
    static double tmax = 0.0;
    static double print_step = 0.0;

    static double relx = 0.0;
    static double rely = 0.0;
    static double relz = 0.0;

    static double relvx = 0.0;
    static double relvy = 0.0;
    static double relvz = 0.0;

    static const char *orient_var[] = {"Euler angles ", "Quaternion "};
    static int orient_var_choice = 0;

    static double roll1 = 0.0;
    static double pitch1 = 0.0;
    static double yaw1 = 0.0;

    static double roll2 = 0.0;
    static double pitch2 = 0.0;
    static double yaw2 = 0.0;

    static double q10 = 1.0;
    static double q11 = 0.0;
    static double q12 = 0.0;
    static double q13 = 0.0;

    static double q20 = 1.0;
    static double q21 = 0.0;
    static double q22 = 0.0;
    static double q23 = 0.0;

    static const char *frame_type[] = {"Inertial frame", "Body frame"};
    static int frame_type_choice = 0;

    static double w1x = 0.0;
    static double w1y = 0.0;
    static double w1z = 0.0;

    static double w2x = 0.0;
    static double w2y = 0.0;
    static double w2z = 0.0;

    static double w11 = 0.0;
    static double w12 = 0.0;
    static double w13 = 0.0;

    static double w21 = 0.0;
    static double w22 = 0.0;
    static double w23 = 0.0;

    static const char *integ_method[] = {"Runge-Kutta-Fehlberg 78 ", "Bulirsch-Stoer", "Dormand-Prince 5 ", "Runge-Kutta 4 (explicit)"};
    static int integ_method_choice = 0;
    static double tol = 1e-9;


    static bool export_time = true;
    static bool export_rel_pos_vel = true;
    static bool export_ang = true;
    static bool export_quat = true;
    static bool export_wi = true;
    static bool export_wb = true;
    static bool export_ener_mom = true;
    static bool export_kep = true;

    static log kimin_log;


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
            ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        /* field : ellipsoid shape model */
        ImGui::Text("Shape model");
        if (ImGui::Checkbox("Ellipsoids", &ell_checkbox) && ell_checkbox)
        {
            pressed_ell_ok = false;
        }
        if (ell_checkbox && !pressed_ell_ok)
        {
            obj_checkbox = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y)); 
            ImGui::Begin("  ");
                
            //ellipsoid semiaxes menu

            ImGui::Text("Semiaxes");

            /* field : a1 semiaxis */
            ImGui::Text("a1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(0);
                    ImGui::InputDouble("", &a1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : b1 semiaxis */
            ImGui::Text("b1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(1);
                    ImGui::InputDouble("", &b1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : c1 semiaxis */
            ImGui::Text("c1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(2);
                    ImGui::InputDouble("", &c1, 0.0, 0.0,"%.5lf");
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
                    ImGui::InputDouble("", &a2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : b2 semiaxis */
            ImGui::Text("b2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(4);
                    ImGui::InputDouble("", &b2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            /* field : c2 semiaxis */
            ImGui::Text("c2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(5);
                    ImGui::InputDouble("", &c2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[km]");

            ImGui::Dummy(ImVec2(0.0f,15.0f));

            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
            {
                pressed_ell_ok = true;
            }
            ImGui::End();
        }

        /* field : .obj shape model */
        if (ImGui::Checkbox(".obj files", &obj_checkbox) && obj_checkbox)
        {
            pressed_obj_ok = false;
        }
        if (obj_checkbox && !pressed_obj_ok)
        {
            ell_checkbox = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y)); //display position of the obj files menu 
            ImGui::Begin("  ");
            ImGui::Text("KIMIN's .obj database\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            if (ImGui::RadioButton("Body 1", refer_to_body == 1))
                refer_to_body = 1;
            if (ImGui::RadioButton("Body 2", refer_to_body == 2))
                refer_to_body = 2;

            
            if (ImGui::TreeNodeEx("v"))
            {
                for (int i = 0; i < path_v.size(); ++i)
                {
                    if (refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_v[i].string().c_str(), (current_v1 == i)))
                            {current_v1 = i; current_vf1 = -1; current_vfn1 = -1; }
                    }
                    else
                    {
                        if (ImGui::Selectable(path_v[i].string().c_str(), (current_v2 == i)))
                            {current_v2 = i; current_vf2 = -1; current_vfn2 = -1; }
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("vf"))
            {
                for (int i = 0; i < path_vf.size(); ++i)
                {
                    if (refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_vf[i].string().c_str(), (current_vf1 == i)))
                            {current_v1 = -1; current_vf1 = i; current_vfn1 = -1; }
                    }
                    else
                    {
                        if (ImGui::Selectable(path_vf[i].string().c_str(), (current_vf2 == i)))
                            {current_v2 = -1; current_vf2 = i; current_vfn2 = -1; }
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("vfn"))
            {
                for (int i = 0; i < path_vfn.size(); ++i)
                {
                    if (refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_vfn[i].string().c_str(), (current_vfn1 == i)))
                            {current_v1 = -1; current_vf1 = -1; current_vfn1 = i; }
                    }
                    else
                    {
                        if (ImGui::Selectable(path_vfn[i].string().c_str(), (current_vfn2 == i)))
                            {current_v2 = -1; current_vf2 = -1; current_vfn2 = i; }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))   
            {
                pressed_obj_ok = true;
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
                ImGui::InputDouble("", &M1, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");
        
        /* field : M2 */
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(7);
                ImGui::InputDouble("", &M2, 0.0, 0.0,"%.5lf");
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
                ImGui::InputDouble("", &t0, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");

        /* field : tmax */
        ImGui::Text("tmax         ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(9);
                ImGui::InputDouble("", &tmax, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");
        
        /* field : print step */
        ImGui::Text("Print step ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(10);
                ImGui::InputDouble("", &print_step, 0.0, 0.0,"%.5lf");
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
                ImGui::InputDouble("", &relx, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");
        
        /* field : relative position y */
        ImGui::Text("y   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(12);
                ImGui::InputDouble("", &rely, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");

        /* field : relative position z */
        ImGui::Text("z   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(13);
                ImGui::InputDouble("", &relz, 0.0, 0.0,"%.5lf");
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
                ImGui::InputDouble("", &relvx, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        
        /* field : relative velocity y */
        ImGui::Text("vy  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(15);
                ImGui::InputDouble("", &relvy, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");

        /* field : relative velocity z */
        ImGui::Text("vz  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(16);
                ImGui::InputDouble("", &relvz, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");

        ImGui::Text("Orientations");

        /* field : orientation variables (combo) */
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(17);
                ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (orient_var_choice == 0)
        {
            /* field : roll 1 angle */
            ImGui::Text("roll 1    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(18);
                    ImGui::InputDouble("", &roll1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : pitch 1 angle */
            ImGui::Text("pitch 1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0);
                ImGui::PushID(19);
                    ImGui::InputDouble("", &pitch1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            /* field : yaw 1 angle */
            ImGui::Text("yaw 1  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(20);
                    ImGui::InputDouble("", &yaw1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : roll 2 angle */
            ImGui::Text("roll 2    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(21);
                    ImGui::InputDouble("", &roll2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : pitch 2 angle */
            ImGui::Text("pitch 2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(22);
                    ImGui::InputDouble("", &pitch2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            /* field : yaw 2 angle */
            ImGui::Text("yaw 2  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(23);
                    ImGui::InputDouble("", &yaw2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
        }
        else if (orient_var_choice == 1)
        {
            /* field : q10 quaternion component */
            ImGui::Text("q10   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(24);
                    ImGui::InputDouble("", &q10, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q11 quaternion component */
            ImGui::Text("q11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(25);
                    ImGui::InputDouble("", &q11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q12 quaternion component */
            ImGui::Text("q12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(26);
                    ImGui::InputDouble("", &q12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q13 quaternion component */
            ImGui::Text("q13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(27);
                    ImGui::InputDouble("", &q13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q20 quaternion component */
            ImGui::Text("q20   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(28);
                    ImGui::InputDouble("", &q20, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q21 quaternion component */
            ImGui::Text("q21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(29);
                    ImGui::InputDouble("", &q21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q22 quaternion component */
            ImGui::Text("q22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(30);
                    ImGui::InputDouble("", &q22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q23 quaternion component */
            ImGui::Text("q23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(31);
                    ImGui::InputDouble("", &q23, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");
        }

        ImGui::Text("Angular velocity");

        /* field : frame choice (combo) */
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(32);
                ImGui::Combo("  ", &frame_type_choice, frame_type, IM_ARRAYSIZE(frame_type));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (frame_type_choice == 0)
        {
            /* field : w1x (inertial) angular velocity */
            ImGui::Text("w1x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(33);
                    ImGui::InputDouble("", &w1x, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w1y (inertial) angular velocity */
            ImGui::Text("w1y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(34);
                    ImGui::InputDouble("", &w1y, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            
            /* field : w1z (inertial) angular velocity */
            ImGui::Text("w1z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(35);
                    ImGui::InputDouble("", &w1z, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");



            /* field : w2x (inertial) angular velocity */
            ImGui::Text("w2x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(36);
                    ImGui::InputDouble("", &w2x, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w2y (inertial) angular velocity */
            ImGui::Text("w2y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(37);
                    ImGui::InputDouble("", &w2y, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            
            /* field : w2z (inertial) angular velocity */
            ImGui::Text("w2z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(38);
                    ImGui::InputDouble("", &w2z, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

        }
        else if (frame_type_choice == 1)
        {
            /* field : w11 (body) angular velocity */
            ImGui::Text("w11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(39);
                    ImGui::InputDouble("", &w11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w12 (body) angular velocity */
            ImGui::Text("w12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(40);
                    ImGui::InputDouble("", &w12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w13 (body) angular velocity */
            ImGui::Text("w13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(41);
                    ImGui::InputDouble("", &w13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");





            /* field : w21 (body) angular velocity */
            ImGui::Text("w21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(42);
                    ImGui::InputDouble("", &w21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w22 (body) angular velocity */
            ImGui::Text("w22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(43);
                    ImGui::InputDouble("", &w22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");

            /* field : w23 (body) angular velocity */
            ImGui::Text("w23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(44);
                    ImGui::InputDouble("", &w23, 0.0, 0.0,"%.5lf");
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
                ImGui::Combo("  ", &integ_method_choice, integ_method, IM_ARRAYSIZE(integ_method));
            ImGui::PopID();
        ImGui::PopItemWidth();

        /* field : integration method tolerance */
        ImGui::Text("tolerance ");
        ImGui::SameLine();
        ImGui::PushItemWidth(200.0);
            ImGui::PushID(46);
                ImGui::InputDouble("", &tol, 0.0, 0.0,"%e");
            ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        /* field : whether to assume a collision detection criterion or not */
        ImGui::Checkbox("Detect binary collision", &detect_binary_collision);

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Files to export");

        /* fields : which files to export after the simulation */
        ImGui::Checkbox("Time", &export_time);
        ImGui::Checkbox("Relative position/velocity", &export_rel_pos_vel);
        ImGui::Checkbox("Euler angles", &export_ang);
        ImGui::Checkbox("Quaternions", &export_quat);
        ImGui::Checkbox("Angular velocities (inertial-frame)", &export_wi);
        ImGui::Checkbox("Angular velocities (body-frames)", &export_wb);
        ImGui::Checkbox("Energy & momentum", &export_ener_mom);
        ImGui::Checkbox("Relative Keplerian elements", &export_kep);

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        /* field : whether to render OpenGL playback video or not */
        ImGui::Checkbox("Playback video", &playback_video);

        ImGui::Dummy(ImVec2(0.0f,20.0f));

        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            pressed_run = true;

        ImGui::End();

        /*
        if (pressed_run)
        {
            kimin_log.add("Current inputs : \n\n");
            kimin_log.add("Simulation name = ["); kimin_log.add(simname); kimin_log.add("]\n");
            kimin_log.add("Ellipsoid menu = ["); kimin_log.add(std::to_string(show_ell_axes_menu).c_str()); kimin_log.add("]\n");
            if (show_ell_axes_menu)
            {
                kimin_log.add("a1 = ["); kimin_log.add(std::to_string(a1).c_str()); kimin_log.add("]\n");
                kimin_log.add("b1 = ["); kimin_log.add(std::to_string(b1).c_str()); kimin_log.add("]\n");
                kimin_log.add("c1 = ["); kimin_log.add(std::to_string(c1).c_str()); kimin_log.add("]\n");

                kimin_log.add("a2 = ["); kimin_log.add(std::to_string(a2).c_str()); kimin_log.add("]\n");
                kimin_log.add("b2 = ["); kimin_log.add(std::to_string(b2).c_str()); kimin_log.add("]\n");
                kimin_log.add("c2 = ["); kimin_log.add(std::to_string(c2).c_str()); kimin_log.add("]\n");
            }
            kimin_log.add(".obj menu = ["); kimin_log.add(std::to_string(show_obj_files_menu).c_str()); kimin_log.add("]\n"); 
            //check which .obj(s)
            
            kimin_log.add("M1 = ["); kimin_log.add(std::to_string(M1).c_str()); kimin_log.add("]\n");
            kimin_log.add("M2 = ["); kimin_log.add(std::to_string(M2).c_str()); kimin_log.add("]\n");
            
            kimin_log.add("t0 = ["); kimin_log.add(std::to_string(t0).c_str()); kimin_log.add("]\n");
            kimin_log.add("tmax = ["); kimin_log.add(std::to_string(tmax).c_str()); kimin_log.add("]\n");
            kimin_log.add("print_step = ["); kimin_log.add(std::to_string(print_step).c_str()); kimin_log.add("]\n");

            kimin_log.add("x = ["); kimin_log.add(std::to_string(relx).c_str()); kimin_log.add("]\n");
            kimin_log.add("y = ["); kimin_log.add(std::to_string(rely).c_str()); kimin_log.add("]\n");
            kimin_log.add("z = ["); kimin_log.add(std::to_string(relz).c_str()); kimin_log.add("]\n");

            kimin_log.add("vx = ["); kimin_log.add(std::to_string(relvx).c_str()); kimin_log.add("]\n");
            kimin_log.add("vy = ["); kimin_log.add(std::to_string(relvy).c_str()); kimin_log.add("]\n");
            kimin_log.add("vz = ["); kimin_log.add(std::to_string(relvz).c_str()); kimin_log.add("]\n");
    
            kimin_log.add("orient_var_choice = ["); kimin_log.add(std::to_string(orient_var_choice).c_str()); kimin_log.add("]\n");
            if (orient_var_choice == 0)
            {
                kimin_log.add("roll 1 = ["); kimin_log.add(std::to_string(roll1).c_str()); kimin_log.add("]\n");
                kimin_log.add("pitch 1 = ["); kimin_log.add(std::to_string(pitch1).c_str()); kimin_log.add("]\n");
                kimin_log.add("yaw 1 = ["); kimin_log.add(std::to_string(yaw1).c_str()); kimin_log.add("]\n");

                kimin_log.add("roll 2 = ["); kimin_log.add(std::to_string(roll2).c_str()); kimin_log.add("]\n");
                kimin_log.add("pitch 2 = ["); kimin_log.add(std::to_string(pitch2).c_str()); kimin_log.add("]\n");
                kimin_log.add("yaw 2 = ["); kimin_log.add(std::to_string(yaw2).c_str()); kimin_log.add("]\n");
            }
            if (orient_var_choice == 1)
            {
                kimin_log.add("q10 = ["); kimin_log.add(std::to_string(q10).c_str()); kimin_log.add("]\n");
                kimin_log.add("q11 = ["); kimin_log.add(std::to_string(q11).c_str()); kimin_log.add("]\n");
                kimin_log.add("q12 = ["); kimin_log.add(std::to_string(q12).c_str()); kimin_log.add("]\n");
                kimin_log.add("q13 = ["); kimin_log.add(std::to_string(q13).c_str()); kimin_log.add("]\n");

                kimin_log.add("q20 = ["); kimin_log.add(std::to_string(q20).c_str()); kimin_log.add("]\n");
                kimin_log.add("q21 = ["); kimin_log.add(std::to_string(q21).c_str()); kimin_log.add("]\n");
                kimin_log.add("q22 = ["); kimin_log.add(std::to_string(q22).c_str()); kimin_log.add("]\n");
                kimin_log.add("q23 = ["); kimin_log.add(std::to_string(q23).c_str()); kimin_log.add("]\n");
            }

            kimin_log.add("frame_type_choice = ["); kimin_log.add(std::to_string(frame_type_choice).c_str()); kimin_log.add("]\n");
            if (frame_type_choice == 0)
            {
                kimin_log.add("w1x = ["); kimin_log.add(std::to_string(w1x).c_str()); kimin_log.add("]\n");
                kimin_log.add("w1y = ["); kimin_log.add(std::to_string(w1y).c_str()); kimin_log.add("]\n");
                kimin_log.add("w1z = ["); kimin_log.add(std::to_string(w1z).c_str()); kimin_log.add("]\n");

                kimin_log.add("w2x = ["); kimin_log.add(std::to_string(w2x).c_str()); kimin_log.add("]\n");
                kimin_log.add("w2y = ["); kimin_log.add(std::to_string(w2y).c_str()); kimin_log.add("]\n");
                kimin_log.add("w2z = ["); kimin_log.add(std::to_string(w2z).c_str()); kimin_log.add("]\n");
            }
            if (frame_type_choice == 1)
            {
                kimin_log.add("w11 = ["); kimin_log.add(std::to_string(w11).c_str()); kimin_log.add("]\n");
                kimin_log.add("w12 = ["); kimin_log.add(std::to_string(w12).c_str()); kimin_log.add("]\n");
                kimin_log.add("w13 = ["); kimin_log.add(std::to_string(w13).c_str()); kimin_log.add("]\n");

                kimin_log.add("w21 = ["); kimin_log.add(std::to_string(w21).c_str()); kimin_log.add("]\n");
                kimin_log.add("w22 = ["); kimin_log.add(std::to_string(w22).c_str()); kimin_log.add("]\n");
                kimin_log.add("w23 = ["); kimin_log.add(std::to_string(w23).c_str()); kimin_log.add("]\n");
            }


            kimin_log.add("integ_method = ["); kimin_log.add(std::to_string(integ_method_choice).c_str()); kimin_log.add("]\n");
            kimin_log.add("tol = ["); kimin_log.add(std::to_string(tol).c_str()); kimin_log.add("]\n");



            kimin_log.add("detect bin coll = ["); kimin_log.add(std::to_string(detect_binary_collision).c_str()); kimin_log.add("]\n");


            kimin_log.add("export time = ["); kimin_log.add(std::to_string(export_time).c_str()); kimin_log.add("]\n");
            kimin_log.add("export pos & vel = ["); kimin_log.add(std::to_string(export_rel_pos_vel).c_str()); kimin_log.add("]\n");
            kimin_log.add("export eul ang = ["); kimin_log.add(std::to_string(export_ang).c_str()); kimin_log.add("]\n");
            kimin_log.add("export quat = ["); kimin_log.add(std::to_string(export_quat).c_str()); kimin_log.add("]\n");
            kimin_log.add("export wi = ["); kimin_log.add(std::to_string(export_wi).c_str()); kimin_log.add("]\n");
            kimin_log.add("export wb = ["); kimin_log.add(std::to_string(export_wb).c_str()); kimin_log.add("]\n");
            kimin_log.add("export ener mom = ["); kimin_log.add(std::to_string(export_ener_mom).c_str()); kimin_log.add("]\n");
            kimin_log.add("export kep = ["); kimin_log.add(std::to_string(export_kep).c_str()); kimin_log.add("]\n");
























            kimin_log.add("\n");


        }
        */
        kimin_log.draw("Log (under development)", NULL, mode->width, mode->height);

        pressed_run = false;

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
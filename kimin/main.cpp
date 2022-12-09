#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>

#include"../include/directory.hpp"

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
    GLFWwindow *window = glfwCreateWindow(800,600, "KIMIN", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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

    //basic gui decision variables (show this, show that, don't show this anymore, etc...)
    //probably we're gonna have a lot of these variables until the end, hence >>> class, map, or sth.
    bool show_ell_axes_menu = false;
    bool show_obj_files_menu = false;
    bool ell_xclose_button_existence = true;
    bool obj_xclose_button_existence = true;
    bool detect_binary_collision = true;

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
        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_FirstUseEver); //extremely carefule with the FirstUseEver flag 
        ImGui::Begin("GUI");

        /* field : simulation name */
        ImGui::Text("Simulation name");
        //20 characters (+ '\0') available for the simulation name directory (don't forget to write name validity checks)
        static char simname[21] = "";
        ImGui::PushItemWidth(200); //field box width (height is auto)
            ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0,15));

        /* field : ellipsoid shape model */
        ImGui::Text("Shape model");
        ImGui::Checkbox("Ellipsoids", &show_ell_axes_menu);
        if (show_ell_axes_menu)
        {
            show_obj_files_menu = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y)); //display position of the ellipsoid semiaxes menu 
            ell_xclose_button_existence = true;
            ImGui::Begin("  ", &ell_xclose_button_existence);
            if (ell_xclose_button_existence == false)
            {
                show_ell_axes_menu = false;
                ImGui::End();
            }
            else
            {
                //ellipsoid semiaxes menu

                ImGui::Text("Semiaxes");

                /* field : a1 semiaxis */
                static double a1 = 0.0;
                ImGui::Text("a1 ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                    ImGui::PushID(0);
                        ImGui::InputDouble("", &a1, 0.0, 0.0,"%.5lf");
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("[km]");

                /* field : b1 semiaxis */
                static double b1 = 0.0;
                ImGui::Text("b1 ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                    ImGui::PushID(1);
                        ImGui::InputDouble("", &b1, 0.0, 0.0,"%.5lf");
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("[km]");

                /* field : c1 semiaxis */
                static double c1 = 0.0;
                ImGui::Text("c1 ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                    ImGui::PushID(2);
                        ImGui::InputDouble("", &c1, 0.0, 0.0,"%.5lf");
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("[km]");

                ImGui::Dummy(ImVec2(0,10));

                /* field : a2 semiaxis */
                static double a2 = 0.0;
                ImGui::Text("a2 ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                    ImGui::PushID(3);
                        ImGui::InputDouble("", &a2, 0.0, 0.0,"%.5lf");
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("[km]");

                /* field : b2 semiaxis */
                static double b2 = 0.0;
                ImGui::Text("b2 ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                    ImGui::PushID(4);
                        ImGui::InputDouble("", &b2, 0.0, 0.0,"%.5lf");
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("[km]");

                /* field : c2 semiaxis */
                static double c2 = 0.0;
                ImGui::Text("c2 ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100);
                    ImGui::PushID(5);
                        ImGui::InputDouble("", &c2, 0.0, 0.0,"%.5lf");
                    ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("[km]");

                ImGui::Dummy(ImVec2(0,15));

                if (ImGui::Button("OK", ImVec2(50,30)))
                    show_ell_axes_menu = false;

                ImGui::End();
            }
        }

        /* field : .obj shape model */
        ImGui::Checkbox(".obj files", &show_obj_files_menu);
        if (show_obj_files_menu)
        {
            show_ell_axes_menu = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y)); //display position of the obj files menu 
            obj_xclose_button_existence = true;
            ImGui::Begin("  ", &obj_xclose_button_existence);
            if (obj_xclose_button_existence == false)
            {
                show_obj_files_menu = false;
                ImGui::End();
            }
            else
            {
                
                ImGui::Text("KIMIN's obj database\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
                ImGui::Dummy(ImVec2(0,15));

                static int refer_to_body = 1; //initial body choice for the .obj file
                if (ImGui::RadioButton("Body 1", refer_to_body == 1))
                    refer_to_body = 1;
                if (ImGui::RadioButton("Body 2", refer_to_body == 2))
                    refer_to_body = 2;

                if (ImGui::TreeNodeEx("v"))
                {
                    std::vector<std::filesystem::path> pv = lsobj("../resources/obj/v");
                    for (int i = 0; i < pv.size(); ++i)
                        ImGui::Selectable(pv[i].string().c_str());
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("vf"))
                {
                    std::vector<std::filesystem::path> pvf = lsobj("../resources/obj/vf");
                    for (int i = 0; i < pvf.size(); ++i)
                        ImGui::Selectable(pvf[i].string().c_str());
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("vfn"))
                {
                    std::vector<std::filesystem::path> pvfn = lsobj("../resources/obj/vfn");
                    for (int i = 0; i < pvfn.size(); ++i)
                        ImGui::Selectable(pvfn[i].string().c_str());
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0, 15));

                if (ImGui::Button("OK", ImVec2(50,30)))
                    show_obj_files_menu = false;

                ImGui::End();
            }
        }

        ImGui::Dummy(ImVec2(0, 15));

        ImGui::Text("Mass");

        /* field : M1 */
        static double M1 = 0.0;
        ImGui::Text("M1 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(6);
                ImGui::InputDouble("", &M1, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");
        
        /* field : M1 */
        static double M2 = 0.0;
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(7);
                ImGui::InputDouble("", &M2, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[kg]");

        ImGui::Dummy(ImVec2(0,15));


        ImGui::Text("Integration time parameters");

        /* field : t0 */
        static double t0 = 0.0;
        ImGui::Text("t0              ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(8);
                ImGui::InputDouble("", &t0, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");

        /* field : tmax */
        static double tmax = 0.0;
        ImGui::Text("tmax         ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(9);
                ImGui::InputDouble("", &tmax, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");
        
        /* field : print step */
        static double print_step = 0.0;
        ImGui::Text("print_step ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(10);
                ImGui::InputDouble("", &print_step, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[days]");

        ImGui::Dummy(ImVec2(0,20));

        ImGui::Text("Initial state");
        ImGui::Indent();

        ImGui::Text("Relative position");

        /* field : relative position x */
        static double relx = 0.0;
        ImGui::Text("x   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(11);
                ImGui::InputDouble("", &relx, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");
        
        /* field : relative position y */
        static double rely = 0.0;
        ImGui::Text("y   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(12);
                ImGui::InputDouble("", &rely, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");

        /* field : relative position z */
        static double relz = 0.0;
        ImGui::Text("z   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(13);
                ImGui::InputDouble("", &relz, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km]");

        ImGui::Text("Relative velocity");


        /* field : relative velocity x */
        static double relvx = 0.0;
        ImGui::Text("vx  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(14);
                ImGui::InputDouble("", &relvx, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        
        /* field : relative velocity y */
        static double relvy = 0.0;
        ImGui::Text("vy  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(15);
                ImGui::InputDouble("", &relvy, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");

        /* field : relative velocity z */
        static double relvz = 0.0;
        ImGui::Text("vz  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
            ImGui::PushID(16);
                ImGui::InputDouble("", &relvz, 0.0, 0.0,"%.5lf");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[km/sec]");

        ImGui::Text("Orientations");

        /* field : orientation variables (combo) */
        static const char *orient_var[] = {"Euler angles ", "Quaternion "};
        static int orient_var_choice = 0;
        ImGui::PushItemWidth(200);
            ImGui::PushID(17);
                ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (orient_var_choice == 0)
        {
            /* field : roll 1 angle */
            static double roll1 = 0.0;
            ImGui::Text("roll 1   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(18);
                    ImGui::InputDouble("", &roll1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : pitch 1 angle */
            static double pitch1 = 0.0;
            ImGui::Text("pitch 1   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(19);
                    ImGui::InputDouble("", &pitch1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            /* field : yaw 1 angle */
            static double yaw1 = 0.0;
            ImGui::Text("yaw 1   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(20);
                    ImGui::InputDouble("", &yaw1, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : roll 2 angle */
            static double roll2 = 0.0;
            ImGui::Text("roll 2   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(21);
                    ImGui::InputDouble("", &roll2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");

            /* field : pitch 2 angle */
            static double pitch2 = 0.0;
            ImGui::Text("pitch 2   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(22);
                    ImGui::InputDouble("", &pitch2, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[deg]");
            
            /* field : yaw 2 angle */
            static double yaw2 = 0.0;
            ImGui::Text("yaw 2   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
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
            static double q10 = 1.0;
            ImGui::Text("q10   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(24);
                    ImGui::InputDouble("", &q10, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q11 quaternion component */
            static double q11 = 0.0;
            ImGui::Text("q11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(25);
                    ImGui::InputDouble("", &q11, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q12 quaternion component */
            static double q12 = 0.0;
            ImGui::Text("q12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(26);
                    ImGui::InputDouble("", &q12, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q13 quaternion component */
            static double q13 = 0.0;
            ImGui::Text("q13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(27);
                    ImGui::InputDouble("", &q13, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");





            /* field : q20 quaternion component */
            static double q20 = 1.0;
            ImGui::Text("q20   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(28);
                    ImGui::InputDouble("", &q20, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q21 quaternion component */
            static double q21 = 0.0;
            ImGui::Text("q21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(29);
                    ImGui::InputDouble("", &q21, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q22 quaternion component */
            static double q22 = 0.0;
            ImGui::Text("q22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(30);
                    ImGui::InputDouble("", &q22, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");

            /* field : q23 quaternion component */
            static double q23 = 0.0;
            ImGui::Text("q23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
                ImGui::PushID(31);
                    ImGui::InputDouble("", &q23, 0.0, 0.0,"%.5lf");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("[  ]");
        }

        //angular velocity

        /*
        ImGui::Text("Angular velocity");

        static const char *frame_type[] = {"Inertial frame", "Body frame"};
        static int frame_type_choice = 0;
        ImGui::PushID(19);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("  ", &frame_type_choice, frame_type, IM_ARRAYSIZE(frame_type));
        ImGui::PopID();
        ImGui::PopItemWidth();
        if (frame_type_choice == 0)
        {
            static double wx = 0.0;
            ImGui::PushID(20);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("wx ");
            ImGui::SameLine();
            ImGui::InputDouble("", &wx, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double wy = 0.0;
            ImGui::PushID(21);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("wy ");
            ImGui::SameLine();
            ImGui::InputDouble("", &wy, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            
            static double wz = 0.0;
            ImGui::PushID(22);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("wz ");
            ImGui::SameLine();
            ImGui::InputDouble("", &wz, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }
        else if (frame_type_choice == 1)
        {
            static double w1 = 0.0;
            ImGui::PushID(23);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("w1 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &w1, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double w2 = 0.0;
            ImGui::PushID(24);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("w2 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &w2, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double w3 = 0.0;
            ImGui::PushID(25);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("w3 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &w3, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }

        //ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Unindent();


        //integrator
        ImGui::Text("Integrator");
        static const char *integ_method[] = {"Runge-Kutta-Fehlberg 78 ", "Bulirsch-Stoer", "Dormand-Prince 5 ", "Runge-Kutta 4 (explicit)"};
        static int integ_method_choice = 0;
        ImGui::PushID(26);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("  ", &integ_method_choice, integ_method, IM_ARRAYSIZE(integ_method));
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double tol = 0.0;
        ImGui::PushID(27);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("tolerance ");
        ImGui::SameLine();
        ImGui::InputDouble("", &tol, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[  ]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Checkbox("Detect binary collision", &detect_binary_collision);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Files to export");
        static bool print_time = true;
        ImGui::Checkbox("Time", &print_time);
        static bool print_rel_pos_vel = true;
        ImGui::Checkbox("Relative position/velocity", &print_rel_pos_vel);
        static bool print_ang = true;
        ImGui::Checkbox("Euler angles", &print_ang);
        static bool print_quat = true;
        ImGui::Checkbox("Quaternions", &print_quat);
        static bool print_wi = true;
        ImGui::Checkbox("Angular velocities (inertial-frame)", &print_wi);
        static bool print_wb = true;
        ImGui::Checkbox("Angular velocities (body-frames)", &print_wb);
        static bool print_ener_mom = true;
        ImGui::Checkbox("Energy & momentum", &print_ener_mom);
        static bool print_kep = true;
        ImGui::Checkbox("Relative Keplerian elements", &print_kep);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));


        if (ImGui::Button("Run", ImVec2(50,30)));
        {

        }


        





        */


        




 
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
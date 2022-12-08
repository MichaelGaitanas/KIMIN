#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>
#include<vector>
#include<string>
#include<filesystem>

std::vector<std::filesystem::path> lsobj(const char *path)
{
    std::vector<std::filesystem::path> paths; 
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
        paths.push_back(entry.path());
    return paths;
}

void raw_hardware_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    return;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0,width,height);
    return;
}

int main()
{
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../font/arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);




    bool show_ell_axes = false;
    bool show_obj_files = false;
    bool ell_xclose = true;
    bool obj_xclose = true;
    bool detect_binary_collision = true;


    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
 
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver); //if undefined behavior, comment delete this line
        ImGui::Begin("GUI");

        //simulation name
        ImGui::Text("Simulation name");
        static char simname[20] = "";
        ImGui::PushItemWidth(150.0f);
        ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Shape model");
        ImGui::Checkbox("Ellipsoids", &show_ell_axes);
        if (show_ell_axes)
        {
            show_obj_files = false;
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowPos().y));
            ell_xclose = true;
            ImGui::Begin("  ", &ell_xclose);
            if (ell_xclose == false)
            {
                show_ell_axes = false;
                ImGui::End();
            }
            else
            {
                ImGui::Text("Semiaxes");

                static double a1 = 0.0f;
                ImGui::PushID(0);
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("a1 ");
                ImGui::SameLine();
                ImGui::InputDouble("", &a1, 0.0f, 0.0f,"%.5lf");
                ImGui::SameLine();
                ImGui::Text("[km]");
                ImGui::PopID();
                ImGui::PopItemWidth();

                static double b1 = 0.0f;
                ImGui::PushID(1);
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("b1 ");
                ImGui::SameLine();
                ImGui::InputDouble("", &b1, 0.0f, 0.0f,"%.5lf");
                ImGui::SameLine();
                ImGui::Text("[km]");
                ImGui::PopID();
                ImGui::PopItemWidth();

                static double c1 = 0.0f;
                ImGui::PushID(2);
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("c1 ");
                ImGui::SameLine();
                ImGui::InputDouble("", &c1, 0.0f, 0.0f,"%.5lf");
                ImGui::SameLine();
                ImGui::Text("[km]");
                ImGui::PopID();
                ImGui::PopItemWidth();

                ImGui::Dummy(ImVec2(0.0f, 10.0f));

                static double a2 = 0.0f;
                ImGui::PushID(3);
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("a2 ");
                ImGui::SameLine();
                ImGui::InputDouble("", &a2, 0.0f, 0.0f,"%.5lf");
                ImGui::SameLine();
                ImGui::Text("[km]");
                ImGui::PopID();
                ImGui::PopItemWidth();

                static double b2 = 0.0f;
                ImGui::PushID(4);
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("b2 ");
                ImGui::SameLine();
                ImGui::InputDouble("", &b2, 0.0f, 0.0f,"%.5lf");
                ImGui::SameLine();
                ImGui::Text("[km]");
                ImGui::PopID();
                ImGui::PopItemWidth();

                static double c2 = 0.0f;
                ImGui::PushID(5);
                ImGui::PushItemWidth(100.0f);
                ImGui::Text("c2 ");
                ImGui::SameLine();
                ImGui::InputDouble("", &c2, 0.0f, 0.0f,"%.5lf");
                ImGui::SameLine();
                ImGui::Text("[km]");
                ImGui::PopID();
                ImGui::PopItemWidth();

                ImGui::Dummy(ImVec2(0.0f, 15.0f));



                if (ImGui::Button("OK", ImVec2(50,30)))
                {
                    show_ell_axes = false;
                }
                ImGui::End();
            }
        }

        ImGui::Checkbox(".obj files", &show_obj_files);
        if (show_obj_files)
        {
            show_ell_axes = false;
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowPos().y));
            obj_xclose = true;
            ImGui::Begin("  ", &obj_xclose);
            if (obj_xclose == false)
            {
                show_obj_files = false;
                ImGui::End();
            }
            else
            {
                ImGui::Text("KIMIN's obj database                                                          ");

                if (ImGui::TreeNodeEx("v"))
                {
                    std::vector<std::filesystem::path> pv = lsobj("../resources/obj/v");
                    for (int i = 0; i < pv.size(); ++i)
                    {
                        ImGui::Selectable(pv[i].string().c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("vf"))
                {
                    std::vector<std::filesystem::path> pvf = lsobj("../resources/obj/vf");
                    for (int i = 0; i < pvf.size(); ++i)
                    {
                        ImGui::Selectable(pvf[i].string().c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("vfn"))
                {
                    std::vector<std::filesystem::path> pvfn = lsobj("../resources/obj/vfn");
                    for (int i = 0; i < pvfn.size(); ++i)
                    {
                        ImGui::Selectable(pvfn[i].string().c_str());
                    }
                    ImGui::TreePop();
                }

                if (ImGui::Button("OK", ImVec2(50,30)))
                {
                    show_obj_files = false;
                }
                ImGui::End();
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        //M1
        ImGui::Text("Mass");
        static double M1 = 0.0;
        ImGui::PushID(0);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("M1 ");
        ImGui::SameLine();
        ImGui::InputDouble("", &M1, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        
        //M2
        static double M2 = 0.0;
        ImGui::PushID(1);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::InputDouble("", &M2, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Integration time parameters");

        //t0
        static double t0 = 0.0;
        ImGui::PushID(2);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("t0             ");
        ImGui::SameLine();
        ImGui::InputDouble("", &t0, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        //tmax
        static double tmax = 0.0;
        ImGui::PushID(3);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("tmax        ");
        ImGui::SameLine();
        ImGui::InputDouble("", &tmax, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        
        //print step dt
        static double print_step = 0.0;
        ImGui::PushID(4);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("print step ");
        ImGui::SameLine();
        ImGui::InputDouble("", &print_step, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f, 20.0f));





        ImGui::Text("Initial state");

        ImGui::Indent();

        ImGui::Text("Relative position");
        //rel x
        static double relx = 0.0;
        ImGui::PushID(5);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("x   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relx, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[km]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        //rel y
        static double rely = 0.0;
        ImGui::PushID(6);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("y   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &rely, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[km]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        //rel z
        static double relz = 0.0;
        ImGui::PushID(7);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("z   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relz, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[km]");
        ImGui::PopID();
        ImGui::PopItemWidth();





        ImGui::Text("Relative velocity");
        //rel vx
        static double relvx = 0.0;
        ImGui::PushID(8);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("vx  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relvx, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        //rel vy
        static double relvy = 0.0;
        ImGui::PushID(9);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("vy  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relvy, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        //rel vz
        static double relvz = 0.0;
        ImGui::PushID(10);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("vz  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relvz, 0.0, 0.0,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        



        //orientation
        ImGui::Text("Orientation");
        static const char *orient_var[] = {"Euler angles ", "Quaternion "};
        static int orient_var_choice = 0;
        ImGui::PushID(11);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
        ImGui::PopID();
        ImGui::PopItemWidth();
        if (orient_var_choice == 0)
        {
            static double roll = 0.0;
            ImGui::PushID(12);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("roll    ");
            ImGui::SameLine();
            ImGui::InputDouble("", &roll, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[deg]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double pitch = 0.0;
            ImGui::PushID(13);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("pitch ");
            ImGui::SameLine();
            ImGui::InputDouble("", &pitch, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[deg]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            
            static double yaw = 0.0;
            ImGui::PushID(14);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("yaw  ");
            ImGui::SameLine();
            ImGui::InputDouble("", &yaw, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[deg]");
            ImGui::PopID();
            ImGui::PopItemWidth();
        }
        else if (orient_var_choice == 1)
        {
            static double q0 = 1.0;
            ImGui::PushID(15);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q0 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q0, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double q1 = 0.0f;
            ImGui::PushID(16);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q1 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q1, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double q2 = 0.0;
            ImGui::PushID(17);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q2 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q2, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double q3 = 0.0;
            ImGui::PushID(18);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q3 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q3, 0.0, 0.0,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();
        }

        //angular velocity
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
#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>

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
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("GUI");

        ImGui::Text("Simulation name");
        ImGui::SameLine();
        static char simname[30] = "";
        ImGui::PushItemWidth(100.0f);
        ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Binary physics model");
        static const char *pmodels[] = {"Mascons", "Expansion", "Polyhedra"};
        static int selected = 0;
        ImGui::PushItemWidth(100.0f);
        ImGui::Combo("  ", &selected, pmodels, IM_ARRAYSIZE(pmodels));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Shape file (.obj)");
        if (ImGui::Button("Load")) { }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Text("Time parameters");
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        static double t0 = 0.0f;
        ImGui::PushID(0);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("Epoch     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &t0, 0.0f, 0.0f,"%.1lf");
        ImGui::SameLine();
        ImGui::Text("[past J2000]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double tmax = 0.0f;
        ImGui::PushID(1);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("Duration  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &tmax, 0.0f, 0.0f,"%.1lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        
        static double print_step = 0.0f;
        ImGui::PushID(2);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("Print step");
        ImGui::SameLine();
        ImGui::InputDouble("", &print_step, 0.0f, 0.0f,"%.1lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Text("State vector");
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        static double relx = 0.0f;
        ImGui::PushID(3);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel x   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relx, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double rely = 0.0f;
        ImGui::PushID(4);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel y   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &rely, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double relz = 0.0f;
        ImGui::PushID(5);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel z   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relz, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double relvx = 0.0f;
        ImGui::PushID(6);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel vx  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relvx, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double relvy = 0.0f;
        ImGui::PushID(7);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel vy  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relvy, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double relvz = 0.0f;
        ImGui::PushID(8);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel vz  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &relvz, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        ////////////////////////////////////////////////////////////////////////////

        static double roll1 = 0.0f;
        ImGui::PushID(9);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("roll 1  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &roll1, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double pitch1 = 0.0f;
        ImGui::PushID(10);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("pitch 1 ");
        ImGui::SameLine();
        ImGui::InputDouble("", &pitch1, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double yaw1 = 0.0f;
        ImGui::PushID(11);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("yaw 1   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &yaw1, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double w1x = 0.0f;
        ImGui::PushID(12);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("w1x     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &w1x, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[rad/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double w1y = 0.0f;
        ImGui::PushID(13);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("w1y     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &w1y, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[rad/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double w1z = 0.0f;
        ImGui::PushID(14);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("w1z     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &w1z, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[rad/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        ////////////////////////////////////////////////////////////////////////////////////

        static double roll2 = 0.0f;
        ImGui::PushID(15);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("roll 2  ");
        ImGui::SameLine();
        ImGui::InputDouble("", &roll2, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double pitch2 = 0.0f;
        ImGui::PushID(16);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("pitch 2 ");
        ImGui::SameLine();
        ImGui::InputDouble("", &pitch2, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double yaw2 = 0.0f;
        ImGui::PushID(17);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("yaw 2   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &yaw2, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double w2x = 0.0f;
        ImGui::PushID(18);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("w2x     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &w2x, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[rad/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double w2y = 0.0f;
        ImGui::PushID(19);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("w2y     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &w2y, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[rad/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double w2z = 0.0f;
        ImGui::PushID(20);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("w2z     ");
        ImGui::SameLine();
        ImGui::InputDouble("", &w2z, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[rad/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        /////////////////////////////////////////////////////////////////////////////////////

        ImGui::Text("Impactor data");
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        static double mass = 0.0f;
        ImGui::PushID(21);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("mass            ");
        ImGui::SameLine();
        ImGui::InputDouble("", &mass, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double vel = 0.0f;
        ImGui::PushID(22);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("rel velocity    ");
        ImGui::SameLine();
        ImGui::InputDouble("", &vel, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[km/sec]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double planeang = 0.0f;
        ImGui::PushID(23);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("angle (plane)   ");
        ImGui::SameLine();
        ImGui::InputDouble("", &planeang, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double offplaneang = 0.0f;
        ImGui::PushID(24);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("angle (offplane)");
        ImGui::SameLine();
        ImGui::InputDouble("", &offplaneang, 0.0f, 0.0f,"%.7lf");
        ImGui::SameLine();
        ImGui::Text("[deg]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        if (ImGui::Button("Run", ImVec2(60,30))) { }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

       
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
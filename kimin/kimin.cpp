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

bool show_ell_axes = false;
bool show_obj_files = false;
bool ell_xclose = true;
bool obj_xclose = true;

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
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //create GUI window
        ImGui::Begin("GUI");

        //simulation name
        ImGui::Text("Simulation name");
        static char simname[20] = "";
        ImGui::PushItemWidth(150.0f);
        ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Shape model");
        ImGui::Checkbox("Ellipsoid", &show_ell_axes);
        if (show_ell_axes)
        {
            show_obj_files = false;
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 200.0f, ImGui::GetWindowPos().y));
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

        ImGui::Checkbox(".obj file", &show_obj_files);
        if (show_obj_files)
        {
            show_ell_axes = false;
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 200.0f, ImGui::GetWindowPos().y));
            obj_xclose = true;
            ImGui::Begin("  ", &obj_xclose);
            if (obj_xclose == false)
            {
                show_obj_files = false;
                ImGui::End();
            }
            else
            {
                ImGui::Text("KIMIN's obj database");

                if (ImGui::TreeNodeEx("v"))
                {
                    std::vector<std::filesystem::path> pv = lsobj("../resources/obj/v");
                    for (int i = 0; i < pv.size(); ++i)
                    {
                        ImGui::Text(pv[i].string().c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("vf"))
                {
                    std::vector<std::filesystem::path> pvf = lsobj("../resources/obj/vf");
                    for (int i = 0; i < pvf.size(); ++i)
                    {
                        ImGui::Text(pvf[i].string().c_str());
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNodeEx("vfn"))
                {
                    std::vector<std::filesystem::path> pvfn = lsobj("../resources/obj/vfn");
                    for (int i = 0; i < pvfn.size(); ++i)
                    {
                        ImGui::Text(pvfn[i].string().c_str());
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

        ImGui::Text("Mass");
        static double M1 = 0.0f;
        ImGui::PushID(0);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("M1 ");
        ImGui::SameLine();
        ImGui::InputDouble("", &M1, 0.0f, 0.0f,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        
        static double M2 = 0.0f;
        ImGui::PushID(1);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::InputDouble("", &M2, 0.0f, 0.0f,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

 
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
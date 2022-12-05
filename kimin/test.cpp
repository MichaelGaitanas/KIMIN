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

    GLFWwindow *window = glfwCreateWindow(800,600, "Rigid body rotation", NULL, NULL);
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
    io.Fonts->AddFontFromFileTTF("../font/arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool render_free_rotation_menu = false;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::Begin("GUI", NULL, ImGuiWindowFlags_MenuBar);

        if(ImGui::BeginMenuBar())
        {
            if(ImGui::BeginMenu("New simulation"))
            {
                if(ImGui::MenuItem("Free rotation"))
                {
                    render_free_rotation_menu = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }



        if (render_free_rotation_menu)
        {
            //simulation name
            ImGui::Text("Simulation name");
            ImGui::SameLine();
            static char simname[40] = "";
            ImGui::PushItemWidth(130.0f);
            ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 15.0f));

            //rigid body shape
            ImGui::Text("Rigid body shape");
            static const char *shape_models[] = {"  ","Database .obj file", "Ellipsoid"};
            static int selected = 0;
            ImGui::PushItemWidth(160.0f);
            ImGui::Combo("  ", &selected, shape_models, IM_ARRAYSIZE(shape_models));
            ImGui::PopItemWidth();
            if (selected == 1)
            {
                ImGui::Begin(".obj database");
                ImGui::Text("This is window A");
                ImGui::End();
            }


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
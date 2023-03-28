#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>

int main()
{
	glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800,600, "Proper threading", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    
    // Initialize ImGui and set up the GLFW and OpenGL3 backends
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../font/Roboto-Regular.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesGreek());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Split the window into two columns
        ImGui::Columns(2, NULL, false);
        ImGui::SetColumnWidth(0, 70.0f);

        static double epoch = 0.0;
        ImGui::Text("Epoch");
        ImGui::NextColumn();
        ImGui::PushItemWidth(100.0f);
        ImGui::PushID(1);
            ImGui::InputDouble("[seconds]", &epoch, 0.0, 0.0,"%g");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        static double dur = 0.0;
        ImGui::Text("Duration");
        ImGui::NextColumn();
        ImGui::PushItemWidth(100.0f);
        ImGui::PushID(2);
            ImGui::InputDouble("[days]", &dur, 0.0, 0.0,"%g");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        // End the columns
        ImGui::Columns(1);

        ImGui::Text("Helloooooooooooo");

        ImGui::Columns(2, NULL, false);
        ImGui::SetColumnWidth(0, 30.0f);

        static double a = 0.0;
        ImGui::Text("a       ");
            ImGui::NextColumn();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(3);
                    ImGui::InputDouble("[km]", &a, 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::NextColumn();
                        
        static double e = 0.0;
        ImGui::Text("e       ");
        ImGui::NextColumn();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(4);
                ImGui::InputDouble("[  ]", &e, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        static double q10 = 12.00043;
        ImGui::Text("q10");
        ImGui::NextColumn();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(5);
                ImGui::InputDouble("[deg]", &q10, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        ImGui::Columns(2);

        ImGui::Render();
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
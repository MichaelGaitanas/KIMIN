#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>

#include<future>
//#include<thread>

class solution
{
public:
    double x;
    solution() : x(1.2345) { }
};

solution run_physics()
{
    solution sol;
    for (double z = 0.0; z < 20000.0; z += 0.001)
        sol.x = exp(sin(sqrt(z*fabs(z)+ cos(z))));
    return sol;
}

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
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    std::future<solution> physics_future;
    bool running_physics = false;

    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Window");

        ImGui::Text("FPS [ %.1f ], ", ImGui::GetIO().Framerate);
        if( ImGui::Button("Test button"))
        { /*Do nothing. Just render a button*/ }
        if (ImGui::Button("Physics button"))
        {
            if (!running_physics)
            {
                physics_future = std::async(std::launch::async, run_physics);
                running_physics = true;
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (running_physics && physics_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            solution result = physics_future.get();
            printf("Physics result: %lf\n", result.x);
            running_physics = false;
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
	return 0;
}
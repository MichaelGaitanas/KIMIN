#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>
#include<cmath>
#include<future>

class Solution
{
public:
    double x;
    Solution() : x(1.2345) { }
};

Solution run_physics()
{
    Solution sol;
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











































class Console
{
private:
    ImGuiTextBuffer buffer;
    bool scroll_to_bottom;
public:
    void cls()
    {
        buffer.clear();
    }
    void add_text(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
    }
    void render()
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(win_width/7.0f, win_height - win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width - 2*win_width/7.0f, win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Console", NULL);
        if (ImGui::Button("Clear "))
            cls();
        ImGui::SameLine();
        ImGui::Text("FPS [ %.1f ], ", ImGui::GetIO().Framerate);
        ImGui::BeginChild("Scroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(buffer.begin());
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();
        ImGui::End();
    }
};

class Properties
{
public:
    double prop;
    bool clicked_run, clicked_kill;
    std::future<Solution> physics_future;
    Properties() : prop(0.1), clicked_run(false), clicked_kill(false) { }

    void render(GLFWwindow *pointer)
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width/7.0f, win_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties ", NULL, ImGuiWindowFlags_MenuBar);
        ImGui::Text("prop"); ImGui::SameLine();
        ImGui::InputDouble(" ", &prop, 0.0, 0.0,"%g");

        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            clicked_run = true;

        if (clicked_run)
        {
            if (ImGui::Button("Kill", ImVec2(50.0f,30.0f)))
                clicked_kill = true;
        }
        else
        {
            ImGui::BeginDisabled();
                ImGui::Button("Kill", ImVec2(50.0f,30.0f));
            ImGui::EndDisabled();
        }

        ImGui::End();
    }
};

class GUI
{
public:
    Properties properties;
    Console console;

    //Constructor and correct initialization of &io.
    GUI(GLFWwindow *pointer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.Fonts->AddFontFromFileTTF("../font/Roboto-Regular.ttf", 15.0f, NULL, io.Fonts->GetGlyphRangesGreek());
        //(void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGuiStyle &imstyle = ImGui::GetStyle();
        imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
        imstyle.FrameRounding = 5.0f;
        imstyle.WindowRounding = 5.0f;
        imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    }

    //Destructor
    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    //Create an new imgui frame.
    void begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    //Render the gui stuff.
    void render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void on_click_run()
    {
        if (properties.clicked_run)
        {
            Solution integrator(properties);

            console.add_text("Running... ");
            //Solution solution = integrator.run();
            std::thread integrator_thread(std::bind(&Integrator::run, integrator));
            integrator_thread.detach();
            properties.clicked_run = false;
        }
        return;
    }
};

class Window
{
    
private:

    GLFWwindow *m_pointer;
    int m_width, m_height;
    float m_aspectratio;

    static void framebuffer_size_callback(GLFWwindow *pointer, int width, int height)
    {
        Window *instance = static_cast<Window*>(glfwGetWindowUserPointer(pointer));
        if (instance != NULL)
        {
            instance->m_width = width;
            instance->m_height = height;
            instance->m_aspectratio = width/(float)height;
            glViewport(0,0, width,height);
        }
        else
            printf("'glfwGetWindowUserPointer(pointer)' is NULL. Exiting framebuffer_size_callback()...\n");

        return;
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        return;
    }

public:

    Window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        m_width = mode->width;
        m_height = mode->height;

        m_pointer = glfwCreateWindow(m_width, m_height, "KIMIN", NULL, NULL);
        if (m_pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwSetWindowUserPointer(m_pointer, this);
        glfwMakeContextCurrent(m_pointer);
        glfwSetWindowSizeLimits(m_pointer, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwSetFramebufferSizeCallback(m_pointer, framebuffer_size_callback);
        glfwSetKeyCallback(m_pointer, key_callback);
    }

    ~Window()
    {
        glfwDestroyWindow(m_pointer);
        glfwTerminate();
    }

    void game_loop()
    {
        GUI gui(m_pointer);
        glClearColor(0.2f,0.2f,0.2f,1.0f);

        while (!glfwWindowShouldClose(m_pointer))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            gui.begin();
            gui.properties.render(m_pointer);
            gui.console.render();
            gui.render();

            gui.on_click_run();

            glfwSwapBuffers(m_pointer);
            glfwPollEvents();
        }
        return;
    }
};

int main()
{
    Window window;
    window.game_loop();

    return 0;
}
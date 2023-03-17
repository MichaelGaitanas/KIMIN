
#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<cstdio>
#include<cstdarg>
#include<string>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

//This function identifies the operating system in which the program is running.
std::string os_name()
{
    #if defined(__APPLE__) || defined(__MACH__)
        return "Mac OSX";
    #elif defined(__linux__) || defined(__unix) || defined(__unix__) || defined(__FreeBSD__)
        return "Linux||Unix||FreeBSD";
    #elif defined(_WIN32) || defined(_WIN64)
        return "Windows";
    #else
        return "Other OS";
    #endif
}

class Console
{
private:
    ImGuiTextBuffer buffer;
    bool scroll_to_bottom;
public:
    //Clear the console.
    void cls()
    {
        buffer.clear();
    }

    //Add formatted text to the console.
    void add(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
    }

    //Render the console imgui window.
    void render(const char *title = "Console ", bool *popened = NULL)
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(win_width/7.0f, win_height - win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width - 2*win_width/7.0f, win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, popened);

        //mouse input : clear the console
        if (ImGui::Button("Clear "))
            cls();
        
        ImGui::SameLine();

        ImGui::Text("FPS [ %.1f ],  ", ImGui::GetIO().Framerate);

        ImGui::SameLine();
        
        ImGui::Text("OS [ %s ]", os_name().c_str());

        ImGui::Separator();

        ImGui::BeginChild("Scroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(buffer.begin());
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();

        ImGui::End();
    }
};

class GUI
{
public:
    //Properties properties;
    //Graphics graphics;
    Console console;

    //Constructor and correct initialization of &io.
    GUI(GLFWwindow *pointer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.Fonts->AddFontFromFileTTF("../font/arial.ttf", 15.0f);
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGuiStyle &imstyle = ImGui::GetStyle();
        imstyle.FrameRounding = 5.0f;
        imstyle.WindowRounding = 5.0f;
    }

    //Destructor
    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    //Create an new imgui frame.
    void new_frame()
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
        return;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        return;
    }

public:

    Window(int width, int height, const char* title) : m_width(width),
                                                       m_height(height)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        m_pointer = glfwCreateWindow(m_width, m_height, title, NULL, NULL);
        if (m_pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwMakeContextCurrent(m_pointer);
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

            gui.new_frame();
            gui.console.render();
            gui.render();

            glfwSwapBuffers(m_pointer);
            glfwPollEvents();
        }
        return;
    }
};



int main()
{
    Window window(800,600,"KIMIN");
    window.game_loop();

    return 0;
}
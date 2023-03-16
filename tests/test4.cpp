#include<cstdio>

//#include"../imgui/imgui.h"
//#include"../imgui/imgui_impl_glfw.h"
//#include"../imgui/imgui_impl_opengl3.h"
//#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

class Callback
{
public:
    int width, height;
    float aspectratio;
    
    Callback(Window &window) : width(window.width),
                               height(window.height),
                               aspectratio(window.aspectratio)
    { }

    void framebuffer_size_callback(GLFWwindow *pointer, int w, int h)
    {
        width  = w;
        height = h;
        aspectratio = (float)w/h;

        glViewport(0,0, w,h);
    }

    void key_callback(GLFWwindow *pointer, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(pointer, true);
    }
};

class Window
{
public:
    GLFWwindow *pointer; //unique window pointer
    const GLFWvidmode *mode; 
    int width, height; //glfw window size
    float aspectratio; //glfw window aspect ratio, basically width/height
    Callback callback;

    //Costructor to run once a glfw (window) object is instantiated.
    Window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        width = height = 800;
        pointer = glfwCreateWindow(width, height, "KIMIN", NULL, NULL);
        if (pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwMakeContextCurrent(pointer);
        glfwSwapInterval(1);
        glfwSetWindowSizeLimits(pointer, 600, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwGetWindowSize(pointer, &width, &height);
        aspectratio = width/(float)height;

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwSetFramebufferSizeCallback(pointer, callback.framebuffer_size_callback);
        glfwSetKeyCallback(pointer, callback.key_callback);
    }

    //Destructor.
    ~Window()
    {
        glfwTerminate();
    }

};

int main()
{
    Window window;

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window.pointer))
    {
        glClear(GL_COLOR_BUFFER_BIT);


        glfwSwapBuffers(window.pointer);
        glfwPollEvents();
    }

    return 0;
}
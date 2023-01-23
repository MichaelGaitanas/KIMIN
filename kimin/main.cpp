#include<cstdio>
#include<cmath>

#include"../include/window.hpp"
#include"../include/gui.hpp"
#include"../include/directory.hpp"
//#include"../include/logger.hpp"
#include"../include/ios.hpp"

int main()
{
    window win;
    win.add_key_callback();
    win.add_framebuffer_size_callback();

    gui::create_imgui_context(); //static member function
    gui ui(win.pointer);

    inputs ins;
    //logger lg;

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(win.pointer))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        ui.new_frame();
        ui.left_panel(ins, win.pointer);
        ui.render();

        glfwSwapBuffers(win.pointer);
        glfwPollEvents();
    }

    return 0;
}
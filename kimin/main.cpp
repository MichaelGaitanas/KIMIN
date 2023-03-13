
#include<cstdio>
#include<cmath>
#include<thread>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"../include/typedef.hpp"
#include"../include/glfw.hpp"
#include"../include/gui.hpp"
#include"../include/properties.hpp"

int main()
{
    glfw win;
    win.add_key_callback();
    win.add_framebuffer_size_callback();
    gui::create_imgui_context(); //static member function
    gui ui(win.pointer);

    properties props;

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(win.pointer))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        ui.new_frame();
        ui.lp.render(props, win.pointer);
        ui.rp.render(props);
        ui.dp.render("Log ", NULL, (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));
        ui.render();
        glfwSwapBuffers(win.pointer);
        glfwPollEvents();

        if (props.clicked_run)
        {
            strvec errors = props.validate();
            if (!errors.size())
            {
                std::thread propagation_thread(std::bind(&properties::propagate, props));
                propagation_thread.detach();
                ui.dp.add("Running physics...  ");
                    
                ui.dp.add("Done."); ui.dp.add("\n");
                ui.dp.add("Exporting txt files...  ");
                    props.export_txt_files();
                ui.dp.add("Done."); ui.dp.add("\n");
            }
            else
            {
                for (int i = 0; i < errors.size(); ++i)
                {
                    ui.dp.add(errors[i].c_str());
                    ui.dp.add("\n");
                }
                ui.dp.add("\n");
            }
        }
        props.clicked_run = false;
    }

    return 0;
}
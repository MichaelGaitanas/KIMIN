#include<cstdio>
#include<cmath>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"../include/glfw.hpp"
#include"../include/gui.hpp"
#include"../include/logger.hpp"
#include"../include/io.hpp"

int main()
{
    glfw win;
    win.add_key_callback();
    win.add_framebuffer_size_callback();

    gui::create_imgui_context(); //static member function
    gui ui(win.pointer);

    inputs ins;
    logger lg;

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(win.pointer))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        ui.new_frame();
        ui.left_panel(ins, win.pointer);

        if (ins.clicked_run)
        {
            strvec errors = ins.validate();
            if (!errors.size())
            {
                lg.add("Running physics...  ");
                    ins.propagate();
                lg.add("Done."); lg.add("\n");

                lg.add("Exporting txt files...  ");
                    //ins.export_json_files();
                    ins.export_txt_files();
                lg.add("Done."); lg.add("\n");
            }
            else
            {
                for (int i = 0; i < errors.size(); ++i)
                {
                    lg.add(errors[i].c_str());
                    lg.add("\n");
                }
                lg.add("\n");
            }
            
        }
        lg.draw("Log ", NULL, (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));

        ins.clicked_run = false;

        ui.right_panel();
        
        ui.render();

        glfwSwapBuffers(win.pointer);
        glfwPollEvents();
    }

    return 0;
}
#include<cstdio>
#include<cmath>

#include"../include/window.hpp"

int main()
{
    Window window;
    window.game_loop();

    return 0;
}








































/*
int main()
{
    Window window;
    window.add_key_callback();
    window.add_framebuffer_size_callback();
    
    GUI::create_imgui_context(); //static member function
    GUI gui(window.pointer);

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window.pointer))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        gui.new_frame();
        gui.console.render();
        gui.properties.render(window.pointer);
        gui.graphics.render();
        gui.render();

        if (gui.properties.clicked_run)
        {
            strvec errors = gui.properties.validate();
            if (!errors.size())
            {
                Propagator propagator(gui.properties);
                propagator.run();
                //std::thread propagator_thread(std::bind(&Propagator::run, propagator));
                //propagator_thread.detach();
            }
            else
            {
                for (int i = 0; i < errors.size(); ++i)
                {
                    gui.console.add(errors[i].c_str());
                    gui.console.add("\n");
                }
                gui.console.add("\n");
            }
        }
        gui.properties.clicked_run = false;


        glfwSwapBuffers(window.pointer);
        glfwPollEvents();
    }

    return 0;
}
*/
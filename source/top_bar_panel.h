/* This class handles the rendering logic of the top bar in the gui. */

#ifndef TOP_BAR_PANEL_H
#define TOP_BAR_PANEL_H

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

class top_bar_panel
{
public:
    bool export_is_enabled;
    bool export_sol_clicked;
    
    top_bar_panel() : export_is_enabled(false),
                      export_sol_clicked(false)
    { }

    void render()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Export solution", nullptr, false, export_is_enabled))
                    export_sol_clicked = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
};

#endif
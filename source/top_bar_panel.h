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
    bool export_txt_clicked, export_json_clicked;
    
    top_bar_panel() : export_is_enabled(false),
                      export_txt_clicked(false),
                      export_json_clicked(false)
    { }

    void render()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Export .txt", nullptr, false, export_is_enabled))
                {
                    export_txt_clicked = true;
                }
                if (ImGui::MenuItem("Export .json", nullptr, false, export_is_enabled))
                {
                    export_json_clicked = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
};

#endif
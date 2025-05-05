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

    void render(GLFWwindow *wpointer, bool &confirm_exit)
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

        if (confirm_exit)
        {
            float sx = ImGui::GetIO().DisplaySize.x, sy = ImGui::GetIO().DisplaySize.y;
            ImGui::SetNextWindowPos(ImVec2(0.5f*sx, 0.5f*sy), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(0.1f*sx, 0.1f*sy), ImGuiCond_Always);
            ImGui::Begin("Exit KIMIN ?", &confirm_exit, ImGuiWindowFlags_NoResize);
            
            ImVec2 butt_size(50.0f, 30.0f);
            float butt_spacing = ImGui::GetStyle().ItemSpacing.x;
            float total_width = 2.0f*butt_size.x + butt_spacing;
            float avail_width = ImGui::GetContentRegionAvail().x;
            float xoffset = 0.5f*(avail_width - total_width);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xoffset);
            //Now draw the two buttons:
            if (ImGui::Button("No", butt_size))
                confirm_exit = false;
            ImGui::SameLine();
            if (ImGui::Button("Yes", butt_size))
                glfwSetWindowShouldClose(wpointer, true);

            ImGui::End();
        }
    }
};

#endif
/* This class handles the rendering logic of the top bar in the gui. */

#ifndef TOP_BAR_PANEL_H
#define TOP_BAR_PANEL_H

#include<vector>
#include<filesystem>

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

class top_bar_panel
{
public:
    bool import_props_clicked, import_props_confirm;
    std::string properties_path;
    std::vector<std::filesystem::path> properties_list;
    bool export_is_enabled, export_sol_clicked;
    
    top_bar_panel() : import_props_clicked(false),
                      import_props_confirm(false),
                      properties_path(""),
                      properties_list{},
                      export_is_enabled(false),
                      export_sol_clicked(false)
    { }

    void render(GLFWwindow *wpointer, bool &confirm_exit)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import properties"))
                {
                    import_props_clicked = true;
                    properties_list = list_properties_files(); //List the files (once) right after the click to 'Import properties'. Remember, this will work even if u paste a new file while the app is running.
                }
                if (ImGui::MenuItem("Export active solution", nullptr, false, export_is_enabled))
                    export_sol_clicked = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (import_props_clicked)
        {
            float sx = ImGui::GetIO().DisplaySize.x;
            float sy = ImGui::GetIO().DisplaySize.y;
            ImVec2 winsize{ 0.3f*sx, 0.4f*sy };
            ImGui::SetNextWindowSize(winsize, ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImVec2(0.5f*sx, 0.5f*sy), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)); 
            ImGui::Begin("Select properties file", &import_props_clicked);
            if (ImGui::TreeNodeEx("Available properties files", ImGuiTreeNodeFlags_Framed))
            {
                for (size_t i = 0; i < properties_list.size(); ++i)
                {
                    const auto &p = properties_list[i];
                    std::string display = p.parent_path().filename().string() + "/" + p.filename().string();
                    bool selected = (properties_path == p.string());
                    if (ImGui::Selectable(display.c_str(), selected))
                        properties_path = p.string();
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "Import file" button. This must be pressed, otherwise the properties pannel will not be updated.
            if (ImGui::Button("Import file", ImVec2(70.0f,30.0f)))
            {
                import_props_clicked = false; //This will close the window (encapsulated).
                if (!properties_path.empty())
                    import_props_confirm = true; //And this is will communicate with gui::poll_topbar_events(), which then will communicate with properties::import_file().
            }
            ImGui::End();
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

private:
    std::vector<std::filesystem::path> list_properties_files()
    {
        std::vector<std::filesystem::path> paths;

        //First scan all simulations/ child directories.
        std::filesystem::path simsdir = "../simulations";
        if (std::filesystem::exists(simsdir) && std::filesystem::is_directory(simsdir))
            for (auto &entry : std::filesystem::directory_iterator(simsdir))
                if (entry.is_directory())
                {
                    std::filesystem::path p = entry.path()/"properties.txt";
                    if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p))
                        paths.push_back(p);
                }

        //Then scan the properties/ directory.
        std::filesystem::path propsdir = "../properties";
        if (std::filesystem::exists(propsdir) && std::filesystem::is_directory(propsdir))
        {
            for (auto &entry : std::filesystem::directory_iterator(propsdir))
                if (entry.is_regular_file() && entry.path().extension() == ".txt")
                    paths.push_back(entry.path());
        }
    
        return paths;
    }
};

#endif
/* This class handles the rendering logic of the top bar in the gui. */

#ifndef TOPBAR_H
#define TOPBAR_H

#include<vector>
#include<filesystem>
#include<string>

#include<GLFW/glfw3.h>

#include"../imgui/imgui.h"

#include"constants.h"

class topbar
{
public:
    bool import_properties_clicked, import_properties_confirm;
    std::string properties_path;
    std::vector<std::filesystem::path> properties_list;
    bool export_solution_is_enabled, export_solution_clicked;
    
    topbar() : import_properties_clicked(false),
               import_properties_confirm(false),
               properties_path(""),
               properties_list{},
               export_solution_is_enabled(false),
               export_solution_clicked(false)
    { }

    void render(GLFWwindow *wpointer, bool &confirm_exit)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import properties"))
                {
                    import_properties_clicked = true;
                    properties_list = list_properties_files(); //List the files after the click to 'Import properties'. This will work even if u paste a new file while the KIMIN is running.
                }
                if (ImGui::MenuItem("Export active solution", nullptr, false, export_solution_is_enabled))
                    export_solution_clicked = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        const float sx = ImGui::GetIO().DisplaySize.x;
        const float sy = ImGui::GetIO().DisplaySize.y;
        if (import_properties_clicked)
        {
            ImGui::SetNextWindowSize(ImVec2(0.4f*sx, 0.4f*sy), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos( ImVec2(0.5f*sx, 0.5f*sy), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)); 
            ImGui::Begin("Select properties file", &import_properties_clicked);
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
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));

            if (properties_path.empty())
                ImGui::BeginDisabled();

            //Final "Import file" button. This must be pressed, otherwise the properties pannel will not be updated.
            if (ImGui::Button("Import file", ImVec2(70.0f*SCX,30.0f*SCY)))
            {
                import_properties_clicked = false; //This will close the window.
                if (!properties_path.empty())
                    import_properties_confirm = true; //And this is will communicate with gui::poll_topbar_events(), which then will communicate with properties::import_file().
            }

            if (properties_path.empty())
                ImGui::EndDisabled();

            ImGui::End();
        }

        if (confirm_exit)
        {
            ImGui::SetNextWindowPos( ImVec2(0.5f*sx, 0.5f*sy), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(0.1f*sx, 0.1f*sy), ImGuiCond_Always);
            ImGui::Begin("Exit KIMIN ?", &confirm_exit, ImGuiWindowFlags_NoResize);
            
            const ImVec2 button_size(50.0f*SCX, 30.0f*SCY);
            const float total_width = 2.0f*button_size.x + ImGui::GetStyle().ItemSpacing.x;
            const float avail_width = ImGui::GetContentRegionAvail().x;
            const float xoffset = 0.5f*(avail_width - total_width);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xoffset);
            //Now draw the two buttons :
            if (ImGui::Button("No", button_size))
                confirm_exit = false;
            ImGui::SameLine();
            if (ImGui::Button("Yes", button_size))
                glfwSetWindowShouldClose(wpointer, true);

            ImGui::End();
        }
    }

private:
    std::vector<std::filesystem::path> list_properties_files()
    {
        std::vector<std::filesystem::path> paths;

        //First scan all simulations/ child directories.
        std::filesystem::path simsdir = SIM_ROOT_DIR;
        if (std::filesystem::exists(simsdir) && std::filesystem::is_directory(simsdir))
            for (auto &entry : std::filesystem::directory_iterator(simsdir))
                if (entry.is_directory())
                {
                    std::filesystem::path p = entry.path()/"properties.txt";
                    if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p))
                        paths.push_back(p);
                }

        //Then scan the properties/ directory.
        std::filesystem::path propsdir = PROPERTIES_DIR;
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
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include"imgui.h"

#include<cstdio>
#include<cstdarg>

//KIMIN's log window
class logger
{
public:
    ImGuiTextBuffer buffer;
    bool scroll_to_bottom;

    //Clear the log.
    void cls()
    {
        buffer.clear();
    }

    //Add formatted text to the log.
    void add(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
    }

    //Render the log imgui window on top (actually as part) of the glfw window.
    void draw(const char *title, bool *popened = NULL, float monitor_width = 1.0f, float monitor_height = 1.0f)
    {
        ImGui::SetNextWindowPos(ImVec2(monitor_width/2.0f, monitor_height - 225.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(monitor_width/2.0f, 150.0f), ImGuiCond_FirstUseEver); 
        ImGui::Begin(title, popened);

        // field : clear the log
        if (ImGui::Button("Clear "))
            cls();
        
        ImGui::SameLine();

        ImGui::Text("[ %.1f fps ]", ImGui::GetIO().Framerate);

        ImGui::Separator();

        ImGui::BeginChild("Scroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(buffer.begin());
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();

        ImGui::End();
    }
};

#endif
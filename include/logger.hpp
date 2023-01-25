#ifndef LOGGER_HPP
#define LOGGER_HPP

#include"imgui.h"

#include<cstdio>
#include<cstdarg>

#include"directory.hpp"

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
    void draw(const char *title, bool *popened = NULL, str vendor = "", str renderer = "", str version = "")
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(win_width/7.0f, win_height - win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width - 2*win_width/7.0f, win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, popened);

        //mouse input : clear the log
        if (ImGui::Button("Clear "))
            cls();
        
        ImGui::SameLine();

        ImGui::Text("fps [ %.1f ], ", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::Text("Vendor [ %s ], ", vendor.c_str());
        ImGui::SameLine();
        ImGui::Text("Renderer [ %s ], ", renderer.c_str());
        ImGui::SameLine();
        ImGui::Text("OpenGL [ %s ], ", version.c_str());
        ImGui::SameLine();
        ImGui::Text("OS [ %s ] ", os_name().c_str()); //directory.hpp
        ImGui::SameLine();
        ImGui::Text(" %c", "|/-\\"[(int)(ImGui::GetTime()/0.1f) & 3]);

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
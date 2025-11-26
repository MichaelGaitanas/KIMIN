/* This class handles the rendering logic of the down panel (console) in the gui. */

#ifndef CONSOLE_PANEL_H
#define CONSOLE_PANEL_H

#include<cstdarg>
#include<sstream>
#include<string>

#include<boost/date_time.hpp>
#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include"constants.h"

class console_panel
{
private:
    ImGuiTextBuffer buffer{};
    bool scroll_to_bottom = false;
    const int max_buffer_size = 60000; //In bytes (1 byte for each ASCII char and 1-4 bytes for each unicode char due to UTF-8 encoding).

    void cls()
    {
        buffer.clear();
    }

    void auto_cls()
    {
        if (buffer.size() > max_buffer_size)
        {
            buffer.clear();
            buffer.append("[Console] : Automatic clearance of the console.");
            scroll_to_bottom = true;
        }
    }

    std::string get_local_time()
    {
        boost::posix_time::ptime timeloc = boost::posix_time::second_clock::local_time();
        std::ostringstream datetime;
        datetime << "[" << timeloc << "] ";
        return datetime.str();
    }

public:
    //Add formatted text to the console.
    void add_text(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
        auto_cls();
    }

    //Add formatted local time and then formatted text to the console.
    void add_timed_text(const char *text)
    {
        add_text(get_local_time().c_str());
        add_text(text);
    }

    //Render the console window.
    void render()
    {
        float sx = ImGui::GetIO().DisplaySize.x;
        float sy = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0.15f*sx, 0.8f*sy), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.7f*sx, 0.2f*sy), ImGuiCond_FirstUseEver);
        ImGui::Begin("Console", nullptr);

        //Clear button.
        if (ImGui::Button("Clear", ImVec2(60.0f*SCX, 25.0f*SCY)))
            cls();
        
        //Display FPS and GPU.
        ImGui::SameLine();
        ImGui::Text("FPS [ %.0f ]     GPU [ %s ]", ImGui::GetIO().Framerate, glGetString(GL_RENDERER));
        ImGui::Separator();
        
        ImGui::BeginChild("Scroll", ImVec2(0.0f,0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(buffer.begin());
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();
        ImGui::End();
    }
};

#endif
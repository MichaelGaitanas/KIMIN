#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdarg>

#include"typedef.hpp"

#include <sstream>
#include <boost/date_time.hpp>

class Console
{

private:

    ImGuiTextBuffer buffer;
    bool scroll_to_bottom;

public:

    //Clear the console.
    void cls()
    {
        buffer.clear();
    }

    //Add formatted text to the console.
    void add_text(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
    }

    //Identify the operating system.
    str os_name()
    {
        #if defined(__APPLE__) || defined(__MACH__)
            return "Mac OSX";
        #elif defined(__linux__) || defined(__unix) || defined(__unix__)
            return "Linux||Unix";
        #elif defined(__FreeBSD__)
            return "FreeBSD";
        #elif defined(_WIN32) || defined(_WIN64)
            return "Windows";
        #else
            return "Other";
        #endif
    }

    //Render the console imgui window.
    void render()
    {
        float offsety = 0;
        float ysize = ImGui::GetIO().DisplaySize.y/7.0f;
        if ( ysize < 200){
            offsety = 200 - ysize;
        }
        ImGui::SetNextWindowPos( ImVec2(     ImGui::GetIO().DisplaySize.x/7.0f, 6.0f*ImGui::GetIO().DisplaySize.y/7.0f - offsety), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(5.0f*ImGui::GetIO().DisplaySize.x/7.0f,      ImGui::GetIO().DisplaySize.y/7.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Console", nullptr);
        //Mouse input : Clear the console.
        if (ImGui::Button("Clear "))
            cls();
        
        //Display FPS and OS.
        ImGui::SameLine();
        ImGui::Text("Monitor FPS [ %.1f ] ,  OS [ %s ] ,  GPU [ %s ] ", ImGui::GetIO().Framerate, os_name().c_str(), glGetString(GL_RENDERER));
        ImGui::Separator();
        
        ImGui::BeginChild("Scroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(buffer.begin());
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();
        ImGui::End();
    }

    void timedlog(const char *format){
        add_text(get_local_time().c_str());
        add_text(format);
        add_text("\n");
    }

    std::string get_local_time(){
        boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
        std::ostringstream datetime;
        datetime << "[" << timeLocal << "] ";
        std::string outdate = datetime.str();
        return outdate;
    }

};

#endif
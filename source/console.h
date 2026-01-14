/* This class handles the rendering logic of the down panel (console) in the gui. */

#ifndef CONSOLE_H
#define CONSOLE_H

#include<cstdarg>
#include<sstream>
#include<string>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include"constants.h"

class console
{
private:
    ImGuiTextBuffer buffer{};
    bool scroll_to_bottom = false;

    void auto_clear()
    {
        if (buffer.size() > CONSOLE_BUFFER_MAX_SIZE)
        {
            buffer.clear();
            buffer.append("[Console] : Automatic clearance of the console.");
            scroll_to_bottom = true;
        }
    }

public:
    //Print formatted text to the console.
    void print(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
        auto_clear();
    }

    //Render the console window.
    void render()
    {
        const float sx = ImGui::GetIO().DisplaySize.x;
        const float sy = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0.15f*sx, 0.8f*sy), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.7f*sx, 0.2f*sy), ImGuiCond_FirstUseEver);
        ImGui::Begin("Console", nullptr);

        //Clear button.
        if (ImGui::Button("Clear", ImVec2(60.0f*SCX, 25.0f*SCY)))
            buffer.clear();
        
        //Display FPS and GPU.
        ImGui::SameLine();
        ImGui::Text("FPS [ %.0f ]   -   GPU [ %s ]   -   OpenGL [ %s ]", ImGui::GetIO().Framerate, glGetString(GL_RENDERER), glGetString(GL_VERSION));
        ImGui::Separator();
        
        ImGui::BeginChild("Scroll", ImVec2(0.0f,0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f,0.8f,0.1f, 1.0f));
        ImGui::TextUnformatted(buffer.begin());
        ImGui::PopStyleColor();
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();
        
        ImGui::End();
    }
};

#endif
/*
#include<algorithm>

bool TwoDSlider(const char* label, ImVec2* value, ImVec2 min, ImVec2 max, ImVec2 size = ImVec2(100,100))
{
    ImGui::Text("%s", label);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(label, size);

    bool changed = false;
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0))
    {
        ImVec2 mouse = ImGui::GetIO().MousePos;
        float x = (mouse.x - pos.x) / size.x;
        float y = (mouse.y - pos.y) / size.y;
        value->x = min.x + x * (max.x - min.x);
        value->y = min.y + y * (max.y - min.y);
        changed = true;
    }

    // Clamp
    value->x = std::clamp(value->x, min.x, max.x);
    value->y = std::clamp(value->y, min.y, max.y);

    // Draw background
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(pos, ImVec2(pos.x+size.x, pos.y+size.y), IM_COL32(60,60,60,255));
    draw_list->AddRect(pos, ImVec2(pos.x+size.x, pos.y+size.y), IM_COL32(255,255,255,255));

    // Draw handle
    float tx = (value->x - min.x) / (max.x - min.x);
    float ty = (value->y - min.y) / (max.y - min.y);
    ImVec2 handle = ImVec2(pos.x + tx * size.x, pos.y + ty * size.y);
    draw_list->AddCircleFilled(handle, 5.0f, IM_COL32(255,0,0,255));

    return changed;
}

ImVec2 myval = ImVec2(0.0f, 0.0f);
if (TwoDSlider("2D Control", &myval, ImVec2(-1,-1), ImVec2(1,1)))
{
    // myval.x, myval.y updated
}
*/
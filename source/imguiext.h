/* This header contains solely "imgui exterior" functions, in the sense that "extends" imgui's standard capabilities. We do this in order to
   accomplish some gui features that are not natively supported by imgui. Here, we obey the naming conventions adopted by imgui's source code.  */

#ifndef IMGUIEXT_H
#define IMGUIEXT_H

#include"../imgui/imgui.h"

namespace ImGuiExt
{
    //This function renders a triangle upon a gui widget.
    void TreeTriangle(ImDrawList *list, ImVec2 center, float r, bool down, ImU32 col)
    {
        list->PathClear();
        if (down)
        {
            list->PathLineTo(ImVec2(center.x - r, center.y - 0.6f*r));
            list->PathLineTo(ImVec2(center.x + r, center.y - 0.6f*r));
            list->PathLineTo(ImVec2(center.x,     center.y +      r));
        }
        else
        {
            list->PathLineTo(ImVec2(center.x - 0.6f*r, center.y - r));
            list->PathLineTo(ImVec2(center.x - 0.6f*r, center.y + r));
            list->PathLineTo(ImVec2(center.x +      r, center.y));
        }
        list->PathFillConvex(col);
    }

    //This function renders a TreeNodeEx() + ImGui::SeparatorText() effect. The whole purpose is to render a collapsing header and a gray line at the same row.
    bool TreeNodeSeparatorText(const char *label, bool default_open = false)
    {
        const ImGuiID id = ImGui::GetID(label);

        ImGuiStorage *storage = ImGui::GetStateStorage();
        bool open = storage->GetBool(id, default_open);

        //Full-width row sizing
        const float w = ImGui::GetContentRegionAvail().x, h = ImGui::GetFrameHeight();
        const ImVec2 start = ImGui::GetCursorScreenPos();

        //Create a clickable item, spanning the full row.
        ImGui::InvisibleButton(label, ImVec2(w,h)); //"label" is used only for ID, not drawn.
        const bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            open = !open;
            storage->SetBool(id, open);
        }

        ImDrawList *list  = ImGui::GetWindowDrawList();
        ImGuiStyle &style = ImGui::GetStyle();

        //Hover background :
        if (hovered)
            list->AddRectFilled(start, ImVec2(start.x + w, start.y + h), ImGui::GetColorU32(ImGuiCol_HeaderHovered), style.FrameRounding);

        //Widget layout : line + triangle + text + line
        const float pad_x   = style.FramePadding.x;
        const float pad_y   = style.FramePadding.y;
        const float arrow_r = 0.22f*h;
        const ImVec2 text_size = ImGui::CalcTextSize(label);
        const float inner = style.ItemInnerSpacing.x;

        //"Group" : triangle (width = 2*arrow_r) + inner spacing + text
        const float group_w = 2.0f*arrow_r + inner + text_size.x;
        //Choose where the group starts (left aligned, with inset) :
        const float left_inset = 20.0f*SCX;
        const float group_x0 = start.x + left_inset + pad_x;

        const ImVec2 arrow_center(group_x0 + arrow_r, start.y + 0.5f*h);
        const float text_x = group_x0 + 2.0f*arrow_r + inner;
        const ImVec2 text_pos(text_x, start.y + pad_y);
        const float mid_y = start.y + 0.5f*h;
        const float gap = 10.0f*SCX;

        //Lines with equal void space around the group :
        const float left_line_x1  = group_x0 - gap;
        const float right_line_x0 = group_x0 + group_w + gap;

        list->AddLine(ImVec2(start.x, mid_y),       ImVec2(left_line_x1,  mid_y), ImGui::GetColorU32(ImGuiCol_Separator), style.SeparatorTextBorderSize);
        list->AddLine(ImVec2(right_line_x0, mid_y), ImVec2(start.x + w, mid_y),   ImGui::GetColorU32(ImGuiCol_Separator), style.SeparatorTextBorderSize);

        //Triangle + text :
        TreeTriangle(list, arrow_center, arrow_r, open, ImGui::GetColorU32(ImGuiCol_Text));
        list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

        return open;
    }
}

#endif
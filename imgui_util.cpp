#include "imgui_util.h"

namespace EB
{
    namespace ImGui
    {
        namespace Keyboard
        {
            int get_pressed_key(ImGuiIO& io)
            {
                for(int i=0; i < IM_ARRAYSIZE(io.KeysDown); i++) 
                    if(::ImGui::IsKeyDown(i)) return i;

                return 0;
            }

            int get_released_key(ImGuiIO& io)
            {
                for(int i=0; i < IM_ARRAYSIZE(io.KeysDown); i++) 
                    if(::ImGui::IsKeyReleased(i)) return i;

                return 0;
            }

            std::string key_to_str(int vkey)
            {
                UINT scanCode = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);

                CHAR szName[128];
                int result = 0;

                switch (vkey)
                {
                    case VK_LEFT: 
                    case VK_UP: 
                    case VK_RIGHT: 
                    case VK_DOWN:
                    case VK_RCONTROL:
                    case VK_RMENU:
                    case VK_LWIN: 
                    case VK_RWIN: 
                    case VK_APPS:
                    case VK_PRIOR: 
                    case VK_NEXT:
                    case VK_END: 
                    case VK_HOME:
                    case VK_INSERT: 
                    case VK_DELETE:
                    case VK_DIVIDE:
                    case VK_NUMLOCK:
                    scanCode |= KF_EXTENDED;
                    default:
                    result = GetKeyNameTextA(scanCode << 16, szName, 128);
                }

                if(result == 0) return "";

                return szName;
            }
        }

        namespace Text
        {
            void center(std::string text)
            {
                ImVec2 window_size = ::ImGui::GetWindowSize();
                ImVec2 text_size   = ::ImGui::CalcTextSize(text.c_str());

                ::ImGui::SetCursorPosX((window_size.x / 2) - (text_size.x / 2));
                ::ImGui::Text(text.c_str());
            }

            void right(std::string text)
            {
                ImVec2 window_size = ::ImGui::GetWindowSize();
                ImVec2 text_size   = ::ImGui::CalcTextSize(text.c_str());

                ::ImGui::SetCursorPosX(window_size.x - text_size.x);
                ::ImGui::Text(text.c_str());
            }

            void partial_colored(std::string text_left, std::string colored_text_middle, std::string text_right, ImVec4 color, float spacing)
            {
                ::ImGui::Text(text_left.c_str());
                ::ImGui::SameLine();

                ::ImGui::SetCursorPosX(::ImGui::GetCursorPosX() - spacing);
                ::ImGui::TextColored(color, colored_text_middle.c_str());
                ::ImGui::SameLine();

                ::ImGui::SetCursorPosX(::ImGui::GetCursorPosX() - spacing);
                ::ImGui::Text(text_right.c_str());
            }

            void center_partial_colored(std::string text_left, std::string colored_text_middle, std::string text_right, ImVec4 color, float spacing)
            {
                ImVec2 window_size = ::ImGui::GetWindowSize();
                ImVec2 text_size   = ::ImGui::CalcTextSize((text_left+" "+colored_text_middle+" "+text_right).c_str());

                ::ImGui::SetCursorPosX((window_size.x / 2) - (text_size.x / 2));
                ::ImGui::Text(text_left.c_str());
                ::ImGui::SameLine();

                ::ImGui::SetCursorPosX(::ImGui::GetCursorPosX() - spacing);
                ::ImGui::TextColored(color, colored_text_middle.c_str());
                ::ImGui::SameLine();

                ::ImGui::SetCursorPosX(::ImGui::GetCursorPosX() - spacing);
                ::ImGui::Text(text_right.c_str());
            }

            std::string padding(std::string text, unsigned int size, bool left_padding)
            {
                for(unsigned int i=0; i < size; i++)
                {
                    if(left_padding)
                        text = " " + text;
                    else
                        text += " ";
                }

                return text;
            }
        }

        namespace Layout
        {
            void center_item(float item_width)
            {
                ImVec2 window_size  = ::ImGui::GetWindowSize();
                float indent_amount = (window_size.x / 2) - (item_width / 2);

                ::ImGui::SetNextItemWidth(item_width);
                ::ImGui::SetCursorPosX(indent_amount);
            }
        }

        namespace UI
        {
            // Credit to: https://github.com/ocornut/imgui/issues/1901#issue-335266223
            // @value: between 0.f and 1.f
            // @size_arg: x is width, y is height
            /*
            EB::ImGui::UI::loading_bar("##test_bar", 
            0.25f, 
            { 400.f, 6.f },
            ImGui::ColorConvertFloat4ToU32(ImColor(255.f, 255.f, 255.f)), 
            ImGui::ColorConvertFloat4ToU32(ImColor(255.f, 0.f, 0.f)));
            */
            bool loading_bar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) 
            {
                ImGuiWindow* window = ::ImGui::GetCurrentWindow();
                if (window->SkipItems)
                    return false;

                ImGuiContext& g = *GImGui;
                const ImGuiStyle& style = g.Style;
                const ImGuiID id = window->GetID(label);

                ImVec2 pos = window->DC.CursorPos;
                ImVec2 size = size_arg;
                size.x -= style.FramePadding.x * 2;

                const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
                ::ImGui::ItemSize(bb, style.FramePadding.y);
                if (!::ImGui::ItemAdd(bb, id))
                    return false;

                // Render
                const float circleStart = size.x * 0.8f;
                const float circleEnd = size.x - 50.f;
                const float circleWidth = circleEnd - circleStart;

                window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
                window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);

                const float t = g.Time;
                const float r = size.y / 2;
                const float speed = 1.5f;

                const float a = speed*0;
                const float b = speed*0.333f;
                const float c = speed*0.666f;

                const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
                const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
                const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;

                window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
                window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
                window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
            }

            // Credit to: https://github.com/ocornut/imgui/issues/1901#issue-335266223 
            bool loading_spinner(const char* label, float radius, int thickness, const ImU32& color)
            {
                ImGuiWindow* window = ::ImGui::GetCurrentWindow();
                if (window->SkipItems)
                    return false;

                ImGuiContext& g = *GImGui;
                const ImGuiStyle& style = g.Style;
                const ImGuiID id = window->GetID(label);

                ImVec2 pos = window->DC.CursorPos;
                ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);

                const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
                ::ImGui::ItemSize(bb, style.FramePadding.y);
                if (!::ImGui::ItemAdd(bb, id))
                    return false;

                // Render
                window->DrawList->PathClear();

                int num_segments = 30;
                int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));

                const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
                const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

                const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);

                for (int i = 0; i < num_segments; i++) {
                    const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
                    window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * radius,
                                                 centre.y + ImSin(a+g.Time*8) * radius));
                }

                window->DrawList->PathStroke(color, false, thickness);
            }

            // Credit to: https://github.com/ocornut/imgui/issues/1901#issuecomment-444929973
            // EB::ImGui::UI::loading_circle("##test", 25, ImColor(255.f, 255.f, 255.f), ImColor(255.f, 0.f, 0.f), 6, 5.f);
            void loading_circle(const char* label, const float indicator_radius, const ImVec4& main_color, const ImVec4& backdrop_color, const int circle_count, const float speed)
            {
                ImGuiWindow* window = ::ImGui::GetCurrentWindow();
                if (window->SkipItems) {
                    return;
                }

                ImGuiContext& g = *GImGui;
                const ImGuiStyle& style = g.Style;
                const ImGuiID id = window->GetID(label);

                const ImVec2 pos = window->DC.CursorPos;
                const float circle_radius = indicator_radius / 10.0f;
                const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f,
                                pos.y + indicator_radius * 2.0f));
                ::ImGui::ItemSize(bb, style.FramePadding.y);
                if (!::ImGui::ItemAdd(bb, id)) {
                    return;
                }
                const float t = g.Time;
                const auto degree_offset = 2.0f * IM_PI / circle_count;
                for (int i = 0; i < circle_count; ++i) {
                    const auto x = indicator_radius * std::sin(degree_offset * i);
                    const auto y = indicator_radius * std::cos(degree_offset * i);
                    const auto growth = max(0.0f, std::sin(t * speed - i * degree_offset));
                    ImVec4 color;
                    color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
                    color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
                    color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
                    color.w = 1.0f;
                    window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x,
                                                      pos.y + indicator_radius - y),
                                                      circle_radius + growth * circle_radius,
                                                      ::ImGui::GetColorU32(color));
                }
            }
        }

        void new_line()
        {
            ::ImGui::Text("");
        }
    }
}
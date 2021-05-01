#pragma once

#include <Windows.h>
#include "imgui.h"
#include "imgui_internal.h"

#include <string>
#include <cmath>
#include "stdarg.h"

namespace EB
{
    namespace ImGui
    {
        namespace Keyboard
        {
            // Returns the first key currently down
            int get_pressed_key(ImGuiIO& io);

            // Returns the last released key
            int get_released_key(ImGuiIO& io);

            std::string key_to_str(int vkey);
        }

        namespace Text
        {
            void center(std::string text);
            void center_colored(std::string text, ImVec4 color);
            void right(std::string text);
            void partial_colored(std::string text_left, std::string colored_text_middle, std::string text_right, ImVec4 color, float spacing=8.f);
            void center_partial_colored(std::string text_left, std::string colored_text_middle, std::string text_right, ImVec4 color, float spacing=8.f);
            std::string padding(std::string text, unsigned int size, bool left_padding=true);
        }
        
        namespace Layout
        {
            void center_item(float item_width);
        }

        namespace UI
        {
            bool loading_bar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
            bool loading_spinner(const char* label, float radius, int thickness, const ImU32& color);
            void loading_circle(const char* label, const float indicator_radius, const ImVec4& main_color, const ImVec4& backdrop_color, const int circle_count, const float speed);
        }

        void new_line();
    }
}

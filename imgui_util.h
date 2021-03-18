#pragma once

#include <Windows.h>
#include "imgui.h"

#include <string>
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
            void right(std::string text);
            std::string padding(std::string text, unsigned int size, bool left_padding=true);
        }
        
        namespace Layout
        {
            void center_item(float item_width);
        }

        void new_line();
    }
}

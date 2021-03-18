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

        void new_line()
        {
            ::ImGui::Text("");
        }
    }
}

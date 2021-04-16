#pragma once

#include <string>
#include <locale>
#include <codecvt>

// Requirements: C++11

namespace EB
{
    namespace WString
    {
        std::string to_string(std::wstring const& wstr);
    }
}

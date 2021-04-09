#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <utility>

namespace EB
{
    namespace String
    {
        std::vector<std::string> split(std::string const& s, char delim);

        std::wstring to_wstring(std::string const& str);
    }
}

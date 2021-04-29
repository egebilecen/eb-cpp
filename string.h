#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <algorithm>

namespace EB
{
    namespace String
    {
        std::vector<std::string> split(std::string const& s, char delim);
        void ltrim(std::string& str);
        void rtrim(std::string& str);
        void trim(std::string& str);
        void lowercase(std::string& str);
        void uppercase(std::string& str);

        std::wstring to_wstring(std::string const& str);
    }
}

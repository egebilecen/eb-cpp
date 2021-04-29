#include "string.h"

namespace EB
{
    namespace String
    {
        std::vector<std::string> split(std::string const& s, char delim)
        {
            std::vector<std::string> result;
            std::istringstream iss(s);

            for (std::string token; std::getline(iss, token, delim); )
            {
                result.push_back(std::move(token));
            }

            return result;
        }

        void ltrim(std::string& str)
        {
            str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
        }

        void rtrim(std::string& str)
        {
            str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), str.end());
        }

        void trim(std::string& str)
        {
            ltrim(str);
            rtrim(str);
        }

        std::string& lowercase(std::string& str)
        {
            std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){
                return std::tolower(c);
            });

            return str;
        }

        std::string& uppercase(std::string& str)
        {
            std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){
                return std::toupper(c);
            });

            return str;
        }

        std::wstring to_wstring(std::string const& str)
        {
            return std::wstring(str.begin(), str.end());
        }
    }
}

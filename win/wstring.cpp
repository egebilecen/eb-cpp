#include "wstring.h"

namespace EB
{
    namespace WString
    {
        std::string to_string(std::wstring const& wstr)
        {
            using convert_type = std::codecvt_utf8<wchar_t>;
            std::wstring_convert<convert_type, wchar_t> converter;

            return converter.to_bytes(wstr);
        }
    }
}

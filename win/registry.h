#pragma once

#include <Windows.h>
#include <string>

namespace EB
{
    namespace Windows
    {
        namespace Registry
        {
            enum class ErrorCode
            {
                NONE,
                READ_ERROR,
                INCORRECT_VALUE_TYPE_ERROR
            };

            static ErrorCode _last_error_code = ErrorCode::NONE;
                
            std::wstring read_registry_value_string(HKEY root, std::wstring key, std::wstring name);
            ErrorCode get_last_error_code();
        }
    }
}

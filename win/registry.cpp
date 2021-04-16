#include "registry.h"

namespace EB
{
    namespace Windows
    {
        namespace Registry
        {
            std::wstring read_registry_value_string(HKEY root, std::wstring key, std::wstring name)
            {
                HKEY hkey;

                if(RegOpenKeyEx(root, key.c_str(), 0, KEY_READ, &hkey) != ERROR_SUCCESS)
                {
                    _last_error_code = ErrorCode::READ_ERROR;
                    return L"";
                }

                DWORD type;
                DWORD data_size;

                if(RegQueryValueEx(hkey, name.c_str(), NULL, &type, NULL, &data_size) != ERROR_SUCCESS)
                {
                    RegCloseKey(hkey);

                    _last_error_code = ErrorCode::READ_ERROR;
                    return L"";
                }

                if(type != REG_SZ)
                {
                    RegCloseKey(hkey);

                    _last_error_code = ErrorCode::INCORRECT_VALUE_TYPE_ERROR;
                    return L"";
                }

                std::wstring value(data_size / sizeof(wchar_t), L'\0');

                if(RegQueryValueExW(hkey, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &data_size) != ERROR_SUCCESS)
                {
                    RegCloseKey(hkey);

                    _last_error_code = ErrorCode::READ_ERROR;
                    return L"";
                }

                RegCloseKey(hkey);

                size_t first_null = value.find_first_of(L'\0');
                if(first_null != std::wstring::npos) 
                    value.resize(first_null);

                _last_error_code = ErrorCode::NONE;
                return value;
            }

            ErrorCode get_last_error_code()
            {
                return _last_error_code;
            }
        }
    }
}

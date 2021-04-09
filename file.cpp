#include "file.h"

namespace EB
{
    namespace File
    {
        bool is_exist(std::string filename)
        {
            struct stat buffer;
            return stat(filename.c_str(), &buffer) == 0;
        }
    }
}

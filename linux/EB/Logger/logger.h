#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>

using namespace std;

namespace EB
{
    namespace Logger
    {
        //--------------------------------- Enums
        enum LOG_TYPE : uint8_t
        {
            INFO = 1,
            WARNING,
            ERROR
        };

        struct LOG_COLOR
        {
            //https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
            inline static string const RESET   = "\033[0m";
            inline static string const INFO    = "\033[0;97;100m"; //white text
            inline static string const WARNING = "\033[0;93;100m"; //light yellow text
            inline static string const ERROR   = "\033[0;31;100m"; //red text
            inline static string const FILE    = "\033[0;97;44m";
        };

        //--------------------------------- Variables
        string         extern LOG_FILE;
        LOG_TYPE const extern DEFAULT_LOG_TYPE;
        string   const extern EOL;

        //--------------------------------- Functions
        string GetFileName(string file_path);

        void Log(string   text,
                 string   caller_file,
                 int      caller_line,
                 LOG_TYPE type       = DEFAULT_LOG_TYPE,
                 bool     write2file = true);
    }
}

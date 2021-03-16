#include "logger.h"

using namespace std;

namespace EB
{
    namespace Logger
    {
        //--------------------------------- Variable Definations
        string         LOG_FILE         = "logger.log";
        LOG_TYPE const DEFAULT_LOG_TYPE = LOG_TYPE::INFO;
        string   const EOL              = "\n";

        //--------------------------------- Function Definations
        string GetFileName(string file_path)
        {
            char     delimiter = '/';
            uint32_t occurance = file_path.find_last_of(delimiter);
            string   file_name = file_path.substr(occurance + 1);

            return file_name;
        }

        void Log(string   text,
                 string   caller_file,
                 int      caller_line,
                 LOG_TYPE type,
                 bool     write2file)
        {
            string log_text;
            time_t raw_time         = time(nullptr);
            struct tm* current_time = localtime(&raw_time);

            string type_text;
            string type_color;
            string date = (current_time->tm_mday    < 10 ? "0" : "") + to_string(current_time->tm_mday)    + "/" +
                          (current_time->tm_mon + 1 < 10 ? "0" : "") + to_string(current_time->tm_mon + 1) + "/" +
                           to_string(1900 + current_time->tm_year);
            string time = (current_time->tm_hour < 10 ? "0" : "") + to_string(current_time->tm_hour) + ":" +
                          (current_time->tm_min  < 10 ? "0" : "") + to_string(current_time->tm_min)  + ":" +
                          (current_time->tm_sec  < 10 ? "0" : "") + to_string(current_time->tm_sec);

            if     (type == LOG_TYPE::INFO)    type_text = "[INFO]";
            else if(type == LOG_TYPE::WARNING) type_text = "[WARNING]";
            else if(type == LOG_TYPE::ERROR)   type_text = "[ERROR]";
            else                               type_text = "[WTF?]";

            if     (type == LOG_TYPE::INFO)    type_color = LOG_COLOR::INFO;
            else if(type == LOG_TYPE::WARNING) type_color = LOG_COLOR::WARNING;
            else if(type == LOG_TYPE::ERROR)   type_color = LOG_COLOR::ERROR;

            log_text = type_color + type_text + LOG_COLOR::RESET +
                       " - " +
                       "[" + date + ", " + time + "]" +
                       " - " +
                       text +
                       " "+ LOG_COLOR::FILE + "(" + GetFileName(caller_file) + ":" + to_string(caller_line) + ")" + LOG_COLOR::RESET +
                       EOL;

            cout << log_text;

            if(write2file)
            {
                ofstream file(LOG_FILE, ios_base::app);
                file << log_text;
                file.close();
            }
        }
    }
}

#include "web_request.h"

namespace EB
{
    namespace WebRequest
    {
        size_t _write_data(void* ptr, size_t size, size_t nmemb, void* stream)
        {
            size_t written = fwrite(ptr, size, nmemb, (FILE*) stream);
            return written;
        }

        size_t _write_string(void* ptr, size_t size, size_t nmemb, std::string* data) 
        {
            data->append((char*) ptr, size * nmemb);
            return size * nmemb;
        }

        std::string _convert_post_field_to_str(PostField& post_field)
        {
            std::string post_field_str = "";

            for(int i=0; i < post_field.size(); i++)
            {
                PostData data = post_field[i];
                post_field_str += data.first + "=" + data.second;

                if(i != post_field.size() - 1) post_field_str += "&";
            }

            return post_field_str;
        }

        bool download_file(std::string url, std::string file_path) 
        {
            CURL* curl;
            FILE* file;

            curl = curl_easy_init();

            if(curl)
            {
                file = fopen(file_path.c_str(), "wb");

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

                if(url.substr(0, 6) == "https:")
                {
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
                }

                curl_easy_perform(curl);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &_last_http_code);
                curl_easy_cleanup(curl);

                fclose(file);

                return true;
            }

            _last_http_code = -1;
            return false;
        }
    
        bool send_post_request(std::string url, PostField post_field, std::string* response)
        {
            *response = "";

            CURL *curl;
            curl = curl_easy_init();

            if(curl)
            {
                std::string post_field_str = _convert_post_field_to_str(post_field);

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_field_str.c_str());

                curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());

                if(url.substr(0, 6) == "https:")
                {
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
                }

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_string);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

                curl_easy_perform(curl);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &_last_http_code);
                curl_easy_cleanup(curl);

                return true;
            }

            _last_http_code = -1;
            return false;
        }

        bool send_get_request(std::string url, std::string* response)
        {
            *response = "";

            CURL *curl;
            curl = curl_easy_init();

            if(curl)
            {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

                curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_string);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

                curl_easy_perform(curl);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &_last_http_code);
                curl_easy_cleanup(curl);

                return true;
            }

            _last_http_code = -1;
            return false;
        }

        long get_last_http_code()
        {
            return _last_http_code;
        }
    }
}

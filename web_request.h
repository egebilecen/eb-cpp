#pragma once
#define CURL_STATICLIB

#include <curl/curl.h>
#include <string>
#include <vector>
#include <utility>

/*
    Include these libraries:
    libcurl_a.lib
    Ws2_32.lib
    Crypt32.lib
    Wldap32.lib
    Normaliz.lib
*/

namespace EB
{
    namespace WebRequest
    {
        typedef std::pair<std::string, std::string> PostData;
        typedef std::vector<PostData> PostField;

        static long        last_http_code = -1;
        static std::string useragent      = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/89.0.4389.90 Safari/537.36";

        size_t _write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
        size_t _write_string(void *ptr, size_t size, size_t nmemb, std::string* data);
        std::string _convert_post_field_to_str(PostField& post_field);

        bool download_file(std::string url, std::string file_path);

        // post_field = "example=1&request=yes"
        bool send_post_request(std::string url, PostField post_field, std::string* response);
        bool send_get_request(std::string url, std::string* response);
    }
}

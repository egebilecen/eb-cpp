#include "string.h"

namespace EB
{
    size_t String::delimiter_count(std::string const * const str,
                                   char const                delimiter,
                                   std::vector<size_t>*      pos_out)
    {
        size_t occurance         = 0;
        std::string::size_type i = 0;

        while((i = (*str).find(delimiter, i)) != std::string::npos)
        {
            if(pos_out != nullptr) (*pos_out).push_back(i);

            occurance++;
            i += 1;
        }

        return occurance;
    }

    std::vector<std::string> String::substrings(std::string const * const         str,
                                                std::vector<size_t> const * const pos_list)
    {
        size_t start_i = 0;
        std::vector<std::string> substrings;

        for(size_t i=0; i < (*pos_list).size(); i++)
        {
            size_t pos = (*pos_list).at(i);

            substrings.push_back((*str).substr(start_i, pos - start_i));

            start_i += pos + 1;
        }

        return substrings;
    }

    std::pair<std::string, std::string> String::split_to_two(std::string const * const str,
                                                             char const                delimiter)
    {
        size_t pos = (*str).find(delimiter, 0);

        return std::pair((*str).substr(0, pos), (*str).substr(pos + 1, (*str).length() - (pos + 1)));
    }
}

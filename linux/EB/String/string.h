#pragma once

#include <string>
#include <vector>
#include <utility>

namespace EB
{
    class String
    {
        public:
            static size_t delimiter_count(std::string const * const str,
                                          char const                delimiter,
                                          std::vector<size_t>*      pos_out = nullptr);

            static std::vector<std::string> substrings(std::string const * const         str,
                                                       std::vector<size_t> const * const pos_list);

            static std::pair<std::string, std::string> split_to_two(std::string const * const str,
                                                                    char const                delimiter);
    };
}

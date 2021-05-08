#include "utility.h"

namespace EB
{
    namespace Utility
    {
        int rand_int(const int& min, const int& max){
            static bool first = true;

            if (first) 
            {  
                srand(time(NULL)); //seeding for the first time only!
                first = false;
            }

            return min + rand() % (max + 1 - min);
        }

        std::string rand_str(const size_t& length)
        {
            std::string random_str = "";

            for(size_t i=0; i < length; i++)
                random_str += ALPHABET_ALLCASES[rand_int(0, ALPHABET_ALLCASES_LENGTH)];

            return random_str;
        }
    }
}

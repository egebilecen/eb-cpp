#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../String/string.h"

namespace EB
{
    namespace Protocol
    {
        class Base
        {
            public:
                //--------------------------------- Type Defines
                typedef std::pair<std::string, void *> KeyParamPair;
                typedef std::vector<KeyParamPair> ParamList;
                typedef void (*FuncHandler)(Base * const);
                typedef std::pair<std::string, std::string> KeyValPair;
                typedef std::pair<std::string, FuncHandler> KeyHandlerPair;

                //--------------------------------- Constructor Decleration
                Base();

                //--------------------------------- Getter Method Decleration(s)
                FuncHandler get_handler(std::string key);
                void * get_param(std::string key);

                //--------------------------------- Adder Method Decleration(s)
                void add_handler(std::string key,
                                 FuncHandler func);
                void add_param(std::string  key,
                               void * const ptr);

                //--------------------------------- Method Decleration(s)
                virtual void handle(std::string const * const data);

                //--------------------------------- Variable Decleration(s)
                std::vector<KeyHandlerPair> handler_list;
                ParamList                   param_list;

            protected:
                //--------------------------------- Method Decleration(s)
                std::vector<KeyValPair> parse(std::string const * const data);
        };

        class EBP : public Base {};
    }
}

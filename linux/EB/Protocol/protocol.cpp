#include "protocol.h"

namespace EB
{
    namespace Protocol
    {
    //--------------------------------- Public
        //--------------------------------- Constructor Defination
        Base::Base() {}

        //--------------------------------- Getter Method Defination(s)
        Base::FuncHandler Base::get_handler(std::string key)
        {
            for(uint32_t i=0; i < this->handler_list.size(); i++)
            {
                KeyHandlerPair p     = this->handler_list.at(i);
                std::string    f_key = std::get<0>(p);

                if(key == f_key)
                {
                    FuncHandler f = std::get<1>(p);
                    return f;
                }
            }

            return nullptr;
        }

        void * Base::get_param(std::string key)
        {
            for(uint32_t i=0; i < this->param_list.size(); i++)
            {
                KeyParamPair p     = this->param_list.at(i);
                std::string  p_key = std::get<0>(p);

                if(key == p_key)
                {
                    void * param = std::get<1>(p);
                    return param;
                }
            }

            return nullptr;
        }

        //--------------------------------- Adder Method Defination(s)
        void Base::add_handler(std::string key,
                               FuncHandler func)
        {
            KeyHandlerPair p(key, func);
            this->handler_list.push_back(p);
        }

        void Base::add_param(std::string  key,
                             void * const ptr)
        {
            KeyParamPair p(key, ptr);
            this->param_list.push_back(p);
        }

        //--------------------------------- Method Defination(s)
        void Base::handle(std::string const * const data)
        {
                std::vector<KeyValPair> parsed_data = this->parse(data);

                for(size_t i=0; i < parsed_data.size(); i++)
                {
                    KeyValPair data_pair = parsed_data.at(i);

                    FuncHandler func_handler = this->get_handler(std::get<0>(data_pair));

                    if(func_handler != nullptr) func_handler(this);
                }
        }

    //--------------------------------- Private
        //--------------------------------- Method Defination(s)
        std::vector<Base::KeyValPair> Base::parse(std::string const * const data)
        {
            std::vector<KeyValPair> kv_vec;

            std::vector<size_t> pos_list;
            String::delimiter_count(data, ';', &pos_list);
            std::vector<std::string> split_data = String::substrings(data, &pos_list);

            for(size_t i=0; i < split_data.size(); i++)
            {
                std::string _data = split_data.at(i);

                kv_vec.push_back(String::split_to_two(&_data, ':'));
            }

            return kv_vec;
        }
    }
}

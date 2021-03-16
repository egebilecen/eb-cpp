#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <fstream>
#include <sys/stat.h>

namespace EB
{
    class Settings
    {
    public:
        //---------------- Public
        //------- Type Definations
        typedef std::pair<std::string, std::string> SettingPair;
        typedef std::vector<SettingPair> SettingList;

    private:
        //---------------- Private
        //------- Config Variables
        char seperator = '=';

        //------- Setting Variables
        std::string settings_file;
        SettingList default_settings;
        SettingList settings;

        //------- Getter Function Declarations
        std::string get_settings_str(SettingList& setting_list);
        int         get_setting_index_by_key(std::string key);

    public:
        //---------------- Public
        //------- Constructor Declarations
        Settings(std::string settings_file);

        //------- General Function Declarations
        bool load();
        bool save();

        //------- Setter Function Declarations
        void set_default_settings(SettingList& settings);

        template <typename T> 
        bool set_setting(std::string key, T value, bool autosave=true);

        //------- Getter Function Declarations
        template <typename T> 
        T get_setting(std::string key);
    };

    //---------------- Inline Template Function Definations
    //------- Setter Function
    template<typename T>
    inline bool Settings::set_setting(std::string key, T value, bool autosave)
    {
        int setting_index = this->get_setting_index_by_key(key);

        if(setting_index < 0) return false;

        this->settings[setting_index] = SettingPair(key, std::to_string(value));

        if(autosave) this->save();

        return true;
    }

    template<>
    inline bool Settings::set_setting(std::string key, std::string value, bool autosave)
    {
        int setting_index = this->get_setting_index_by_key(key);

        if(setting_index < 0) return false;

        this->settings[setting_index] = SettingPair(key, value);

        if(autosave) this->save();

        return true;
    }

    //------- Getter Function
    template<typename T>
    inline T Settings::get_setting(std::string key)
    {
        int setting_index = this->get_setting_index_by_key(key);

        if(setting_index < 0) return T();

        std::istringstream convert(this->settings[setting_index].second);

        T value;
        convert >> value;

        return value;
    }
}

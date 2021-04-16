#include "settings.h"

namespace EB
{
    //---------------------------------- Function Definations
    //---------------- Private
    //------- Getter Functions
    std::string Settings::get_settings_str(SettingList& setting_list)
    {
        std::string settings_str = "";

        for(size_t i=0; i < setting_list.size(); i++)
        {
            SettingPair setting = setting_list[i];
            settings_str += setting.first + this->seperator + setting.second + "\n";
        }

        return settings_str;
    }

    int Settings::get_setting_index_by_key(std::string const& key, bool search_in_default_settings)
    {
        EB::Settings::SettingList& setting_list = 
            search_in_default_settings ? this->default_settings : this->settings;

        for(int i=0; i < setting_list.size(); i++)
            if(setting_list[i].first == key) return i;

        return -1;
    }

    int Settings::get_setting_index_by_value(std::string const& value, int nth)
    {
        int found = 0;

        for(int i=0; i < this->settings.size(); i++)
        {
            if(this->settings[i].second == value
            && (found++) + 1 == nth) return i;
        }

        return -1;
    }

    //---------------- Public
    //------- Constructor
    Settings::Settings(std::string settings_file) : settings_file(settings_file)
    {
    }

    //------- General Function Definations
    bool Settings::load()
    {
        std::string file_content = "";

        // Check if file exist. If not, create a file with default settings.
        struct stat file_info;

        if(stat(this->settings_file.c_str(), &file_info) != 0)
        {
            std::ofstream file;
            file.open(this->settings_file);

            if(!file.is_open()) return false;

            if(this->default_settings.size() > 0)
                file << this->get_settings_str(this->default_settings);

            file.close();
        }

        // Open and read the content
        std::ifstream file;
        std::string   temp_str;
        file.open(settings_file);
        
        if(!file.is_open()) return false;

        while(std::getline(file, temp_str))
        {
            size_t delimiter_pos = temp_str.find(this->seperator);
            std::string key      = temp_str.substr(0, delimiter_pos);
            std::string value    = temp_str.substr(delimiter_pos + 1, temp_str.length());

            this->settings.emplace_back(key, value);
        }

        file.close();

        return true;
    }

    bool Settings::save()
    {
        std::ofstream file;
        file.open(this->settings_file);

        if(!file.is_open()) return false;

        file << this->get_settings_str(this->settings);
        file.close();

        return true;
    }

    void Settings::reset_to_default_settings(std::vector<std::string>* settings_to_exclude)
    {
        Settings::SettingList settings;

        for(size_t i=0; i < this->default_settings.size(); i++)
        {
            Settings::SettingPair default_setting = this->default_settings[i];

            if(settings_to_exclude != nullptr
            && std::find(settings_to_exclude->begin(), settings_to_exclude->end(), default_setting.first) != settings_to_exclude->end())
            {
                int setting_index = this->get_setting_index_by_key(default_setting.first);

                if(setting_index > -1)
                {
                    settings.push_back(this->settings[setting_index]);
                    continue;
                }
            }

            settings.push_back(default_setting);
        }

        this->settings = settings;
    }

    bool Settings::is_setting_exist(std::string const& key)
    {
        return this->get_setting_index_by_key(key) < 0 ? false : true;
    }

    //------- Setter Function Definations
    void Settings::set_default_settings(SettingList& settings)
    {
        this->default_settings = settings;
    }

    //------- Getter Function Definations
    std::vector<std::string> Settings::get_setting_keys()
    {
        std::vector<std::string> key_list;
        key_list.reserve(this->settings.size());

        for(size_t i=0; i < this->settings.size(); i++)
            key_list.push_back(this->settings[i].first);

        return key_list;
    }
}

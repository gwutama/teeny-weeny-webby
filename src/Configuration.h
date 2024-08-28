#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <map>

class Configuration {
public:
    Configuration();
    void load_from_file(const std::string& config_file);
    std::string get_value(const std::string& key, const std::string& default_value = "") const;
    int get_int_value(const std::string& key, int default_value = 0) const;
    bool is_valid() const { return valid; }

private:
    void parse_config_file(const std::string& config_file);
    std::map<std::string, std::string> config_map;
    bool valid;
};

#endif // CONFIGURATION_H

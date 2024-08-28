#include "Configuration.h"

#include <fstream>
#include <iostream>
#include <sstream>

Configuration::Configuration() : valid(true) {}

void Configuration::load_from_file(const std::string& config_file) {
    parse_config_file(config_file);
}

void Configuration::parse_config_file(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open configuration file: " << config_file << std::endl;
        valid = false;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream line_stream(line);
        std::string key, value;
        if (std::getline(line_stream, key, '=') && std::getline(line_stream, value)) {
            config_map[key] = value;
        } else {
            std::cerr << "Error: Invalid configuration line: " << line << std::endl;
            valid = false;
        }
    }
}

std::string Configuration::get_value(const std::string& key, const std::string& default_value) const {
    auto it = config_map.find(key);
    if (it != config_map.end()) {
        return it->second;
    }
    return default_value;
}

int Configuration::get_int_value(const std::string& key, int default_value) const {
    auto it = config_map.find(key);
    if (it != config_map.end()) {
        return std::stoi(it->second);
    }
    return default_value;
}

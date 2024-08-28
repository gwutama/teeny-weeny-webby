#include <iostream>
#include <unistd.h>

#include "Server.h"
#include "Configuration.h"

const std::string DEFAULT_SERVER_HOST = "0.0.0.0";
const int DEFAULT_SERVER_PORT = 8080;
const int DEFAULT_THREAD_POOL_SIZE = 4;
const std::string DEFAULT_ROOT_DIR = "./www/";
const std::string DEFAULT_CGI_BIN_DIR = "./cgi-bin/";
const std::string DEFAULT_UPLOAD_DIR = "./uploads/";
const int DEFAULT_BUFFER_SIZE = 8192;
const int DEFAULT_CLIENT_TIMEOUT = 60;

void print_help() {
    std::cout << "Usage: tww [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -c <config_file>  Specify the configuration file\n";
    std::cout << "  -h                Show this help message and exit\n";
}

int main(int argc, char* argv[]) {
    // Default values
    std::string config_file = "";
    std::string server_host = DEFAULT_SERVER_HOST;
    int server_port = DEFAULT_SERVER_PORT;
    int thread_pool_size = DEFAULT_THREAD_POOL_SIZE;
    std::string root_dir = DEFAULT_ROOT_DIR;
    std::string cgi_bin_dir = DEFAULT_CGI_BIN_DIR;
    std::string upload_dir = DEFAULT_UPLOAD_DIR;
    int buffer_size = DEFAULT_BUFFER_SIZE;
    int client_timeout = DEFAULT_CLIENT_TIMEOUT;

    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:h")) != -1) {
        switch (opt) {
            case 'c':
                config_file = optarg;
                break;
            case 'h':
                print_help();
                return 0;
            default:
                std::cerr << "Usage: tww [-c <config_file>] [-h]\n";
                return 1;
        }
    }

    // Load configuration from file if -c is provided
    Configuration config;
    if (!config_file.empty()) {
        config.load_from_file(config_file);
        if (!config.is_valid()) {
            std::cerr << "Invalid configuration file: " << config_file << std::endl;
            return 1;
        }

        // Retrieve configuration values
        server_host = config.get_value("SERVER_HOST", server_host);
        server_port = config.get_int_value("SERVER_PORT", server_port);
        thread_pool_size = config.get_int_value("THREAD_POOL_SIZE", thread_pool_size);
        root_dir = config.get_value("ROOT_DIR", root_dir);
        cgi_bin_dir = config.get_value("CGI_BIN_DIR", cgi_bin_dir);
        upload_dir = config.get_value("UPLOAD_DIR", upload_dir);
        buffer_size = config.get_int_value("BUFFER_SIZE", buffer_size);
        client_timeout = config.get_int_value("CLIENT_TIMEOUT", client_timeout);
    }

    // Print configuration to stdout
    std::cout << "Server Configuration:" << std::endl;
    std::cout << "SERVER_HOST: " << server_host << std::endl;
    std::cout << "SERVER_PORT: " << server_port << std::endl;
    std::cout << "THREAD_POOL_SIZE: " << thread_pool_size << std::endl;
    std::cout << "ROOT_DIR: " << root_dir << std::endl;
    std::cout << "CGI_BIN_DIR: " << cgi_bin_dir << std::endl;
    std::cout << "UPLOAD_DIR: " << upload_dir << std::endl;
    std::cout << "BUFFER_SIZE: " << buffer_size << " bytes" << std::endl;
    std::cout << "CLIENT_TIMEOUT: " << client_timeout << " seconds" << std::endl;

    // Start the server
    Server server(server_host, server_port, root_dir, cgi_bin_dir, upload_dir, thread_pool_size, client_timeout, buffer_size);
    server.start();

    return 0;
}

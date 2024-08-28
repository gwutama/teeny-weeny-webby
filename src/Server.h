#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "ThreadPool.h"
#include "Response.h"
#include "Request.h"

class Server {
public:
    Server(const std::string& host, int port,
           const std::string& root_dir, const std::string& cgi_bin_dir, const std::string& upload_dir,
           size_t num_threads, int client_timeout, int buffer_size);
    void start();
    void stop();

private:
    std::string host;
    int port;
    std::string root_dir;
    std::string cgi_bin_dir;
    std::string upload_dir;
    int client_timeout;
    int buffer_size;
    int server_socket;
    ThreadPool thread_pool;
    void setup_socket();
    void handle_client(int client_socket, const std::string& client_ip);
    Response handle_get_request(const Request& request, const std::string& client_ip);
    Response handle_post_request(const Request& request, const std::string& client_ip);
    std::string execute_cgi(const std::string& path, const std::string& query_string, const std::string& post_data, const std::string& request_method, const std::string& content_type, int content_length, const std::string& client_ip);
    void send_service_unavailable(int client_socket);
    void log_event(const std::string& message, int log_level);
    void cleanup();
    void remove_uploaded_files();

    static void signal_handler(int signal);
};

#endif // SERVER_H

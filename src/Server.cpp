#include <iostream>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <dirent.h>
#include <sys/wait.h>

#include "Server.h"
#include "FileUtils.h"
#include "GuidGenerator.h"
#include "MultipartParser.h"
#include "Request.h"
#include "Response.h"
#include "RawResponse.h"


Server* global_server_instance = nullptr;

Server::Server(const std::string& host, int port,
               const std::string& root_dir, const std::string& cgi_bin_dir, const std::string& upload_dir,
               size_t num_threads, int client_timeout, int buffer_size) :
               host(host), port(port),
               root_dir(root_dir), cgi_bin_dir(cgi_bin_dir), upload_dir(upload_dir),
               thread_pool(num_threads), client_timeout(client_timeout), buffer_size(buffer_size) {
    openlog("tww", LOG_PID | LOG_CONS, LOG_USER);
    global_server_instance = this;
    signal(SIGINT, Server::signal_handler);
}

void Server::setup_socket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        log_event("Error creating socket", LOG_ERR);
        exit(1);
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(host.c_str());
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        log_event("Error binding socket to " + host + ":" + std::to_string(port), LOG_ERR);
        exit(1);
    }

    listen(server_socket, 5);
    log_event("Server running on " + host + ":" + std::to_string(port), LOG_INFO);
}

void Server::start() {
    setup_socket();
    while (true) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {
            log_event("Error accepting connection", LOG_ERR);
            continue;
        }

        // Get client IP address
        std::string client_ip = inet_ntoa(client_address.sin_addr);
        log_event("Connection from " + client_ip, LOG_INFO);

        // Check if the thread pool is full
        if (thread_pool.is_full()) {
            send_service_unavailable(client_socket);
            close(client_socket);
            continue;
        }

        // Enqueue the task to handle the client
        thread_pool.enqueue([this, client_socket, client_ip] {
            handle_client(client_socket, client_ip);
        });
    }
}

void Server::stop() {
    log_event("Shutting down server...", LOG_INFO);
    cleanup();
    log_event("Server shutdown complete.", LOG_INFO);
}

void Server::signal_handler(int signal) {
    if (signal == SIGINT) {
        if (global_server_instance != nullptr) {
            global_server_instance->stop();
            exit(0);
        }
    }
}

void Server::handle_client(int client_socket, const std::string& client_ip) {
    Request request(client_socket, buffer_size);

    std::string request_type = request.get_type();
    log_event("Request: " + request_type + " " + request.get_path() + " from " + client_ip, LOG_INFO);

    if (request_type == "GET") {
        auto response = handle_get_request(request, client_ip);
        std::string built_response = response->build_response();

        send(client_socket, built_response.c_str(), built_response.size(), 0);
        log_event("Sent " + std::to_string(built_response.size()) + " bytes to " + client_ip, LOG_INFO);
        close(client_socket);
    } else if (request_type == "POST") {
        auto response = handle_post_request(request, client_ip);
        std::string built_response = response->build_response();

        send(client_socket, built_response.c_str(), built_response.size(), 0);
        log_event("Sent " + std::to_string(built_response.size()) + " bytes to " + client_ip, LOG_INFO);
        close(client_socket);
    } else {
        auto response = std::make_unique<Response>();
        response->set_status_code(405);
        response->set_body("Method Not Allowed");
        std::string built_response = response->build_response();

        send(client_socket, built_response.c_str(), built_response.size(), 0);
        log_event("Sent " + std::to_string(built_response.size()) + " bytes to " + client_ip, LOG_INFO);
        close(client_socket);
    }
}

std::unique_ptr<ResponseBase> Server::handle_get_request(const Request& request, const std::string& client_ip) {
    std::string path = request.get_path();

    if (path.find("/cgi-bin/") == 0) {
        std::string query_string = request.get_query_string();

        // get the name of the cgi script after /cgi-bin/
        std::string script_name = path.substr(8);
        std::string cgi_script_path = cgi_bin_dir + script_name;

        std::string cgi_output = execute_cgi(cgi_script_path, query_string, "", "GET", "", 0, client_ip);

        auto response = std::make_unique<RawResponse>();
        response->set_raw_response(cgi_output);
        return response;
    } else {
        std::string content = FileUtils::read_file(root_dir + path);
        auto response = std::make_unique<Response>();

        if (content.empty()) {
            response->set_status_code(404);
            response->set_body("Not Found");
            log_event("File not found: " + path + " requested by " + client_ip, LOG_WARNING);
        } else {
            std::string extension = FileUtils::get_file_extension(path);
            std::string mime_type = FileUtils::get_mime_type(extension);
            response->set_content_type(mime_type);
            response->set_body(content);
            log_event("File served: " + path + " to " + client_ip + " with MIME type: " + mime_type, LOG_INFO);
        }

        return response;
    }
}

std::unique_ptr<ResponseBase> Server::handle_post_request(const Request& request, const std::string& client_ip) {
    std::string path = request.get_path();
    std::string content_type = request.get_header("Content-Type");

    if (path.find("/cgi-bin/") == 0) {
        int content_length = std::stoi(request.get_header("Content-Length"));
        std::string post_data = request.get_body();

        // get the name of the cgi script after /cgi-bin/
        std::string script_name = path.substr(8);
        std::string cgi_script_path = cgi_bin_dir + script_name;

        std::string cgi_output = execute_cgi(cgi_script_path, "", post_data, "POST", content_type, content_length, client_ip);

        auto response = std::make_unique<RawResponse>();
        response->set_raw_response(cgi_output);
        return response;
    } else if (content_type.find("multipart/form-data") != std::string::npos) {
        // Handle file uploads
        size_t boundary_pos = content_type.find("boundary=");
        std::string boundary = content_type.substr(boundary_pos + 9);

        MultipartParser parser(boundary);
        parser.parse(request.get_body());

        auto files = parser.get_files();
        for (const auto& file : files) {
            std::string guid = GuidGenerator::generate();
            FileUtils::save_file(guid, file.second);
            log_event("File uploaded by " + client_ip + " saved as " + guid, LOG_INFO);
        }

        auto response = std::make_unique<Response>();
        response->set_status_code(200);
        response->set_body("File(s) uploaded successfully");
        return response;
    } else {
        auto response = std::make_unique<Response>();
        response->set_status_code(400);
        response->set_body("Bad Request");
        return response;
    }
}

std::string Server::execute_cgi(const std::string& path, const std::string& query_string, const std::string& post_data, const std::string& request_method, const std::string& content_type, int content_length, const std::string& client_ip) {
    // Check if the CGI script exists and is executable
    if (!FileUtils::file_exists_and_is_executable(path)) {
        log_event("CGI script not found or not executable: " + path, LOG_ERR);
        Response response;
        response.set_status_code(500);
        response.set_body("Internal Server Error");
        return response.build_response();
    }

    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        log_event("Failed to create pipes", LOG_ERR);
        Response response;
        response.set_status_code(500);
        response.set_body("Internal Server Error");
        return response.build_response();
    }

    pid_t pid = fork();
    if (pid == -1) {
        log_event("Failed to fork CGI process", LOG_ERR);
        Response response;
        response.set_status_code(500);
        response.set_body("Internal Server Error");
        return response.build_response();
    }

    if (pid == 0) { // Child process
        close(pipe_in[1]);  // Close unused write end
        close(pipe_out[0]); // Close unused read end

        dup2(pipe_in[0], STDIN_FILENO);  // Redirect stdin to pipe_in
        dup2(pipe_out[1], STDOUT_FILENO); // Redirect stdout to pipe_out
        dup2(pipe_out[1], STDERR_FILENO); // Redirect stderr to pipe_out

        close(pipe_in[0]);
        close(pipe_out[1]);

        // Set CGI environment variables
        setenv("REQUEST_METHOD", request_method.c_str(), 1);
        setenv("QUERY_STRING", query_string.c_str(), 1);
        setenv("SCRIPT_NAME", path.c_str(), 1);
        setenv("CONTENT_LENGTH", std::to_string(content_length).c_str(), 1);
        setenv("CONTENT_TYPE", content_type.c_str(), 1);
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("REMOTE_ADDR", client_ip.c_str(), 1);

        char hostname[1024];
        gethostname(hostname, sizeof(hostname));
        setenv("SERVER_NAME", hostname, 1);
        setenv("SERVER_PORT", std::to_string(port).c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);

        // Execute the CGI script
        execl(path.c_str(), path.c_str(), NULL);

        // If execl fails
        log_event("Failed to execute CGI script: " + path, LOG_ERR);
        _exit(1);
    } else { // Parent process
        close(pipe_in[0]);  // Close unused read end
        close(pipe_out[1]); // Close unused write end

        // Set the alarm to trigger after x seconds
        alarm(client_timeout);

        write(pipe_in[1], post_data.c_str(), post_data.size());
        close(pipe_in[1]); // Done writing

        // Read output from CGI script
        char buffer[buffer_size];
        std::string output;
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            output += buffer;
        }
        close(pipe_out[0]);

        // Cancel the alarm after reading the output
        alarm(0);

        int status;
        waitpid(pid, &status, 0);  // Wait for the child process to finish

        return output;
    }
}


void Server::send_service_unavailable(int client_socket) {
    Response response;
    response.set_status_code(503);
    response.set_body("Service Unavailable");
    std::string built_response = response.build_response();
    send(client_socket, built_response.c_str(), built_response.size(), 0);
    log_event("Service unavailable, too many connections", LOG_WARNING);
}

void Server::log_event(const std::string& message, int log_level) {
    syslog(log_level, "%s", message.c_str());
    std::cout << message << std::endl;
}

void Server::cleanup() {
    close(server_socket);
    thread_pool.~ThreadPool();
    remove_uploaded_files();
}

void Server::remove_uploaded_files() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(upload_dir.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string file_name = ent->d_name;
            if (file_name != "." && file_name != "..") {
                std::string file_path = std::string(upload_dir) + file_name;
                if (remove(file_path.c_str()) == 0) {
                    log_event("Deleted uploaded file: " + file_path, LOG_INFO);
                }
            }
        }
        closedir(dir);
    }
}

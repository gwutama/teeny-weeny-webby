#ifndef REQUEST_H
#define REQUEST_H

#include <string>

class Request {
public:
    Request(int client_socket, int buffer_size);
    std::string get_type() const;
    std::string get_path() const;
    std::string get_query_string() const;
    std::string get_header(const std::string& header_name) const;
    std::string get_body() const;

private:
    std::string raw_request;
    std::string type;
    std::string path;
    std::string query_string;
    std::string body;

    void parse_request();
};

#endif // REQUEST_H

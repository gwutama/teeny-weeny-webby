#include "Request.h"

#include <unistd.h>
#include <sstream>
#include <cstring>

Request::Request(int client_socket, int buffer_size) {
    char buffer[buffer_size];
    std::memset(buffer, 0, buffer_size);
    read(client_socket, buffer, buffer_size);
    raw_request = std::string(buffer);
    parse_request();
}

void Request::parse_request() {
    std::istringstream ss(raw_request);
    ss >> type >> path;

    size_t pos = path.find("?");
    if (pos != std::string::npos) {
        query_string = path.substr(pos + 1);
        path = path.substr(0, pos);
    }

    if (type == "POST") {
        size_t content_length_pos = raw_request.find("Content-Length:");
        if (content_length_pos != std::string::npos) {
            int content_length = 0;
            std::istringstream ss(raw_request.substr(content_length_pos));
            std::string header;
            ss >> header >> content_length;
            body = raw_request.substr(raw_request.length() - content_length, content_length);
        }
    }
}

std::string Request::get_type() const {
    return type;
}

std::string Request::get_path() const {
    return path;
}

std::string Request::get_query_string() const {
    return query_string;
}

std::string Request::get_header(const std::string& header_name) const {
    std::string header_value;
    size_t pos = raw_request.find(header_name);
    if (pos != std::string::npos) {
        size_t start = pos + header_name.length() + 2;
        size_t end = raw_request.find("\r\n", start);
        header_value = raw_request.substr(start, end - start);
    }
    return header_value;
}

std::string Request::get_body() const {
    return body;
}

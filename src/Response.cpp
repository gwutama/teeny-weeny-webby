#include "Response.h"

#include <sstream>

Response::Response() : status_code(200) {}

void Response::set_status_code(int code) {
    status_code = code;
}

void Response::set_content_type(const std::string& type) {
    headers["Content-Type"] = type;
}

void Response::set_body(const std::string& body_content) {
    body = body_content;
    headers["Content-Length"] = std::to_string(body.size());
}

void Response::add_header(const std::string& header_name, const std::string& header_value) {
    headers[header_name] = header_value;
}

std::string Response::get_status_message(int code) const {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

std::string Response::build_response() const {
    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 " << status_code << " " << get_status_message(status_code) << "\r\n";
    for (const auto& header : headers) {
        response_stream << header.first << ": " << header.second << "\r\n";
    }
    response_stream << "\r\n" << body;
    return response_stream.str();
}

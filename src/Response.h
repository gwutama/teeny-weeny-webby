#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <map>

class Response {
public:
    Response();
    void set_status_code(int code);
    void set_content_type(const std::string& type);
    void set_body(const std::string& body);
    void add_header(const std::string& header_name, const std::string& header_value);
    std::string build_response() const;

    int get_status_code() const { return status_code; }

private:
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string get_status_message(int code) const;
};

#endif // RESPONSE_H

#include "MultipartParser.h"

#include <sstream>

MultipartParser::MultipartParser(const std::string& boundary) : boundary(boundary) {}

void MultipartParser::parse(const std::string& body) {
    size_t pos = 0;
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = delimiter + "--";

    while ((pos = body.find(delimiter)) != std::string::npos) {
        size_t end_pos = body.find(delimiter, pos + delimiter.length());
        std::string part = body.substr(pos + delimiter.length(), end_pos - pos - delimiter.length());

        // Extract headers and content
        size_t header_end = part.find("\r\n\r\n");
        std::string headers = part.substr(0, header_end);
        std::string content = part.substr(header_end + 4);

        // Extract the filename from the headers
        size_t filename_pos = headers.find("filename=\"");
        if (filename_pos != std::string::npos) {
            std::string filename = extract_filename(headers);

            // Convert content to a vector of unsigned char (byte array)
            std::vector<unsigned char> file_content(content.begin(), content.end());
            files.push_back({filename, file_content});
        }

        pos = end_pos;
    }
}

std::string MultipartParser::extract_filename(const std::string& disposition) {
    size_t filename_start = disposition.find("filename=\"") + 10;
    size_t filename_end = disposition.find("\"", filename_start);
    return disposition.substr(filename_start, filename_end - filename_start);
}

std::string MultipartParser::extract_content_type(const std::string& headers) {
    size_t content_type_pos = headers.find("Content-Type: ");
    if (content_type_pos != std::string::npos) {
        size_t start = content_type_pos + 14;
        size_t end = headers.find("\r\n", start);
        return headers.substr(start, end - start);
    }
    return "application/octet-stream";
}

std::vector<std::pair<std::string, std::vector<unsigned char>>> MultipartParser::get_files() const {
    return files;
}

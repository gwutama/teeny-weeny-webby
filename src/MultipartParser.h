#ifndef MULTIPARTPARSER_H
#define MULTIPARTPARSER_H

#include <string>
#include <vector>
#include <utility>

class MultipartParser {
public:
    MultipartParser(const std::string& boundary);
    void parse(const std::string& body);

    std::vector<std::pair<std::string, std::vector<unsigned char>>> get_files() const;

private:
    std::string boundary;
    std::vector<std::pair<std::string, std::vector<unsigned char>>> files;

    std::string extract_filename(const std::string& disposition);
    std::string extract_content_type(const std::string& headers);
};

#endif // MULTIPARTPARSER_H

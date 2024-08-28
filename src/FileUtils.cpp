#include "FileUtils.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unistd.h>

std::string FileUtils::get_file_extension(const std::string& path) {
    size_t dot_pos = path.find_last_of(".");
    if (dot_pos == std::string::npos) return "";
    return path.substr(dot_pos + 1);
}

std::string FileUtils::get_mime_type(const std::string& extension) {
    static const std::unordered_map<std::string, std::string> mime_types = {
            {"html", "text/html"}, {"css", "text/css"}, {"js", "application/javascript"},
            {"png", "image/png"}, {"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"},
            {"gif", "image/gif"}, {"bmp", "image/bmp"}, {"ico", "image/x-icon"},
            {"json", "application/json"}, {"xml", "application/xml"}, {"pdf", "application/pdf"},
            {"zip", "application/zip"}, {"mp3", "audio/mpeg"}, {"mp4", "video/mp4"},
            {"ogg", "application/ogg"}, {"wav", "audio/wav"}, {"webm", "video/webm"},
            {"txt", "text/plain"}, {"csv", "text/csv"}
    };
    auto it = mime_types.find(extension);
    return it != mime_types.end() ? it->second : "application/octet-stream";
}

std::string FileUtils::read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void FileUtils::save_file(const std::string& filename, const std::vector<unsigned char>& content) {
    std::ofstream file(filename, std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(content.data()), content.size());
        file.close();
    }
}

bool FileUtils::file_exists_and_is_executable(const std::string& path) {
    return access(path.c_str(), F_OK | X_OK) == 0;
}
#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
#include <vector>

class FileUtils {
public:
    static std::string get_file_extension(const std::string& path);
    static std::string get_mime_type(const std::string& extension);
    static std::string read_file(const std::string& path);
    static void save_file(const std::string& filename, const std::vector<unsigned char>& content);
    static bool file_exists_and_is_executable(const std::string& path);
};

#endif // FILEUTILS_H

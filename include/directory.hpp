#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include<string>
#include<filesystem>
#include<fstream>
#include<vector>

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

std::vector<std::filesystem::path> lsobj(const char *path)
{
    std::vector<std::filesystem::path> paths; 
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
        paths.push_back(entry.path());
    return paths;
}

void mkdir(const char *path)
{
    std::filesystem::create_directories(path);
    return;
}

#endif
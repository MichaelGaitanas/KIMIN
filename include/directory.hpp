#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include<string>
#include<filesystem>
#include<fstream>
#include<vector>

str os_name()
{
    #if __APPLE__ || __MACH__
        return "Mac OSX";
    #elif __linux__ || __unix || __unix__ || __FreeBSD__
        return "Linux||Unix||FreeBSD";
    #elif _WIN32 || _WIN64
        return "Windows";
    #else
        return "Unknown OS";
    #endif
}

//This function receives as input a path to a directory and as a result it returns a vector of paths, corresponding
//to all the files (even child directories) found inside 'path/'
std::vector<std::filesystem::path> lsobj(const char *path)
{
    std::vector<std::filesystem::path> paths; 
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
        paths.push_back(entry.path());
    return paths;
}

//Create a directory.
void mkdir(const char *path)
{
    std::filesystem::create_directories(path);
    return;
}

#endif
#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include<string>
#include<filesystem>
#include<fstream>
#include<vector>

#include"typedef.hpp"

str os_name()
{
    //Remember #ifdef and #endif instead of #if and #elif.

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

/*
Not tested yet. Don't trust it.
str compiler_name()
{
    #if _MSVC_LANG
        return "MSVC";
    #elif __MINGW32__
        return "MINGW 32";
    #elif __MINGW64__
        return "MINGW 64";
    #elif __GNUC__
        return "GNU C";
    #elif __clang_version__
        return "CLANG";
    #else
        return "Unknown compiler";
    #endif
}
*/

//This function receives as input a path to a directory and as a result it returns a vector of paths, corresponding
//to all the files (even child directories) found inside 'path/'
std::vector<std::filesystem::path> lsfiles(const char *path)
{
    std::vector<std::filesystem::path> paths; 
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
        paths.push_back(entry.path());
    return paths;
}

#endif
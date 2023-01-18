#include<iostream>
#include<string>

std::string getOsName()
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

int main()
{
    std::string os = getOsName();
    std::cout << os << "\n";
    return 0;
}
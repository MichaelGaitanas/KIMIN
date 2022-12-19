#include<iostream>
#include<fstream>
#include<filesystem>

std::ifstream::pos_type filesize(const char* path)
{
    std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

/*
void mkdir(const char *path)
{
    std::filesystem::create_directories(path);
    return;
}
*/

int main()
{
    bool madedir = std::filesystem::create_directory("dir");
    printf("%d\n",madedir);
    printf("%s\n",std::filesystem::current_path(std::filesystem::temp_directory_path()));

    return 0;
}
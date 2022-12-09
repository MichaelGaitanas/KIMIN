#include<cmath>
#include <fstream>

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

int main()
{
    const char *sz = std::to_string(filesize("../resources/obj/v/didymain2019_1229.obj")).c_str();
    printf("%s\n",sz);

	return 0;
}
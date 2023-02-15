#include<iostream>
#include<fstream>
#include<filesystem>
#include<string>

int main()
{
    bool madedir = std::filesystem::create_directory("simulations");

    char simname[51] = "sim1";
    
    
    //std::string simnname_copy = simname;
    //std::ofstream out("simulations/" + simnname_copy + ".json");

    std::ofstream out("simulations/" + std::string(simname) + ".json");
    
    
    out.close();

    return 0;
}
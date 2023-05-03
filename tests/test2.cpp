#include<iostream>
#include<vector>

bool func(int num, const bool temp)
{
    if (num == 1)
        return true;
    else
        return false;
}

int main()
{
    std::vector<bool> cart = {false,false,false};
    std:: cout << cart[0] << "  "  << cart[1] << "  " << cart[2] << "\n";
    cart[0] = func(1, cart[0]);
    std:: cout << cart[0] << "  "  << cart[1] << "  " << cart[2] << "\n";

    return 0;
}
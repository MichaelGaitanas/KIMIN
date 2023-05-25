#include<iostream>

class Properties
{
public:
    double p;
    Properties() : p(1.23) { };
};

class Integrator : public Properties
{
public:
    int i;
    Integrator(const Properties &properties) : Properties(properties)
    {
        p = 36.0;
        i = 37;
    }
};

class Solution : public Integrator
{
public:
    double s;
    Solution(const Integrator &integrator) : Integrator(integrator)
    {
        p = 1.1;
        i = 2;
        s = 3.0;
    }
};

int main()
{
    Properties props;
    Integrator integ(props);
    Solution sol(integ);

    std::cout << sol.p << "  " << sol.i << "  " << sol.s << "\n";

    return 0;
}
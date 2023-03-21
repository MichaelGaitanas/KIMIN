#ifndef III_HPP
#define III_HPP

#include<cstdio>

#include"ggg.hpp"

class Solution
{
public:
    long xxx;
};

class Integrator : public Properties
{
public:
    double x,y,z;
    Integrator(Properties &properties)
    {
        x = properties.x;
        y = properties.y;
        z = 5.0;

        printf("%lf\n%lf\n%lf\n",x,y,z);
    }
};

#endif

#ifndef GGG_HPP
#define GGG_HPP

#include<cstdio>

class Properties
{
public:
    double x,y;
    int a,b;
    Properties() : x(1.0), y(2.0), a(3), b(4) { }
};

#include"iii.hpp"

class GUI
{
public:
    Properties properties;
    void on_click()
    {
        Integrator integrator(properties);
        Solution solution;
        return;
    }
};

#endif
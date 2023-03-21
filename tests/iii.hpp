#ifndef III_HPP
#define III_HPP

#include"ggg.hpp"

class Integrator
{
public:
    double x,y,z;
    Integrator(Properties &properties)// : x(properties.x),
                                            //  y(properties.y),
                                            //   z(5.0)
    {
        x = properties.x;
        y = properties.y;
        z = 5.0;
        //printf("x = %lf, y = %lf z = %lf\n",x,y,z);
    }
};

#endif
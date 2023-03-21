#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include"typedef.hpp"

class Solution
{
public:

    dvec t;

    dvec x,y,z,d;
    dvec vx,vy,vz,v;
    dvec a,e,i,raan,w,M;

    dvec roll1,pitch1,yaw1;
    dvec w1ix,w1iy,w1iz;
    dvec w1bx,w1by,w1bz;

    dvec roll2,pitch2,yaw2;
    dvec w2ix,w2iy,w2iz;
    dvec w2bx,w2by,w2bz;

    dvec ener, ener_rel_err;
    dvec mom, mom_rel_err;
};

#endif
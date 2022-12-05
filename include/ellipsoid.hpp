#ifndef ELLIPSOID_HPP
#define ELLIPSOID_HPP

#include<cmath>
#include<cstdio>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Moment of inertia matrix of a triaxial ellipsoid in its principal axes.
dmat3 ell_inertia(const double M, const dvec3 &semiaxes)
{
    double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    double Ix = M*(b*b + c*c)/5.0;
    double Iy = M*(a*a + c*c)/5.0;
    double Iz = M*(a*a + b*b)/5.0;
    return {{{Ix,0.0,0.0},
             {0.0,Iy,0.0},
             {0.0,0.0,Iz}}};
}

//Inertial integrals tensor of a triaxial ellipsoid of order 2.
dtens ell_integrals_ord2(const double M, const dvec3 &semiaxes)
{
    double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];

    //initialize the tensor with zeros
    dtens J(3, dmat(3, dvec(3, 0.0) ) );

    //and then fill only the true non zero terms
    J[0][0][0] = M;
    J[2][0][0] = M*a*a/5.0; //Jxx
    J[0][2][0] = M*b*b/5.0; //Jyy
    J[0][0][2] = M*c*c/5.0; //Jzz

    return J;
}

//Brillouin radius of a triaxial ellipsoid (greatest semiaxis).
double ell_brillouin(const dvec3 &semiaxes)
{
    double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    if (a >= b && a >= c)
        return a;
    else if (b >= a && b >= c)
        return b;
    return c;
}

#endif
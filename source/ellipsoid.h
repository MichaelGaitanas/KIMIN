/* Here I have some functions regarding some dynamical characteristics of an ellipsoid. */

#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include<cmath>
#include<cstdio>

#include"constants.h"
#include"typedef.h"
#include"linalg.h"

//Moment of inertia matrix of a triaxial ellipsoid in its principal axes.
dmat3 ell_inertia(const double M, const dvec3 &semiaxes)
{
    const double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    const double Ixx = M*(b*b + c*c)/5.0;
    const double Iyy = M*(a*a + c*c)/5.0;
    const double Izz = M*(a*a + b*b)/5.0;
    return {{{Ixx,0.0,0.0},
             {0.0,Iyy,0.0},
             {0.0,0.0,Izz}}};
}

//Non normalized inertial integral tensor of arbitrary order a triaxial ellipsoid.
dtens ell_integrals(const double M, const dvec3 &semiaxes, const int ord)
{
    const double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0))); //Initialize the tensor with zeros.
    for (int i = 0; i < ord + 1; ++i)
        for (int j = 0; j < ord + 1; ++j)
            for (int k = 0; k < ord + 1; ++k)
                if (i%2 == 0 && j%2 == 0 && k%2 == 0 && i + j + k <= ord) //Otherwise the triple integral either yields zero, or does not appear as a principal axes integral in the potential/force/torque expressions.
                    J[i][j][k] = (3.0*M/(4.0*PI))*pow(a,i)*pow(b,j)*pow(c,k)*std::tgamma(0.5*(i+1))*std::tgamma(0.5*(j+1))*std::tgamma(0.5*(k+1))/std::tgamma(0.5*(i+j+k+5));
    return J;
}

//Brillouin radius of a triaxial ellipsoid (greatest semiaxis).
double ell_brillouin(const dvec3 &semiaxes)
{
    const double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    if (a >= b && a >= c)
        return a;
    else if (b >= a && b >= c)
        return b;
    return c;
}

#endif
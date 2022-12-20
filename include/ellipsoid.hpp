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

//Non normalized inertial integral tensor of arbitrary order a triaxial ellipsoid with constant density.
dtens ell_integrals(const double M, const dvec3 &semiaxes, const int ord)
{
    double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //initialize the tensor with zeros
    for (int i = 0; i < ord + 1; ++i)
        for (int j = 0; j < ord + 1; ++j)
            for (int k = 0; k < ord + 1; ++k)
                if (i%2 == 0 && j%2 == 0 && k%2 == 0) //otherwise the triple integral yields zero
                    J[i][j][k] = (3.0*M/(4.0*pi))*pow(a,i)*pow(b,j)*pow(c,k)*std::tgamma(0.5*(i+1))*std::tgamma(0.5*(j+1))*std::tgamma(0.5*(k+1))/std::tgamma(0.5*(i+j+k+5));
    return J;
}

//Non normalized inertial integral tensor of a triaxial ellipsoid of order 2.
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

//Fill an ellipsoid with semiaxes (a,b,c) with mascons assuming a 'grid_reso' Cartesian grid resolution.
dmatnx3 fill_ell_with_masc(const dvec3 &semiaxes, const ivec3 &grid_reso)
{
    double a = semiaxes[0];
    double b = semiaxes[1];
    double c = semiaxes[2];

    dmatnx3 masc;
    for (int i = 0; i < grid_reso[0]; ++i)
    {
        double x = -a + 2.0*i*a/((double)grid_reso[0] - 1.0);
        for (int j = 0; j < grid_reso[1]; ++j)
        {
            double y = -b + 2.0*j*b/((double)grid_reso[1] - 1.0);
            for (int k = 0; k < grid_reso[2]; ++k)
            {
                double z = -c + 2.0*k*c/((double)grid_reso[2] - 1.0);
                if ( x*x/(a*a) + y*y/(b*b) + z*z/(c*c) < 1.0 + machine_zero ) //equation of ellipsoid
                {
                    masc.push_back({x,y,z});
                }
            }
        }
    }

    return masc;
}

#endif
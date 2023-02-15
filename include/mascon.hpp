#ifndef MASCON_HPP
#define MASCON_HPP

#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"
#include"conversion.hpp"
#include"rigidbody.hpp"

//Center of mass of a mascon distribution with constant density.
dvec3 masc_com(const dmatnx3 &masc)
{
    dvec3 com = {0.0,0.0,0.0};
    for (int i = 0; i < masc.size(); ++i)
    {
        com = com + masc[i];
    }

    return com/masc.size();
}

//Farthest mascon distance with respect to the local coordinate system.
double masc_farthest(const dmatnx3 &masc)
{
    double farthest = length(masc[0]);
    for (int i = 1; i < masc.size(); ++i)
    {
        double dist = length(masc[i]);
        if (dist > farthest)
            farthest = dist;
    }
    return farthest;
}

//Nearest mascon distance with respect to the local coordinate system.
double masc_nearest(const dmatnx3 &masc)
{
    double nearest = length(masc[0]);
    for (int i = 1; i < masc.size(); ++i)
    {
        double dist = length(masc[i]);
        if (dist < nearest)
            nearest = dist;
    }
    return nearest;
}

//Shift the center of mass of a mascon distribution with constant density, so that it coincides with O(0,0,0).
void correct_masc_com(dmatnx3 &masc)
{
    dvec3 com = masc_com(masc);
    for (int i = 0; i < masc.size(); ++i)
    {
        masc[i] = masc[i] - com;
    }

    return;
}

//Moment of inertia of a mascon distribution with constant density.
dmat3 masc_inertia(const double M, const dmatnx3 &masc)
{
    double m = (double)M/masc.size(); //mass of each mascon

    double Ixx,Iyy,Izz,Ixy,Ixz,Iyz;
    Ixx = Iyy = Izz = Ixy = Ixz = Iyz = 0.0;

    for (int i = 0; i < masc.size(); ++i)
    {
        Ixx += ( pow(masc[i][1], 2) + pow(masc[i][2], 2) ); //y[i]^2 + z[i]^2
        Iyy += ( pow(masc[i][0], 2) + pow(masc[i][2], 2) ); //x[i]^2 + z[i]^2
        Izz += ( pow(masc[i][0], 2) + pow(masc[i][1], 2) ); //x[i]^2 + y[i]^2
        Ixy -= masc[i][0]*masc[i][1]; //-x[i]*y[i]
        Ixz -= masc[i][0]*masc[i][2]; //-x[i]*z[i]
        Iyz -= masc[i][1]*masc[i][2]; //-y[i]*z[i]
    }

    //Ixy = Iyx, Ixz = Izx, Iyz = Izy by definition
    return {{{m*Ixx, m*Ixy, m*Ixz},
             {m*Ixy, m*Iyy, m*Iyz},
             {m*Ixz, m*Iyz, m*Izz}}};
}

//Align the principal axes of a mascon distribution with constant density, so that they coincide with the basis {{1,0,0}, {0,1,0}, {0,0,1}}.
void correct_masc_inertia(const double M, dmatnx3 &masc)
{
    dmat3 I = masc_inertia(M, masc);
    dmat3 eigvecs = inertia_eigvecs(I); //3 3x1 real and normalized vectors that form a right handed Cartesian basis

    //instantiate the (unit) coordinate system
    dvec3 xaxis = {1.0,0.0,0.0};
    dvec3 yaxis = {0.0,1.0,0.0};
    dvec3 zaxis = {0.0,0.0,1.0};

    dmat3 rot; //direction cosine matrix
    rot[0][0] = dot(xaxis,eigvecs[0]);
    rot[0][1] = dot(xaxis,eigvecs[1]);
    rot[0][2] = dot(xaxis,eigvecs[2]);
    rot[1][0] = dot(yaxis,eigvecs[0]);
    rot[1][1] = dot(yaxis,eigvecs[1]);
    rot[1][2] = dot(yaxis,eigvecs[2]);
    rot[2][0] = dot(zaxis,eigvecs[0]);
    rot[2][1] = dot(zaxis,eigvecs[1]);
    rot[2][2] = dot(zaxis,eigvecs[2]);
    rot = transpose(rot);

    //multiply the mascon vectors with the rotation matrix
    for (int i = 0; i < masc.size(); ++i)
    {
        masc[i] = dot(rot, masc[i]);
    }

    //now the masc inertia matrix is correct

    return;
}

//Non normalized inertial integral tensor of arbitrary order of a mascon distribution with constant density.
dtens masc_integrals(const double M, const dmatnx3 &masc, const int ord)
{
    double m = (double)M/masc.size(); //mass of each mascon
    dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //initialize the tensor with zeros
    for (int i = 0; i < ord + 1; ++i)
        for (int j = 0; j < ord + 1; ++j)
            for (int k = 0; k < ord + 1; ++k)
                for (int n = 0; n < masc.size(); ++n)
                    J[i][j][k] += m*pow(masc[n][0], i)*pow(masc[n][1], j)*pow(masc[n][2], k); //m*(x[n]^i)*(y[n]^j)*(z[n]^k)
    return J;
}

#endif
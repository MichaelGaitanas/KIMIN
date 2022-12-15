#ifndef TORQUE_HPP
#define TORQUE_HPP

#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Mutual gravity gradient torque of 2 rigid bodies, assuming inertial integral expansion of order 2 approximation.
dvec3 mut_torque_integrals_ord2(const dvec3 &r, const dtens &J1, const dmat3 &A1, const double M2)
{
    double I1x = J1[0][2][0] + J1[0][0][2];
    double I1y = J1[2][0][0] + J1[0][0][2];
    double I1z = J1[2][0][0] + J1[0][2][0];

    dvec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    dvec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    dvec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double dV_dl1 = 3.0*G*I1x*M2*l1/(d*d*d);
    double dV_dm1 = 3.0*G*I1y*M2*m1/(d*d*d);
    double dV_dn1 = 3.0*G*I1z*M2*n1/(d*d*d);

    dvec3 dl1_da1, dm1_da2, dn1_da3;
    dl1_da1 = dm1_da2 = dn1_da3 = ru;

    dvec3 dV_da1 = dV_dl1*dl1_da1;
    dvec3 dV_da2 = dV_dm1*dm1_da2;
    dvec3 dV_da3 = dV_dn1*dn1_da3;

    //tau1
    return -cross(a1, dV_da1) - cross(a2, dV_da2) - cross(a3, dV_da3);
}

#endif
#ifndef FORCE_HPP
#define FORCE_HPP

#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Mutual force of 2 rigid bodies, assuming inertial integral expansion of order 2 approximation.
dvec3 mut_force_integrals_ord2(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
                                               const double M2, const dtens &J2, const dmat3 &A2)
{   
    double I1x = J1[0][2][0] + J1[0][0][2];
    double I1y = J1[2][0][0] + J1[0][0][2];
    double I1z = J1[2][0][0] + J1[0][2][0];

    double I2x = J2[0][2][0] + J2[0][0][2];
    double I2y = J2[2][0][0] + J2[0][0][2];
    double I2z = J2[2][0][0] + J2[0][2][0];

    dvec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    dvec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    dvec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    dvec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    dvec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    dvec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    //order 0 (Keplerian)
    double V0 = -G*M1*M2/d;

    //order 1
    //V1 = 0 (by default)

    //order 2
    double V2 = -(G*M2/(2.0*d*d*d))*( (1.0 - 3.0*l1*l1)*I1x + (1.0 - 3.0*m1*m1)*I1y + (1.0 - 3.0*n1*n1)*I1z ) +
                -(G*M1/(2.0*d*d*d))*( (1.0 - 3.0*l2*l2)*I2x + (1.0 - 3.0*m2*m2)*I2y + (1.0 - 3.0*n2*n2)*I2z );

    double dV_dd = -V0/d - 3*V2/d;
    dvec3 dd_dr = ru;

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d);
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d);
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d);
    dvec3 dl1_dr = (a1*d-r*l1)/(d*d);
    dvec3 dm1_dr = (a2*d-r*m1)/(d*d);
    dvec3 dn1_dr = (a3*d-r*n1)/(d*d);

    double dV_dl2 = 3*G*I2x*M1*l2/(d*d*d);
    double dV_dm2 = 3*G*I2y*M1*m2/(d*d*d);
    double dV_dn2 = 3*G*I2z*M1*n2/(d*d*d);
    dvec3 dl2_dr = (b1*d-r*l2)/(d*d);
    dvec3 dm2_dr = (b2*d-r*m2)/(d*d);
    dvec3 dn2_dr = (b3*d-r*n2)/(d*d);

    //-grad(V)
    return -(dV_dd*dd_dr + dV_dl1*dl1_dr + dV_dm1*dm1_dr + dV_dn1*dn1_dr +
                           dV_dl2*dl2_dr + dV_dm2*dm2_dr + dV_dn2*dn2_dr);
}

#endif

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

//Mutual gravity gradient torque of 2 rigid bodies, assuming inertial integral expansion of order 3 approximation.
dvec3 mut_torque_integrals_ord3(const dvec3 &r, const dtens &J1, const dmat3 &A1, const double M2)
{
    double J1xxx = J1[3][0][0];
    double J1yyy = J1[0][3][0];
    double J1zzz = J1[0][0][3];
    double J1xxy = J1[2][1][0];
    double J1xyy = J1[1][2][0];
    double J1xxz = J1[2][0][1];
    double J1xzz = J1[1][0][2];
    double J1yyz = J1[0][2][1];
    double J1yzz = J1[0][1][2];
    double J1xyz = J1[1][1][1];
    
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

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d) - G*M2*(10*J1xxx*l1*l1 + J1xxx*(5*l1*l1 - 3) + 30*J1xxy*l1*m1 + 30*J1xxz*l1*n1 + 3*J1xyy*(5*m1*m1 - 1) + 30*J1xyz*m1*n1 + 3*J1xzz*(5*n1*n1 - 1))/(2*d*d*d*d);
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d) - G*M2*(3*J1xxy*(5*l1*l1 - 1) + 30*J1xyy*l1*m1 + 30*J1xyz*l1*n1 + 10*J1yyy*m1*m1 + J1yyy*(5*m1*m1 - 3) + 30*J1yyz*m1*n1 + 3*J1yzz*(5*n1*n1 - 1))/(2*d*d*d*d);
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d) - G*M2*(3*J1xxz*(5*l1*l1 - 1) + 30*J1xyz*l1*m1 + 30*J1xzz*l1*n1 + 3*J1yyz*(5*m1*m1 - 1) + 30*J1yzz*m1*n1 + 10*J1zzz*n1*n1 + J1zzz*(5*n1*n1 - 3))/(2*d*d*d*d);

    dvec3 dl1_da1, dm1_da2, dn1_da3;
    dl1_da1 = dm1_da2 = dn1_da3 = ru;

    dvec3 dV_da1 = dV_dl1*dl1_da1;
    dvec3 dV_da2 = dV_dm1*dm1_da2;
    dvec3 dV_da3 = dV_dn1*dn1_da3;

    //tau1
    return -cross(a1, dV_da1) - cross(a2, dV_da2) - cross(a3, dV_da3);
}

//Mutual gravity gradient torque of 2 rigid bodies, assuming inertial integral expansion of order 4 approximation.
dvec3 mut_torque_integrals_ord4(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
                                                const double M2, const dtens &J2, const dmat3 &A2)
{
    double J1xx = J1[2][0][0];
    double J1yy = J1[0][2][0];
    double J1zz = J1[0][0][2];

    double J1xxx = J1[3][0][0];
    double J1yyy = J1[0][3][0];
    double J1zzz = J1[0][0][3];
    double J1xxy = J1[2][1][0];
    double J1xyy = J1[1][2][0];
    double J1xxz = J1[2][0][1];
    double J1xzz = J1[1][0][2];
    double J1yyz = J1[0][2][1];
    double J1yzz = J1[0][1][2];
    double J1xyz = J1[1][1][1];

    double J1xxxx = J1[4][0][0];
    double J1yyyy = J1[0][4][0];
    double J1zzzz = J1[0][0][4];
    double J1xxxy = J1[3][1][0];
    double J1xyyy = J1[1][3][0];
    double J1xxxz = J1[3][0][1];
    double J1xzzz = J1[1][0][3];
    double J1yyyz = J1[0][3][1];
    double J1yzzz = J1[0][1][3];
    double J1xxyy = J1[2][2][0];
    double J1xxzz = J1[2][0][2];
    double J1yyzz = J1[0][2][2];
    double J1xxyz = J1[2][1][1];
    double J1xyyz = J1[1][2][1];
    double J1xyzz = J1[1][1][2];
    
    double I1x = J1[0][2][0] + J1[0][0][2];
    double I1y = J1[2][0][0] + J1[0][0][2];
    double I1z = J1[2][0][0] + J1[0][2][0];

    /////////////////////////////////////////////////////////////////////////////////

    double J2xx = J2[2][0][0];
    double J2yy = J2[0][2][0];
    double J2zz = J2[0][0][2];

    dvec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    dvec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    dvec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    dvec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    dvec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    dvec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    dmat3 C = dot(transpose(A1), A2);
    double c11 = C[0][0], c12 = C[0][1], c13 = C[0][2];
    double c21 = C[1][0], c22 = C[1][1], c23 = C[1][2];
    double c31 = C[2][0], c32 = C[2][1], c33 = C[2][2];

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d) -
                    G*M2*(10*J1xxx*l1*l1 + J1xxx*(5*l1*l1 - 3) + 30*J1xxy*l1*m1 + 30*J1xxz*l1*n1 + 3*J1xyy*(5*m1*m1 - 1) + 30*J1xyz*m1*n1 + 3*J1xzz*(5*n1*n1 - 1))/(2*d*d*d*d) +
                    5*G*(3*J1xx*(J2xx*(2*c11*l2 - 7*l1*l2*l2 + l1) + J2yy*(2*c12*m2 - 7*l1*m2*m2 + l1) + J2zz*(2*c13*n2 - 7*l1*n2*n2 + l1)) - M2*(J1xxxx*l1*(7*l1*l1 - 3) + 14*J1xxxy*l1*l1*m1 + J1xxxy*m1*(7*l1*l1 - 3) + 14*J1xxxz*l1*l1*n1 + J1xxxz*n1*(7*l1*l1 - 3) + 3*J1xxyy*l1*(7*m1*m1 - 1) + 42*J1xxyz*l1*m1*n1 + 3*J1xxzz*l1*(7*n1*n1 - 1) + J1xyyy*m1*(7*m1*m1 - 3) + 3*J1xyyz*n1*(7*m1*m1 - 1) + 3*J1xyzz*m1*(7*n1*n1 - 1) + J1xzzz*n1*(7*n1*n1 - 3)))/(2*d*d*d*d*d);
    
    
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d) -
                    G*M2*(3*J1xxy*(5*l1*l1 - 1) + 30*J1xyy*l1*m1 + 30*J1xyz*l1*n1 + 10*J1yyy*m1*m1 + J1yyy*(5*m1*m1 - 3) + 30*J1yyz*m1*n1 + 3*J1yzz*(5*n1*n1 - 1))/(2*d*d*d*d) +
                    5*G*(3*J1yy*(J2xx*(2*c21*l2 - 7*l2*l2*m1 + m1) + J2yy*(2*c22*m2 - 7*m1*m2*m2 + m1) + J2zz*(2*c23*n2 - 7*m1*n2*n2 + m1)) - M2*(J1xxxy*l1*(7*l1*l1 - 3) + 3*J1xxyy*m1*(7*l1*l1 - 1) + 3*J1xxyz*n1*(7*l1*l1 - 1) + 14*J1xyyy*l1*m1*m1 + J1xyyy*l1*(7*m1*m1 - 3) + 42*J1xyyz*l1*m1*n1 + 3*J1xyzz*l1*(7*n1*n1 - 1) + J1yyyy*m1*(7*m1*m1 - 3) + 14*J1yyyz*m1*m1*n1 + J1yyyz*n1*(7*m1*m1 - 3) + 3*J1yyzz*m1*(7*n1*n1 - 1) + J1yzzz*n1*(7*n1*n1 - 3)))/(2*d*d*d*d*d);
    
    
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d) -
                    G*M2*(3*J1xxz*(5*l1*l1 - 1) + 30*J1xyz*l1*m1 + 30*J1xzz*l1*n1 + 3*J1yyz*(5*m1*m1 - 1) + 30*J1yzz*m1*n1 + 10*J1zzz*n1*n1 + J1zzz*(5*n1*n1 - 3))/(2*d*d*d*d) +
                    5*G*(3*J1zz*(J2xx*(2*c31*l2 - 7*l2*l2*n1 + n1) + J2yy*(2*c32*m2 - 7*m2*m2*n1 + n1) + J2zz*(2*c33*n2 - 7*n1*n2*n2 + n1)) - M2*(J1xxxz*l1*(7*l1*l1 - 3) + 3*J1xxyz*m1*(7*l1*l1 - 1) + 3*J1xxzz*n1*(7*l1*l1 - 1) + 3*J1xyyz*l1*(7*m1*m1 - 1) + 42*J1xyzz*l1*m1*n1 + 14*J1xzzz*l1*n1*n1 + J1xzzz*l1*(7*n1*n1 - 3) + J1yyyz*m1*(7*m1*m1 - 3) + 3*J1yyzz*n1*(7*m1*m1 - 1) + 14*J1yzzz*m1*n1*n1 + J1yzzz*m1*(7*n1*n1 - 3) + J1zzzz*n1*(7*n1*n1 - 3)))/(2*d*d*d*d*d);

    dvec3 dl1_da1, dm1_da2, dn1_da3;
    dl1_da1 = dm1_da2 = dn1_da3 = ru;

    double dV_dc11 = -G*J1xx*J2xx*(24*c11 - 120*l1*l2)/(8*d*d*d*d*d);
    double dV_dc12 = -G*J1xx*J2yy*(24*c12 - 120*l1*m2)/(8*d*d*d*d*d);
    double dV_dc13 = -G*J1xx*J2zz*(24*c13 - 120*l1*n2)/(8*d*d*d*d*d);

    double dV_dc21 = -G*J1yy*J2xx*(24*c21 - 120*l2*m1)/(8*d*d*d*d*d);
    double dV_dc22 = -G*J1yy*J2yy*(24*c22 - 120*m1*m2)/(8*d*d*d*d*d);
    double dV_dc23 = -G*J1yy*J2zz*(24*c23 - 120*m1*n2)/(8*d*d*d*d*d);

    double dV_dc31 = -G*J1zz*J2xx*(24*c31 - 120*l2*n1)/(8*d*d*d*d*d);
    double dV_dc32 = -G*J1zz*J2yy*(24*c32 - 120*m2*n1)/(8*d*d*d*d*d);
    double dV_dc33 = -G*J1zz*J2zz*(24*c33 - 120*n1*n2)/(8*d*d*d*d*d);

    dvec3 dc11_da1, dc21_da2, dc31_da3;
    dvec3 dc12_da1, dc22_da2, dc32_da3;
    dvec3 dc13_da1, dc23_da2, dc33_da3;

    dc11_da1 = dc21_da2 = dc31_da3 = b1;
    dc12_da1 = dc22_da2 = dc32_da3 = b2;
    dc13_da1 = dc23_da2 = dc33_da3 = b3;

    dvec3 dV_da1 = dV_dl1*dl1_da1 + dV_dc11*dc11_da1 + dV_dc12*dc12_da1 + dV_dc13*dc13_da1;
    dvec3 dV_da2 = dV_dm1*dm1_da2 + dV_dc21*dc21_da2 + dV_dc22*dc22_da2 + dV_dc23*dc23_da2;
    dvec3 dV_da3 = dV_dn1*dn1_da3 + dV_dc31*dc31_da3 + dV_dc32*dc32_da3 + dV_dc33*dc33_da3;

    //tau1
    return -cross(a1, dV_da1) - cross(a2, dV_da2) - cross(a3, dV_da3);
}

#endif
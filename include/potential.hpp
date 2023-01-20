#ifndef POTENTIAL_HPP
#define POTENTIAL_HPP

#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Mutual potential of 2 rigid bodies, assuming mascon distributions with constant densities.
double mut_pot_masc(const dvec3 &r, const double M1, const dmatnx3 &masc1, const dmat3 &A1,
                                    const double M2, const dmatnx3 &masc2, const dmat3 &A2)
{
    double sum = 0.0;
    for (int i = 0; i < masc1.size(); ++i)
    {
        dvec3 a1i = dot(A1, masc1[i]);
        for (int j = 0; j < masc2.size(); ++j)
        {
            dvec3 a2j = dot(A2, masc2[j]);
            dvec3 dij = r + a2j - a1i;
            sum += 1.0/length(dij);
        }
    }
    return -G*M1*M2*sum/(masc1.size()*masc2.size());
}

//Mutual potential of 2 rigid bodies, assuming inertial integral expansion of order 2 approximation.
double mut_pot_integrals_ord2(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
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

    //V
    return V0 + V2;
}

//Mutual potential of 2 rigid bodies, assuming inertial integral expansion of order 3 approximation.
double mut_pot_integrals_ord3(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
                                              const double M2, const dtens &J2, const dmat3 &A2)
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

    double J2xxx = J2[3][0][0];
    double J2yyy = J2[0][3][0];
    double J2zzz = J2[0][0][3];
    double J2xxy = J2[2][1][0];
    double J2xyy = J2[1][2][0];
    double J2xxz = J2[2][0][1];
    double J2xzz = J2[1][0][2];
    double J2yyz = J2[0][2][1];
    double J2yzz = J2[0][1][2];
    double J2xyz = J2[1][1][1];

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

    //order 3
    double V3 = -(G*M2/(2*d*d*d*d))*(   l1*(5*l1*l1 - 3)*J1xxx +   m1*(5*m1*m1 - 3)*J1yyy +   n1*(5*n1*n1 - 3)*J1zzz +
                                      3*m1*(5*l1*l1 - 1)*J1xxy + 3*l1*(5*m1*m1 - 1)*J1xyy + 3*n1*(5*l1*l1 - 1)*J1xxz +
                                      3*l1*(5*n1*n1 - 1)*J1xzz + 3*n1*(5*m1*m1 - 1)*J1yyz + 3*m1*(5*n1*n1 - 1)*J1yzz +
                                     30*l1*m1*n1*J1xyz ) +
                 (G*M1/(2*d*d*d*d))*(   l2*(5*l2*l2 - 3)*J2xxx +   m2*(5*m2*m2 - 3)*J2yyy +   n2*(5*n2*n2 - 3)*J2zzz +
                                      3*m2*(5*l2*l2 - 1)*J2xxy + 3*l2*(5*m2*m2 - 1)*J2xyy + 3*n2*(5*l2*l2 - 1)*J2xxz +
                                      3*l2*(5*n2*n2 - 1)*J2xzz + 3*n2*(5*m2*m2 - 1)*J2yyz + 3*m2*(5*n2*n2 - 1)*J2yzz +
                                     30*l2*m2*n2*J2xyz );

    //V
    return V0 + V2 + V3;
}

//Mutual potential of 2 rigid bodies, assuming inertial integral expansion of order 4 approximation.
double mut_pot_integrals_ord4(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
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

    double J2xxx = J2[3][0][0];
    double J2yyy = J2[0][3][0];
    double J2zzz = J2[0][0][3];
    double J2xxy = J2[2][1][0];
    double J2xyy = J2[1][2][0];
    double J2xxz = J2[2][0][1];
    double J2xzz = J2[1][0][2];
    double J2yyz = J2[0][2][1];
    double J2yzz = J2[0][1][2];
    double J2xyz = J2[1][1][1];

    double J2xxxx = J2[4][0][0];
    double J2yyyy = J2[0][4][0];
    double J2zzzz = J2[0][0][4];
    double J2xxxy = J2[3][1][0];
    double J2xyyy = J2[1][3][0];
    double J2xxxz = J2[3][0][1];
    double J2xzzz = J2[1][0][3];
    double J2yyyz = J2[0][3][1];
    double J2yzzz = J2[0][1][3];
    double J2xxyy = J2[2][2][0];
    double J2xxzz = J2[2][0][2];
    double J2yyzz = J2[0][2][2];
    double J2xxyz = J2[2][1][1];
    double J2xyyz = J2[1][2][1];
    double J2xyzz = J2[1][1][2];
    
    double I2x = J2[0][2][0] + J2[0][0][2];
    double I2y = J2[2][0][0] + J2[0][0][2];
    double I2z = J2[2][0][0] + J2[0][2][0];
    
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

    //order 0 (Keplerian)
    double V0 = -G*M1*M2/d;

    //order 1
    //V1 = 0 (by default)

    //order 2
    double V2 = -(G*M2/(2.0*d*d*d))*( (1.0 - 3.0*l1*l1)*I1x + (1.0 - 3.0*m1*m1)*I1y + (1.0 - 3.0*n1*n1)*I1z ) +
                -(G*M1/(2.0*d*d*d))*( (1.0 - 3.0*l2*l2)*I2x + (1.0 - 3.0*m2*m2)*I2y + (1.0 - 3.0*n2*n2)*I2z );

    //order 3
    double V3 = -(G*M2/(2*d*d*d*d))*(   l1*(5*l1*l1 - 3)*J1xxx +   m1*(5*m1*m1 - 3)*J1yyy +   n1*(5*n1*n1 - 3)*J1zzz +
                                      3*m1*(5*l1*l1 - 1)*J1xxy + 3*l1*(5*m1*m1 - 1)*J1xyy + 3*n1*(5*l1*l1 - 1)*J1xxz +
                                      3*l1*(5*n1*n1 - 1)*J1xzz + 3*n1*(5*m1*m1 - 1)*J1yyz + 3*m1*(5*n1*n1 - 1)*J1yzz +
                                     30*l1*m1*n1*J1xyz ) +
                 (G*M1/(2*d*d*d*d))*(   l2*(5*l2*l2 - 3)*J2xxx +   m2*(5*m2*m2 - 3)*J2yyy +   n2*(5*n2*n2 - 3)*J2zzz +
                                      3*m2*(5*l2*l2 - 1)*J2xxy + 3*l2*(5*m2*m2 - 1)*J2xyy + 3*n2*(5*l2*l2 - 1)*J2xxz +
                                      3*l2*(5*n2*n2 - 1)*J2xzz + 3*n2*(5*m2*m2 - 1)*J2yyz + 3*m2*(5*n2*n2 - 1)*J2yzz +
                                     30*l2*m2*n2*J2xyz );
    
    //order 4
    double V4 = -(G*M2/(8*d*d*d*d*d))*( (35*pow(l1,4) - 30*l1*l1 + 3)*J1xxxx + (35*pow(m1,4) - 30*m1*m1 + 3)*J1yyyy + (35*pow(n1,4) - 30*n1*n1 + 3)*J1zzzz +
                                         20*l1*m1*(7*l1*l1 - 3)*J1xxxy + 20*l1*m1*(7*m1*m1 - 3)*J1xyyy +
                                         20*l1*n1*(7*l1*l1 - 3)*J1xxxz + 20*l1*n1*(7*n1*n1 - 3)*J1xzzz +
                                         20*m1*n1*(7*m1*m1 - 3)*J1yyyz + 20*m1*n1*(7*n1*n1 - 3)*J1yzzz +
                                         6*(35*l1*l1*m1*m1 - 5*(l1*l1 + m1*m1) + 1)*J1xxyy +
                                         6*(35*l1*l1*n1*n1 - 5*(l1*l1 + n1*n1) + 1)*J1xxzz +
                                         6*(35*m1*m1*n1*n1 - 5*(m1*m1 + n1*n1) + 1)*J1yyzz +
                                         60*m1*n1*(7*l1*l1 - 1)*J1xxyz + 60*l1*n1*(7*m1*m1 - 1)*J1xyyz + 60*l1*m1*(7*n1*n1 - 1)*J1xyzz ) +
                -(G*M1/(8*d*d*d*d*d))*( (35*pow(l2,4) - 30*l2*l2 + 3)*J2xxxx + (35*pow(m2,4) - 30*m2*m2 + 3)*J2yyyy + (35*pow(n2,4) - 30*n2*n2 + 3)*J2zzzz +
                                         20*l2*m2*(7*l2*l2 - 3)*J2xxxy + 20*l2*m2*(7*m2*m2 - 3)*J2xyyy +
                                         20*l2*n2*(7*l2*l2 - 3)*J2xxxz + 20*l2*n2*(7*n2*n2 - 3)*J2xzzz +
                                         20*m2*n2*(7*m2*m2 - 3)*J2yyyz + 20*m2*n2*(7*n2*n2 - 3)*J2yzzz +
                                         6*(35*l2*l2*m2*m2 - 5*(l2*l2 + m2*m2) + 1)*J2xxyy +
                                         6*(35*l2*l2*n2*n2 - 5*(l2*l2 + n2*n2) + 1)*J2xxzz +
                                         6*(35*m2*m2*n2*n2 - 5*(m2*m2 + n2*n2) + 1)*J2yyzz +
                                         60*m2*n2*(7*l2*l2 - 1)*J2xxyz + 60*l2*n2*(7*m2*m2 - 1)*J2xyyz + 60*l2*m2*(7*n2*n2 - 1)*J2xyzz ) +
                -(G/(8*d*d*d*d*d))*( (6 + 12*c11*c11 - 30*(l1*l1 + l2*l2) - 120*c11*l1*l2 + 210*l1*l1*l2*l2)*J1xx*J2xx +
                                     (6 + 12*c22*c22 - 30*(m1*m1 + m2*m2) - 120*c22*m1*m2 + 210*m1*m1*m2*m2)*J1yy*J2yy +
                                     (6 + 12*c33*c33 - 30*(n1*n1 + n2*n2) - 120*c33*n1*n2 + 210*n1*n1*n2*n2)*J1zz*J2zz +
                                     (6 + 12*c12*c12 - 30*(l1*l1 + m2*m2) - 120*c12*l1*m2 + 210*l1*l1*m2*m2)*J1xx*J2yy +
                                     (6 + 12*c21*c21 - 30*(l2*l2 + m1*m1) - 120*c21*l2*m1 + 210*l2*l2*m1*m1)*J1yy*J2xx +
                                     (6 + 12*c13*c13 - 30*(l1*l1 + n2*n2) - 120*c13*l1*n2 + 210*l1*l1*n2*n2)*J1xx*J2zz +
                                     (6 + 12*c31*c31 - 30*(l2*l2 + n1*n1) - 120*c31*l2*n1 + 210*l2*l2*n1*n1)*J1zz*J2xx +
                                     (6 + 12*c23*c23 - 30*(m1*m1 + n2*n2) - 120*c23*m1*n2 + 210*m1*m1*n2*n2)*J1yy*J2zz +
                                     (6 + 12*c32*c32 - 30*(m2*m2 + n1*n1) - 120*c32*m2*n1 + 210*m2*m2*n1*n1)*J1zz*J2yy );

    //V
    return V0 + V2 + V3 + V4;
}

#endif

#ifndef POTENTIAL_HPP
#define POTENTIAL_HPP

#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Potential of a rigid body upon a test particle at position r, assuming it is a mascon distribution with constant density.
double pot_masc(const dvec3 &r, const double M, const dmatnx3 &masc, const dmat3 &A)
{
    double sum = 0.0;
    for (int i = 0; i < masc.size(); ++i)
    {
        sum += 1.0/length(r - dot(A, masc[i]));
    }
    return -G*M*sum/masc.size();
}

//Potential of a rigid body upon a test particle at position r, assuming inertial integral expansion of order 2 approximation.
double pot_integrals_ord2(const dvec3 &r, const double M, const dtens &J, const dmat3 &A)
{
    double Ix = J[0][2][0] + J[0][0][2];
    double Iy = J[2][0][0] + J[0][0][2];
    double Iz = J[2][0][0] + J[0][2][0];

    dvec3 a1 = {A[0][0], A[1][0], A[2][0]};
    dvec3 a2 = {A[0][1], A[1][1], A[2][1]};
    dvec3 a3 = {A[0][2], A[1][2], A[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l = dot(ru,a1);
    double m = dot(ru,a2);
    double n = dot(ru,a3);

    //order 0 (Keplerian)
    double V0 = -G*M/d;

    //order 1
    //V1 = 0 (by default)

    //order 2
    double V2 = -(G/(2.0*d*d*d))*( (1.0 - 3.0*l*l)*Ix + (1.0 - 3.0*m*m)*Iy + (1.0 - 3.0*n*n)*Iz );

    //V
    return V0 + V2;
}

//Potential of a rigid body upon a test particle at position r, assuming inertial integral expansion of order 3 approximation.
double pot_integrals_ord3(const dvec3 &r, const double M, const dtens &J, const dmat3 &A)
{
    double Jxxx = J[3][0][0];
    double Jyyy = J[0][3][0];
    double Jzzz = J[0][0][3];
    double Jxxy = J[2][1][0];
    double Jxyy = J[1][2][0];
    double Jxxz = J[2][0][1];
    double Jxzz = J[1][0][2];
    double Jyyz = J[0][2][1];
    double Jyzz = J[0][1][2];
    double Jxyz = J[1][1][1];
    
    double Ix = J[0][2][0] + J[0][0][2];
    double Iy = J[2][0][0] + J[0][0][2];
    double Iz = J[2][0][0] + J[0][2][0];

    dvec3 a1 = {A[0][0], A[1][0], A[2][0]};
    dvec3 a2 = {A[0][1], A[1][1], A[2][1]};
    dvec3 a3 = {A[0][2], A[1][2], A[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    dvec3 ru = r/d;

    double l = dot(ru,a1);
    double m = dot(ru,a2);
    double n = dot(ru,a3);

    //order 0 (Keplerian)
    double V0 = -G*M/d;

    //order 1
    //V1 = 0 (by default)

    //order 2
    double V2 = -(G/(2.0*d*d*d))*( (1.0 - 3.0*l*l)*Ix + (1.0 - 3.0*m*m)*Iy + (1.0 - 3.0*n*n)*Iz );

    //order 3
    double V3 = -(G/(2*d*d*d*d))*(    l*(5*l*l - 3)*Jxxx +   m*(5*m*m - 3)*Jyyy +   n*(5*n*n - 3)*Jzzz +
                                    3*m*(5*l*l - 1)*Jxxy + 3*l*(5*m*m - 1)*Jxyy + 3*n*(5*l*l - 1)*Jxxz +
                                    3*l*(5*n*n - 1)*Jxzz + 3*n*(5*m*m - 1)*Jyyz + 3*m*(5*n*n - 1)*Jyzz +
                                   30*l*m*n*Jxyz );

    //V
    return V0 + V2 + V3;
}

//Potential of a polyhedron upon a test particle at position rp, assuming constant density rho (= mass/volume) and geometry data defined by {verts, faces, edges, E, F}.
//E, F are computed in poly_EF() function in polyhedron.hpp. 
double pot_poly(const dvec3 &rp, const double rho, const dmatnx3 &verts, const imatnx3 &faces, const imatnx4 &edges, const dtensnx3x3 &E, const dtensnx3x3 &F)
{
    double sum1 = 0.0;
    for (int i = 0; i < edges.size(); ++i)
    {
        dvec3 r0 = verts[edges[i][0]] - rp;
        dvec3 r1 = verts[edges[i][1]] - rp;
        double l0 = length(r0);
        double l1 = length(r1);
        double le = length(r0 - r1);
        double L = log( (l0 + l1 + le)/(l0 + l1 - le) ); //natural logarithm
        sum1 += dot(dot(r0,E[i]), r0)*L;
    }

    double sum2 = 0.0;
    for (int i = 0; i < faces.size(); ++i)
    {
        dvec3 r0 = verts[faces[i][0]] - rp;
        dvec3 r1 = verts[faces[i][1]] - rp;
        dvec3 r2 = verts[faces[i][2]] - rp;
        double l0 = length(r0);
        double l1 = length(r1);
        double l2 = length(r2);
        double numerator = dot(r0, cross(r1,r2));
        double denominator = l0*l1*l2 + l0*dot(r1,r2) + l1*dot(r2,r0) + l2*dot(r0,r1);
        double w = 2.0*atan2(numerator, denominator);
        sum2 += dot(dot(r0,F[i]), r0)*w;
    }

    return -G*rho*(sum1 - sum2)/2.0;
}

//Potnetial of a rigid body upon a test particle at position r, assuming Stokes coefficients expansion of arbitrary order.
double pot_stokes(const dvec3 &r, const double M, const dmat &C, const dmat &S, const int ord, const double R0)
{
    double x = r[0], y = r[1], z = r[2];
    double d = sqrt(x*x + y*y + z*z);
    dmat V,W;
    for (int n = 0; n < ord + 2; ++n)
    {
        V.push_back(dvec{});
        W.push_back(dvec{});
        for (int m = 0; m < n + 1; ++m)
        {
            V[n].push_back(0.0);
            W[n].push_back(0.0);
            if (n == 0 && m == 0)
            {
                V[n][m] = R0/d;
                W[n][m] = 0.0;
            }
            else if (n == m && n != 0)
            {
                V[m][m] = (2*m-1)*((x*R0/(d*d))*V[m-1][m-1] - (y*R0/(d*d))*W[m-1][m-1]);
                W[m][m] = (2*m-1)*((x*R0/(d*d))*W[m-1][m-1] + (y*R0/(d*d))*V[m-1][m-1]);
            }
            else //n != m
            {
                if (n - 2 < m)
                {
                    V[n][m] = (2*n-1)*z*R0*V[n-1][m]/((n-m)*d*d);
                    W[n][m] = (2*n-1)*z*R0*W[n-1][m]/((n-m)*d*d);
                }
                else
                {
                    V[n][m] = (2*n-1)*z*R0*V[n-1][m]/((n-m)*d*d) - (n+m-1)*R0*R0*V[n-2][m]/((n-m)*d*d);
                    W[n][m] = (2*n-1)*z*R0*W[n-1][m]/((n-m)*d*d) - (n+m-1)*R0*R0*W[n-2][m]/((n-m)*d*d);
                }
            }
        }
    }

    double pot = 0.0;
    for (int n = 0; n < ord + 1; ++n)
        for (int m = 0; m < n + 1; ++m)
            pot += C[n][m]*V[n][m] + S[n][m]*W[n][m];
    pot *= -G*M/R0;
    return pot;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

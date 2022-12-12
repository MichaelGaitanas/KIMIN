#ifndef FORCE_HPP
#define FORCE_HPP

#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Force of a rigid body upon a test paticle at position r, assuming mascon distribution with constant density.
dvec3 force_masc(const dvec3 &r, const double M, const dmatnx3 &masc, const dmat3 &A)
{
    dvec3 sum = {0.0,0.0,0.0};
    for (int i = 0; i < masc.size(); ++i)
    {
        dvec3 ri = dot(A, masc[i]);
        sum = sum + (r - ri)/pow(length(r - ri), 3.0);
    }
    return -G*M*sum/masc.size();
}

//Force of a rigid body upon a test particle at position r, assuming inertial integral expansion of order 2 approximation.
dvec3 force_integrals_ord2(const dvec3 &r, const double M, const dtens &J, const dmat3 &A)
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

    double dV_dd = -V0/d - 3*V2/d;
    dvec3 dd_dr = ru;

    double dV_dl = 3*G*Ix*l/(d*d*d);
    double dV_dm = 3*G*Iy*m/(d*d*d);
    double dV_dn = 3*G*Iz*n/(d*d*d);
    dvec3 dl_dr = (a1*d - r*l)/(d*d);
    dvec3 dm_dr = (a2*d - r*m)/(d*d);
    dvec3 dn_dr = (a3*d - r*n)/(d*d);

    //-grad(V)
    return -(dV_dd*dd_dr + dV_dl*dl_dr + dV_dm*dm_dr + dV_dn*dn_dr);
}

//Force of a rigid body upon a test particle at position r, assuming inertial integral expansion of order 3 approximation.
dvec3 force_integrals_ord3(const dvec3 &r, const double M, const dtens &J, const dmat3 &A)
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

    double dV_dd = -V0/d - 3*V2/d - 4*V3/d;
    dvec3 dd_dr = ru;

    double dV_dl = 3*G*Ix*l/(d*d*d) - G*(10*Jxxx*l*l + Jxxx*(5*l*l - 3) + 30*Jxxy*l*m + 30*Jxxz*l*n + 3*Jxyy*(5*m*m - 1) + 30*Jxyz*m*n + 3*Jxzz*(5*n*n - 1))/(2*d*d*d*d);
    double dV_dm = 3*G*Iy*m/(d*d*d) - G*(3*Jxxy*(5*l*l - 1) + 30*Jxyy*l*m + 30*Jxyz*l*n + 10*Jyyy*m*m + Jyyy*(5*m*m - 3) + 30*Jyyz*m*n + 3*Jyzz*(5*n*n - 1))/(2*d*d*d*d);
    double dV_dn = 3*G*Iz*n/(d*d*d) - G*(3*Jxxz*(5*l*l - 1) + 30*Jxyz*l*m + 30*Jxzz*l*n + 3*Jyyz*(5*m*m - 1) + 30*Jyzz*m*n + 10*Jzzz*n*n + Jzzz*(5*n*n - 3))/(2*d*d*d*d);
    dvec3 dl_dr = (a1*d - r*l)/(d*d);
    dvec3 dm_dr = (a2*d - r*m)/(d*d);
    dvec3 dn_dr = (a3*d - r*n)/(d*d);

    //-grad(V)
    return -(dV_dd*dd_dr + dV_dl*dl_dr + dV_dm*dm_dr + dV_dn*dn_dr);
}

//Force of a polyhedron upon a test particle at position rp, assuming constant density rho (= mass/volume) and geometry data defined by {verts, faces, edges, E, F}.
//E, F are computed in poly_EF() function in polyhedron.hpp. 
dvec3 force_poly(const dvec3 &rp, const double rho, const dmatnx3 &verts, const imatnx3 &faces, const imatnx4 &edges, const dtensnx3x3 &E, const dtensnx3x3 &F)
{
    dvec3 sum1 = {0.0,0.0,0.0};
    for (int i = 0; i < edges.size(); ++i)
    {
        dvec3 r0 = verts[edges[i][0]] - rp;
        dvec3 r1 = verts[edges[i][1]] - rp;
        double l0 = length(r0);
        double l1 = length(r1);
        double le = length(r0 - r1);
        double L = log( (l0 + l1 + le)/(l0 + l1 - le) ); //natural logarithm
        sum1 = sum1 + dot(E[i],r0)*L;
    }

    dvec3 sum2 = {0.0,0.0,0.0};
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
        sum2 = sum2 + dot(F[i],r0)*w;
    }

    return -G*rho*(sum1 - sum2);
}

//Force of a rigid body upon a test particle, assuming Stokes coefficients expansion of arbitrary order.
dvec3 force_stokes(const dvec3 &r, const double M, const dmat &C, const dmat &S, const int ord, const double R0)
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

    double fx = 0.0, fy = 0.0, fz = 0.0;
    for (int n = 0; n < ord + 1; ++n)
    {
        for (int m = 0; m < n + 1; ++m)
        {
            if (m == 0)
            {
                fx += -C[n][0]*V[n+1][1];
                fy += -C[n][0]*W[n+1][1];
            }
            else
            {
                //be extremely careful with the division of the quantity (n-m+2)!/(n-m)!. Make sure that a floating point division will happen.
                fx += 0.5*( -C[n][m]*V[n+1][m+1] - S[n][m]*W[n+1][m+1] + ( (double)factorial(n-m+2.0)/factorial(n-m))*( C[n][m]*V[n+1][m-1] + S[n][m]*W[n+1][m-1]) );
                fy += 0.5*( -C[n][m]*W[n+1][m+1] + S[n][m]*V[n+1][m+1] + ( (double)factorial(n-m+2.0)/factorial(n-m))*(-C[n][m]*W[n+1][m-1] + S[n][m]*V[n+1][m-1]) );
            }
            fz += (n-m+1)*( -C[n][m]*V[n+1][m] - S[n][m]*W[n+1][m] );
        }
    }
    fx *= G*M/(R0*R0);
    fy *= G*M/(R0*R0);
    fz *= G*M/(R0*R0);
    return {fx,fy,fz};
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

//Mutual force of 2 rigid bodies, assuming mascon distributions with constant densities.
dvec3 mut_force_masc(const dvec3 &r, const double M1, const dmatnx3 &masc1, const dmat3 &A1,
                                     const double M2, const dmatnx3 &masc2, const dmat3 &A2)
{
    dvec3 sum = {0.0,0.0,0.0};
    for (int i = 0; i < masc1.size(); ++i)
    {
        dvec3 a1i = dot(A1, masc1[i]);
        for (int j = 0; j < masc2.size(); ++j)
        {
            dvec3 a2j = dot(A2, masc2[j]);
            dvec3 dij = r + a2j - a1i;
            double len = length(dij);
            sum = sum + dij/(len*len*len);
        }
    }
    return -G*M1*M2*sum/(masc1.size()*masc2.size());
}

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

//Mutual force of 2 rigid bodies, assuming inertial integral expansion of order 3 approximation.
dvec3 mut_force_integrals_ord3(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
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

    double dV_dd  = -V0/d - 3*V2/d - 4*V3/d;
    dvec3 dd_dr  = r/d;

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d) - G*M2*(10*J1xxx*l1*l1 + J1xxx*(5*l1*l1 - 3) + 30*J1xxy*l1*m1 + 30*J1xxz*l1*n1 + 3*J1xyy*(5*m1*m1 - 1) + 30*J1xyz*m1*n1 + 3*J1xzz*(5*n1*n1 - 1))/(2*d*d*d*d);
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d) - G*M2*(3*J1xxy*(5*l1*l1 - 1) + 30*J1xyy*l1*m1 + 30*J1xyz*l1*n1 + 10*J1yyy*m1*m1 + J1yyy*(5*m1*m1 - 3) + 30*J1yyz*m1*n1 + 3*J1yzz*(5*n1*n1 - 1))/(2*d*d*d*d);
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d) - G*M2*(3*J1xxz*(5*l1*l1 - 1) + 30*J1xyz*l1*m1 + 30*J1xzz*l1*n1 + 3*J1yyz*(5*m1*m1 - 1) + 30*J1yzz*m1*n1 + 10*J1zzz*n1*n1 + J1zzz*(5*n1*n1 - 3))/(2*d*d*d*d);
    dvec3 dl1_dr = (a1*d - r*l1)/(d*d);
    dvec3 dm1_dr = (a2*d - r*m1)/(d*d);
    dvec3 dn1_dr = (a3*d - r*n1)/(d*d);

    double dV_dl2 = 3*G*I2x*M1*l2/(d*d*d) + G*M1*(10*J2xxx*l2*l2 + J2xxx*(5*l2*l2 - 3) + 30*J2xxy*l2*m2 + 30*J2xxz*l2*n2 + 3*J2xyy*(5*m2*m2 - 1) + 30*J2xyz*m2*n2 + 3*J2xzz*(5*n2*n2 - 1))/(2*d*d*d*d);
    double dV_dm2 = 3*G*I2y*M1*m2/(d*d*d) + G*M1*(3*J2xxy*(5*l2*l2 - 1) + 30*J2xyy*l2*m2 + 30*J2xyz*l2*n2 + 10*J2yyy*m2*m2 + J2yyy*(5*m2*m2 - 3) + 30*J2yyz*m2*n2 + 3*J2yzz*(5*n2*n2 - 1))/(2*d*d*d*d);
    double dV_dn2 = 3*G*I2z*M1*n2/(d*d*d) + G*M1*(3*J2xxz*(5*l2*l2 - 1) + 30*J2xyz*l2*m2 + 30*J2xzz*l2*n2 + 3*J2yyz*(5*m2*m2 - 1) + 30*J2yzz*m2*n2 + 10*J2zzz*n2*n2 + J2zzz*(5*n2*n2 - 3))/(2*d*d*d*d);
    dvec3 dl2_dr = (b1*d - r*l2)/(d*d);
    dvec3 dm2_dr = (b2*d - r*m2)/(d*d);
    dvec3 dn2_dr = (b3*d - r*n2)/(d*d);

    //-grad(V)
    return -(dV_dd*dd_dr + dV_dl1*dl1_dr + dV_dm1*dm1_dr + dV_dn1*dn1_dr +
                           dV_dl2*dl2_dr + dV_dm2*dm2_dr + dV_dn2*dn2_dr);
}

//Mutual force of 2 rigid bodies, assuming inertial integral expansion of order 4 approximation.
dvec3 mut_force_integrals_ord4(const dvec3 &r, const double M1, const dtens &J1, const dmat3 &A1,
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

    double dV_dd  = -V0/d - 3*V2/d - 4*V3/d - 5*V4/d;
    dvec3 dd_dr  = r/d;

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d) -
             G*M2*(10*J1xxx*l1*l1 + J1xxx*(5*l1*l1 - 3) + 30*J1xxy*l1*m1 + 30*J1xxz*l1*n1 + 3*J1xyy*(5*m1*m1 - 1) + 30*J1xyz*m1*n1 + 3*J1xzz*(5*n1*n1 - 1))/(2*d*d*d*d) +
             5*G*(3*J1xx*(J2xx*(2*c11*l2 - 7*l1*l2*l2 + l1) + J2yy*(2*c12*m2 - 7*l1*m2*m2 + l1) + J2zz*(2*c13*n2 - 7*l1*n2*n2 + l1)) - M2*(J1xxxx*l1*(7*l1*l1 - 3) + 14*J1xxxy*l1*l1*m1 + J1xxxy*m1*(7*l1*l1 - 3) + 14*J1xxxz*l1*l1*n1 + J1xxxz*n1*(7*l1*l1 - 3) + 3*J1xxyy*l1*(7*m1*m1 - 1) + 42*J1xxyz*l1*m1*n1 + 3*J1xxzz*l1*(7*n1*n1 - 1) + J1xyyy*m1*(7*m1*m1 - 3) + 3*J1xyyz*n1*(7*m1*m1 - 1) + 3*J1xyzz*m1*(7*n1*n1 - 1) + J1xzzz*n1*(7*n1*n1 - 3)))/(2*d*d*d*d*d);
    
    
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d) -
             G*M2*(3*J1xxy*(5*l1*l1 - 1) + 30*J1xyy*l1*m1 + 30*J1xyz*l1*n1 + 10*J1yyy*m1*m1 + J1yyy*(5*m1*m1 - 3) + 30*J1yyz*m1*n1 + 3*J1yzz*(5*n1*n1 - 1))/(2*d*d*d*d) +
             5*G*(3*J1yy*(J2xx*(2*c21*l2 - 7*l2*l2*m1 + m1) + J2yy*(2*c22*m2 - 7*m1*m2*m2 + m1) + J2zz*(2*c23*n2 - 7*m1*n2*n2 + m1)) - M2*(J1xxxy*l1*(7*l1*l1 - 3) + 3*J1xxyy*m1*(7*l1*l1 - 1) + 3*J1xxyz*n1*(7*l1*l1 - 1) + 14*J1xyyy*l1*m1*m1 + J1xyyy*l1*(7*m1*m1 - 3) + 42*J1xyyz*l1*m1*n1 + 3*J1xyzz*l1*(7*n1*n1 - 1) + J1yyyy*m1*(7*m1*m1 - 3) + 14*J1yyyz*m1*m1*n1 + J1yyyz*n1*(7*m1*m1 - 3) + 3*J1yyzz*m1*(7*n1*n1 - 1) + J1yzzz*n1*(7*n1*n1 - 3)))/(2*d*d*d*d*d);
    
    
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d) -
             G*M2*(3*J1xxz*(5*l1*l1 - 1) + 30*J1xyz*l1*m1 + 30*J1xzz*l1*n1 + 3*J1yyz*(5*m1*m1 - 1) + 30*J1yzz*m1*n1 + 10*J1zzz*n1*n1 + J1zzz*(5*n1*n1 - 3))/(2*d*d*d*d) +
             5*G*(3*J1zz*(J2xx*(2*c31*l2 - 7*l2*l2*n1 + n1) + J2yy*(2*c32*m2 - 7*m2*m2*n1 + n1) + J2zz*(2*c33*n2 - 7*n1*n2*n2 + n1)) - M2*(J1xxxz*l1*(7*l1*l1 - 3) + 3*J1xxyz*m1*(7*l1*l1 - 1) + 3*J1xxzz*n1*(7*l1*l1 - 1) + 3*J1xyyz*l1*(7*m1*m1 - 1) + 42*J1xyzz*l1*m1*n1 + 14*J1xzzz*l1*n1*n1 + J1xzzz*l1*(7*n1*n1 - 3) + J1yyyz*m1*(7*m1*m1 - 3) + 3*J1yyzz*n1*(7*m1*m1 - 1) + 14*J1yzzz*m1*n1*n1 + J1yzzz*m1*(7*n1*n1 - 3) + J1zzzz*n1*(7*n1*n1 - 3)))/(2*d*d*d*d*d);
    
    
    dvec3 dl1_dr = (a1*d - r*l1)/(d*d);
    dvec3 dm1_dr = (a2*d - r*m1)/(d*d);
    dvec3 dn1_dr = (a3*d - r*n1)/(d*d);

    double dV_dl2 = 3*G*I2x*M1*l2/(d*d*d) +
                    G*M1*(10*J2xxx*l2*l2 + J2xxx*(5*l2*l2 - 3) + 30*J2xxy*l2*m2 + 30*J2xxz*l2*n2 + 3*J2xyy*(5*m2*m2 - 1) + 30*J2xyz*m2*n2 + 3*J2xzz*(5*n2*n2 - 1))/(2*d*d*d*d) +
                    5*G*(3*J2xx*(J1xx*(2*c11*l1 - 7*l1*l1*l2 + l2) + J1yy*(2*c21*m1 - 7*l2*m1*m1 + l2) + J1zz*(2*c31*n1 - 7*l2*n1*n1 + l2)) - M1*(J2xxxx*l2*(7*l2*l2 - 3) + 14*J2xxxy*l2*l2*m2 + J2xxxy*m2*(7*l2*l2 - 3) + 14*J2xxxz*l2*l2*n2 + J2xxxz*n2*(7*l2*l2 - 3) + 3*J2xxyy*l2*(7*m2*m2 - 1) + 42*J2xxyz*l2*m2*n2 + 3*J2xxzz*l2*(7*n2*n2 - 1) + J2xyyy*m2*(7*m2*m2 - 3) + 3*J2xyyz*n2*(7*m2*m2 - 1) + 3*J2xyzz*m2*(7*n2*n2 - 1) + J2xzzz*n2*(7*n2*n2 - 3)))/(2*d*d*d*d*d);
    
    
    double dV_dm2 = 3*G*I2y*M1*m2/(d*d*d) +
                    G*M1*(3*J2xxy*(5*l2*l2 - 1) + 30*J2xyy*l2*m2 + 30*J2xyz*l2*n2 + 10*J2yyy*m2*m2 + J2yyy*(5*m2*m2 - 3) + 30*J2yyz*m2*n2 + 3*J2yzz*(5*n2*n2 - 1))/(2*d*d*d*d) +
                    5*G*(3*J2yy*(J1xx*(2*c12*l1 - 7*l1*l1*m2 + m2) + J1yy*(2*c22*m1 - 7*m1*m1*m2 + m2) + J1zz*(2*c32*n1 - 7*m2*n1*n1 + m2)) - M1*(J2xxxy*l2*(7*l2*l2 - 3) + 3*J2xxyy*m2*(7*l2*l2 - 1) + 3*J2xxyz*n2*(7*l2*l2 - 1) + 14*J2xyyy*l2*m2*m2 + J2xyyy*l2*(7*m2*m2 - 3) + 42*J2xyyz*l2*m2*n2 + 3*J2xyzz*l2*(7*n2*n2 - 1) + J2yyyy*m2*(7*m2*m2 - 3) + 14*J2yyyz*m2*m2*n2 + J2yyyz*n2*(7*m2*m2 - 3) + 3*J2yyzz*m2*(7*n2*n2 - 1) + J2yzzz*n2*(7*n2*n2 - 3)))/(2*d*d*d*d*d);
    
    
    double dV_dn2 = 3*G*I2z*M1*n2/(d*d*d) +
                    G*M1*(3*J2xxz*(5*l2*l2 - 1) + 30*J2xyz*l2*m2 + 30*J2xzz*l2*n2 + 3*J2yyz*(5*m2*m2 - 1) + 30*J2yzz*m2*n2 + 10*J2zzz*n2*n2 + J2zzz*(5*n2*n2 - 3))/(2*d*d*d*d) +
                    5*G*(3*J2zz*(J1xx*(2*c13*l1 - 7*l1*l1*n2 + n2) + J1yy*(2*c23*m1 - 7*m1*m1*n2 + n2) + J1zz*(2*c33*n1 - 7*n1*n1*n2 + n2)) - M1*(J2xxxz*l2*(7*l2*l2 - 3) + 3*J2xxyz*m2*(7*l2*l2 - 1) + 3*J2xxzz*n2*(7*l2*l2 - 1) + 3*J2xyyz*l2*(7*m2*m2 - 1) + 42*J2xyzz*l2*m2*n2 + 14*J2xzzz*l2*n2*n2 + J2xzzz*l2*(7*n2*n2 - 3) + J2yyyz*m2*(7*m2*m2 - 3) + 3*J2yyzz*n2*(7*m2*m2 - 1) + 14*J2yzzz*m2*n2*n2 + J2yzzz*m2*(7*n2*n2 - 3) + J2zzzz*n2*(7*n2*n2 - 3)))/(2*d*d*d*d*d);
    
    dvec3 dl2_dr = (b1*d - r*l2)/(d*d);
    dvec3 dm2_dr = (b2*d - r*m2)/(d*d);
    dvec3 dn2_dr = (b3*d - r*n2)/(d*d);

    //-grad(V)
    return -(dV_dd*dd_dr + dV_dl1*dl1_dr + dV_dm1*dm1_dr + dV_dn1*dn1_dr +
                           dV_dl2*dl2_dr + dV_dm2*dm2_dr + dV_dn2*dn2_dr);
}

#endif

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include<cmath>
#include<cstdio>
#include<iostream>
#include<algorithm>

#include"constant.h"
#include"typedef.h"
#include"linalg.h"

//Quaternion odes rhs for a rigid body. The angular velocity w is in the body frame.
dvec4 quat_rhs(const dvec4 &q, const dvec3 &w)
{
    //This comment is useless, but I wanted you to know that right now, I am listening to this
    //https://www.youtube.com/watch?v=A_tA7G1Y8Ew&list=RDX2xpR5EoAGY&index=2&ab_channel=FrankKlepacki-Topic
    //song while writing the code.
    double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0, dq1, dq2, dq3};
}

//Euler odes rhs for a rigid body, assuming I[][] is diagonal (principal axes frame).
//Angular velocity w, moment of inertia I and torque tau are in the body frame.
dvec3 euler_rhs(const dvec3 &w, const dmat3 &I, const dvec3 &tau)
{
    double dw0 = (tau[0] + w[1]*w[2]*(I[1][1] - I[2][2]))/I[0][0];
    double dw1 = (tau[1] + w[2]*w[0]*(I[2][2] - I[0][0]))/I[1][1];
    double dw2 = (tau[2] + w[0]*w[1]*(I[0][0] - I[1][1]))/I[2][2];
    return {dw0, dw1, dw2};
}

bool inertia_is_diagonal(const dmat3 &I, const double tolerance = 1.0e-12)
{
    double Ixx = I[0][0], Ixy = I[0][1], Ixz = I[0][2], Iyy = I[1][1], Iyz = I[1][2], Izz = I[2][2];
    Ixx = std::fabs(Ixx);
    Iyy = std::fabs(Iyy);
    Izz = std::fabs(Izz);
    Ixy = std::fabs(Ixy);
    Ixz = std::fabs(Ixz);
    Iyz = std::fabs(Iyz);

    //Find the maximum value of the diagonal elements.
    double max_diag = std::max(std::max(Ixx, Iyy), Izz);

    //Compare all the off-diagonal elements against the precomputed max diagonal element.
    if (Ixy >= tolerance*max_diag || Ixz >= tolerance*max_diag || Iyz >= tolerance*max_diag)
        return false;
    
    //However, if all off-diagonal checks pass, the inertia matrix is considered diagonal.
    return true;
}

//Calculate the eigenvalues of the inertia matrix, assuming it is 3x3, real and symmetric.
dvec3 inertia_eigvals(const dmat3 &I)
{
    double Ixx = I[0][0], Ixy = I[0][1], Ixz = I[0][2], Iyy = I[1][1], Iyz = I[1][2], Izz = I[2][2];

    //Characteristic polynomial coefficients, computed analytically ( P(x) = -x^3 + b*x^2 + c*x + d ).
    double b = Ixx + Iyy + Izz;
    double c = Ixy*Ixy + Ixz*Ixz - Ixx*Iyy + Iyz*Iyz - Ixx*Izz - Iyy*Izz;
    double d = -Ixz*Ixz*Iyy + 2.0*Ixy*Ixz*Iyz - Ixx*Iyz*Iyz - Ixy*Ixy*Izz + Ixx*Iyy*Izz;

    //Find the roots of the polynomial.
    dcomplexvec sol = solve_cubic(-1.0, b, c, d);

    //The roots ought to be real due to the 'real and symmetric' property of the matrix.
    //Hence we manually get rid of the imaginary part by returning the real part only.
    return {sol[0].real(), sol[1].real(), sol[2].real()};
}

//Calculate the eigenvectors of the inertia matrix, assuming it is 3x3, real and symmetric.
dmat3 inertia_eigvecs(const dmat3 &I)
{
    double Ixx = I[0][0], Ixy = I[0][1], Ixz = I[0][2], Iyy = I[1][1], Iyz = I[1][2], Izz = I[2][2];

    dvec3 eigvals = inertia_eigvals(I);
    std::sort(eigvals.begin(), eigvals.end());

    dvec3 v0,v1,v2;
    //Check if the matrix is diagonal. If yes, we already know the eigenvectors...
    if (inertia_is_diagonal(I))
    {
        v0 = {1.0, 0.0, 0.0};
        v1 = {0.0, 1.0, 0.0};
        v2 = {0.0, 0.0, 1.0};
    }
    else
    {
        double a = -Ixy*Ixy + (Ixx - eigvals[0])*(Iyy - eigvals[0]);
        double b = -Ixy*Ixz + (Ixx - eigvals[0])*Iyz;
        double c = -Ixz*Ixz + (Ixx - eigvals[0])*(Izz - eigvals[0]);

       v0 = {(Ixy*(b+c)/(a+b) - Ixz)/(Ixx - eigvals[0]),
              -(b+c)/(a+b),
              1.0};

        a = -Ixy*Ixy + (Ixx - eigvals[1])*(Iyy - eigvals[1]);
        b = -Ixy*Ixz + (Ixx - eigvals[1])*Iyz;
        c = -Ixz*Ixz + (Ixx - eigvals[1])*(Izz - eigvals[1]);

        v1 = {(Ixy*(b+c)/(a+b) - Ixz)/(Ixx - eigvals[1]),
              -(b+c)/(a+b),
              1.0};

        a = -Ixy*Ixy + (Ixx - eigvals[2])*(Iyy - eigvals[2]);
        b = -Ixy*Ixz + (Ixx - eigvals[2])*Iyz;
        c = -Ixz*Ixz + (Ixx - eigvals[2])*(Izz - eigvals[2]);

        v2 = {(Ixy*(b+c)/(a+b) - Ixz)/(Ixx - eigvals[2]),
              -(b+c)/(a+b),
              1.0};

        //Normalize the eigenvectors.
        v0 = v0/length(v0);
        v1 = v1/length(v1);
        v2 = v2/length(v2);
    }

    //Construct a 3x3 matrix whose ROWS are v0,v1,v2.
    dmat3 eigmat = {{{v0[0], v0[1], v0[2]},
                     {v1[0], v1[1], v1[2]},
                     {v2[0], v2[1], v2[2]} }};

    return transpose(eigmat);
}

//Sphere-sphere collision detection criterion.
bool sphere_sphere_collision(const double distance, const double R1, const double R2)
{
    if (distance <= R1 + R2)
        return true;
    return false;
}

#endif

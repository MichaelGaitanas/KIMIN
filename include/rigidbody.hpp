#ifndef RIGIDBODY_HPP
#define RIGIDBODY_HPP

#include<cmath>
#include<cstdio>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"
#include"conversion.hpp"
#include"potential.hpp"

//Quaternion odes rhs (angular velocity w is in the body frame).
dvec4 quat_rhs(const dvec4 &q, const dvec3 &w)
{
    double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0, dq1, dq2, dq3};
    //This comment is useless, but I wanted you to know that right now, I am listening to this
    //https://www.youtube.com/watch?v=A_tA7G1Y8Ew&list=RDX2xpR5EoAGY&index=2&ab_channel=FrankKlepacki-Topic
    //song while writing the code.
}

//Euler odes rhs assuming I[][] is diagonal (principal axes frame).
//Angular velocity w, moment of inertia I and torque tau are in the body frame.
dvec3 euler_rhs(const dvec3 &w, const dmat3 &I, const dvec3 &tau)
{
    double dw0 = (tau[0] + w[1]*w[2]*(I[1][1] - I[2][2]))/I[0][0];
    double dw1 = (tau[1] + w[2]*w[0]*(I[2][2] - I[0][0]))/I[1][1];
    double dw2 = (tau[2] + w[0]*w[1]*(I[0][0] - I[1][1]))/I[2][2];
    return {dw0, dw1, dw2};
}

//Calculate the eigenvalues of the inertia matrix, assuming it is 3x3, real and symmetric.
dvec3 inertia_eigvals(const dmat3 &I, bool sorted = true)
{
    double Ixx = I[0][0], Ixy = I[0][1], Ixz = I[0][2], Iyy = I[1][1], Iyz = I[1][2], Izz = I[2][2];

    //characteristic polynomial coefficients, computed analytically ( P(x) = -x^3 + b*x^2 + c*x + d )
    double b = Ixx + Iyy + Izz;
    double c = Ixy*Ixy + Ixz*Ixz - Ixx*Iyy + Iyz*Iyz - Ixx*Izz - Iyy*Izz;
    double d = -Ixz*Ixz*Iyy + 2.0*Ixy*Ixz*Iyz - Ixx*Iyz*Iyz - Ixy*Ixy*Izz + Ixx*Iyy*Izz;

    //Find the roots of the polynomial. They ought to be real due to the 'real and symmetric' property of the matrix.
    dcomplexvec sol = solve_cubic(-1.0, b, c, d);

    dvec3 eigvals = {sol[0].real(), sol[1].real(), sol[2].real()};
    if (sorted) //then sort the eigenvalues in ascending order
    {
        if (eigvals[0] > eigvals[1]) swap(eigvals[0], eigvals[1]);
        if (eigvals[0] > eigvals[2]) swap(eigvals[0], eigvals[2]);
        if (eigvals[1] > eigvals[2]) swap(eigvals[1], eigvals[2]);
    }

    return eigvals;
}

//Calculate the eigenvectors of the inertia matrix, assuming it is 3x3, real and symmetric.
dmat3 inertia_eigvecs(const dmat3 &I, bool normalized = true)
{
    double Ixx = I[0][0], Ixy = I[0][1], Ixz = I[0][2], Iyy = I[1][1], Iyz = I[1][2], Izz = I[2][2];
    dvec3 eigvals = inertia_eigvals(I);

    dvec3 v0 = {1.0,
                -(Ixy*Ixz - Ixx*Iyz + Iyz*eigvals[0])/(Ixz*Iyy - Ixy*Iyz - Ixz*eigvals[0]),
                -(Ixy*Ixy - Ixx*Iyy + Ixx*eigvals[0] + Iyy*eigvals[0] - eigvals[0]*eigvals[0])/(-Ixz*Iyy + Ixy*Iyz + Ixz*eigvals[0])};

    dvec3 v1 = {1.0,
                -(Ixy*Ixz - Ixx*Iyz + Iyz*eigvals[1])/(Ixz*Iyy - Ixy*Iyz - Ixz*eigvals[1]),
                -(Ixy*Ixy - Ixx*Iyy + Ixx*eigvals[1] + Iyy*eigvals[1] - eigvals[1]*eigvals[1])/(-Ixz*Iyy + Ixy*Iyz + Ixz*eigvals[1])};

    dvec3 v2 = {1.0,
                -(Ixy*Ixz - Ixx*Iyz + Iyz*eigvals[2])/(Ixz*Iyy - Ixy*Iyz - Ixz*eigvals[2]),
                -(Ixy*Ixy - Ixx*Iyy + Ixx*eigvals[2] + Iyy*eigvals[2] - eigvals[2]*eigvals[2])/(-Ixz*Iyy + Ixy*Iyz + Ixz*eigvals[2])};

    if (normalized)
    {
        v0 = v0/length(v0);
        v1 = v1/length(v1);
        v2 = v2/length(v2);
    }

    /*
    if (v0[0] < 0.0)
    {
        v0[0] = -v0[0];
        v0[1] = -v0[1];
        v0[2] = -v0[2];
    }
    */
    if (v1[1] < 0.0)
    {
        v1[1] = -v1[1];
        v1[0] = -v1[0];
        v1[2] = -v1[2];
    }
    if (v2[2] < 0.0)
    {
        v2[2] = -v2[2];
        v2[0] = -v2[0];
        v2[1] = -v2[1];
    }

    return {{{v0[0],v0[1],v0[2]},
             {v1[0],v1[1],v1[2]},
             {v2[0],v2[1],v2[2]}}};
}
//Sphere-sphere collision detection criterion. 
bool sph_sph_collision(const double dist, const double R1, const double R2)
{
    if (dist <= R1 + R2)
        return true;
    return false;
}

#endif

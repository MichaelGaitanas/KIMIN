#ifndef RIGIDBODY_HPP
#define RIGIDBODY_HPP

#include<cmath>
#include<cstdio>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"
#include"conversion.hpp"
#include"potential.hpp"

//Angle (roll, pitch, yaw) odes rhs (angular velocity w is in the body frame).
//Careful with the (virtual) singularity when pitch = +- pi/2.
dvec3 ang_rhs(const dvec3 &ang, const dvec3 &w)
{
    double roll = ang[0], pitch = ang[1];
    double cr = cos(roll);
    double sr = sin(roll);
    double cp = cos(pitch);
    double sp = sin(pitch);
    double droll = w[0] + (w[1]*sr*sp + w[2]*sp*cr)/cp;
    double dpitch = w[1]*cr - w[2]*sr;
    double dyaw = (w[1]*sr + w[2]*cr)/cp;
    return {droll, dpitch, dyaw};
}

//Rotation matrix odes rhs (angular velocity w is in the body frame).
dmat3 mat_rhs(const dmat3 &A, const dvec3 &w)
{
    double da00 = w[2]*A[0][1] - w[1]*A[0][2];
    double da01 = w[0]*A[0][2] - w[2]*A[0][0];
    double da02 = w[1]*A[0][0] - w[0]*A[0][1];

    double da10 = w[2]*A[1][1] - w[1]*A[1][2];
    double da11 = w[0]*A[1][2] - w[2]*A[1][0];
    double da12 = w[1]*A[1][0] - w[0]*A[1][1];

    double da20 = w[2]*A[2][1] - w[1]*A[2][2];
    double da21 = w[0]*A[2][2] - w[2]*A[2][0];
    double da22 = w[1]*A[2][0] - w[0]*A[2][1];

    return {{{da00,da01,da02},
             {da10,da11,da12},
             {da20,da21,da22}}};
}

//Quaternion odes rhs (angular velocity w is in the body frame).
dvec4 quat_rhs(const dvec4 &q, const dvec3 &w)
{
    double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0, dq1, dq2, dq3};
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

//Euler odes rhs assuming a generic, non principal axes aligned body frame.
//Angular velocity w, moment of inertia I and torque tau are in the body frame.
dvec3 euler_rhs_gen(const dvec3 &w, const dmat3 &I, const dvec3 &tau)
{
    double I00 = I[0][0], I01 = I[0][1], I02 = I[0][2], I11 = I[1][1], I12 = I[1][2], I22 = I[2][2];

    dmat3 Iinv; //inverse inertia matrix
    Iinv[0][0] = (I12*I12  - I11*I22)/(I02*I02*I11 - 2.0*I01*I02*I12 + I00*I12*I12 + I01*I01*I22 - I00*I11*I22);
    Iinv[0][1] = (-I02*I12 + I01*I22)/(I02*I02*I11 - 2.0*I01*I02*I12 + I01*I01*I22 + I00*(I12*I12 - I11*I22));
    Iinv[0][2] = (I02*I11  - I01*I12)/(I02*I02*I11 - 2.0*I01*I02*I12 + I00*I12*I12 + I01*I01*I22 - I00*I11*I22);
    Iinv[1][0] = (-I02*I12 + I01*I22)/(I02*I02*I11 - 2.0*I01*I02*I12 + I01*I01*I22 + I00*(I12*I12 - I11*I22));
    Iinv[1][1] = (I02*I02  - I00*I22)/(I02*I02*I11 - 2.0*I01*I02*I12 + I00*I12*I12 + I01*I01*I22 - I00*I11*I22);
    Iinv[1][2] = (-I01*I02 + I00*I12)/(I02*I02*I11 - 2.0*I01*I02*I12 + I01*I01*I22 + I00*(I12*I12 - I11*I22));
    Iinv[2][0] = (I02*I11  - I01*I12)/(I02*I02*I11 - 2.0*I01*I02*I12 + I00*I12*I12 + I01*I01*I22 - I00*I11*I22);
    Iinv[2][1] = (-I01*I02 + I00*I12)/(I02*I02*I11 - 2.0*I01*I02*I12 + I01*I01*I22 + I00*(I12*I12 - I11*I22));
    Iinv[2][2] = (I01*I01  - I00*I11)/(I02*I02*I11 - 2.0*I01*I02*I12 + I00*I12*I12 + I01*I01*I22 - I00*I11*I22);
     
    return dot(-Iinv, tau - cross(w, dot(I,w)));
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

#endif

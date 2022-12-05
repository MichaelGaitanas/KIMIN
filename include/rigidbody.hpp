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

#endif

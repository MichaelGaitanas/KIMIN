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

//Sphere-sphere collision detection criterion.
bool sphere_sphere_collision(const double distance, const double R1, const double R2)
{
    if (distance <= R1 + R2)
        return true;
    return false;
}

#endif

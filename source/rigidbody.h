#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include"typedef.h"

//Quaternion odes rhs for a rigid body. The angular velocity w is in the body frame.
dvec4 quat_rhs(const dvec4 &q, const dvec3 &w)
{
    //This comment is useless, but I wanted you to know that right now, I am listening to this song while writing the code.
    //https://www.youtube.com/watch?v=A_tA7G1Y8Ew&list=RDX2xpR5EoAGY&index=2&ab_channel=FrankKlepacki-Topic
    const double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    const double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    const double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    const double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0, dq1, dq2, dq3};
}

//Euler odes rhs for a rigid body, assuming I[][] is diagonal (principal axes frame).
//Angular velocity, moment of inertia and the torque are all in the body frame.
dvec3 euler_rhs(const dvec3 &w, const dmat3 &I, const dvec3 &torque)
{
    const double dw0 = (torque[0] + w[1]*w[2]*(I[1][1] - I[2][2]))/I[0][0];
    const double dw1 = (torque[1] + w[2]*w[0]*(I[2][2] - I[0][0]))/I[1][1];
    const double dw2 = (torque[2] + w[0]*w[1]*(I[0][0] - I[1][1]))/I[2][2];
    return {dw0, dw1, dw2};
}

#endif

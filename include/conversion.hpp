#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include<cmath>
#include<cstdio>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Convert a quaternion to a unit one.
dvec4 quat2unit(const dvec4 &q)
{
    return q/sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
}

//Convert a unit quaternion to Euler angles (roll, pitch, yaw).
dvec3 quat2ang(const dvec4 &q)
{
    //roll
    double roll = atan2( 2.0*(q[2]*q[3] + q[0]*q[1]), q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3] );
    //pitch
    double pitch, coeff = q[1]*q[3] - q[0]*q[2];
    if (-2.0*coeff >= 1.0)
        pitch = 2.0*pi;
    else if (-2.0*coeff <= -1.0)
        pitch = -2.0*pi;
    else
        pitch = asin(-2.0*coeff);
    //yaw
    double yaw = atan2( 2.0*(q[1]*q[2] + q[0]*q[3]), q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3] );
    return {roll, pitch, yaw};
}

//Convert a unit quaternion to rotation matrix (homogeneous expression).
dmat3 quat2mat(const dvec4 &q)
{
    double a11 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];
    double a12 = 2.0*(q[1]*q[2] - q[0]*q[3]);
    double a13 = 2.0*(q[1]*q[3] + q[0]*q[2]);
    double a21 = 2.0*(q[1]*q[2] + q[0]*q[3]);
    double a22 = q[0]*q[0] - q[1]*q[1] + q[2]*q[2] - q[3]*q[3];
    double a23 = 2.0*(q[2]*q[3] - q[0]*q[1]);
    double a31 = 2.0*(q[1]*q[3] - q[0]*q[2]);
    double a32 = 2.0*(q[2]*q[3] + q[0]*q[1]);
    double a33 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
    return {{{a11,a12,a13},
             {a21,a22,a23},
             {a31,a32,a33}}};
}

//Convert Euler angles (roll, pitch, yaw) to unit quaternion.
dvec4 ang2quat(const dvec3 &ang)
{
    double roll = ang[0], pitch = ang[1], yaw = ang[2];
    double cr = cos(0.5*roll);
    double sr = sin(0.5*roll);
    double cp = cos(0.5*pitch);
    double sp = sin(0.5*pitch);
    double cy = cos(0.5*yaw);
    double sy = sin(0.5*yaw);
    double q0 = cr*cp*cy + sr*sp*sy;
    double q1 = sr*cp*cy - cr*sp*sy;
    double q2 = cr*sp*cy + sr*cp*sy;
    double q3 = cr*cp*sy - sr*sp*cy;
    return {q0,q1,q2,q3};
}

//Convert a vector from the inertial to the body frame.
dvec3 iner2body(const dvec3 &viner, const dmat3 &A)
{
    return dot(transpose(A), viner);
}

//Convert a vector from the body to the inertial frame.
dvec3 body2iner(const dvec3 &vbody, const dmat3 &A)
{
    return dot(A, vbody);
}

#endif

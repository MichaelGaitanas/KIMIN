/* This header provides a collection of standalone functions for converting between common mathematical and physical quantities. */

#ifndef CONVERSION_H
#define CONVERSION_H

#include<cmath>
#include<cstdio>
#include<algorithm>

#include"constants.h"
#include"typedef.h"
#include"linalg.h"

//Convert a quaternion to a unit one.
dvec4 quat2unit(const dvec4 &q)
{
    return q/sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
}

//Convert a unit quaternion to Euler angles (roll, pitch, yaw).
dvec3 quat2ang(const dvec4 &q)
{
    //Roll :
    double roll = atan2( 2.0*(q[2]*q[3] + q[0]*q[1]), q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3] );

    //Pitch :
    double coeff = 2.0*(q[0]*q[2] - q[1]*q[3]);
    if (coeff > 1.0)
        coeff = 1.0;
    else if (coeff < -1.0)
        coeff = -1.0;
    double pitch = asin(coeff);

    //Yaw :
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

//Convert Euler angles (roll, pitch, yaw) to a unit quaternion.
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

//Convert Euler angles (roll, pitch, yaw) to a rotation matrix.
dmat3 ang2mat(const dvec3 &ang)
{
    double roll = ang[0], pitch = ang[1], yaw = ang[2];
    double cr = cos(roll);
    double sr = sin(roll);
    double cp = cos(pitch);
    double sp = sin(pitch);
    double cy = cos(yaw);
    double sy = sin(yaw);
    double a11 = cp*cy;
    double a12 = sr*sp*cy - sy*cr;
    double a13 = sr*sy + sp*cr*cy;
    double a21 = sy*cp;
    double a22 = sr*sp*sy + cr*cy;
    double a23 = -sr*cy + sp*sy*cr;
    double a31 = -sp;
    double a32 = sr*cp;
    double a33 = cr*cp;
    return {{{a11,a12,a13},
             {a21,a22,a23},
             {a31,a32,a33}}};
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

//Perform unsigned modulo 2*PI to the argument 'angle'.
double wrap_to_2pi(double angle)
{
    angle = fmod(angle, 2.0*PI);
    if (angle < 0.0)
        angle += 2.0*PI;
    return angle;
}

//Calculate the eccentric anomaly, given the mean anomaly and eccentricity (e < 1). The calculation is done by
//inverting Kepler's equation through a Newton-Raphson method.
double M2E(const double M, const double e)
{
    if (fabs(sin(M)) < 1e-15)
        return M;

    double E0, E;
    int iter = 0;
    E = E0 = M + 0.85*e*sin(M)/fabs(sin(M)); //Initial guess. Note : ( x/fabs(x) = sign(x) )
    do
    {
        E0 = E;
        E = E0 - (M - E0 + e*sin(E0))/(-1.0 + e*cos(E0));

        //Loop escape criterion.
        if (++iter > NR_MAX_ITERATIONS)
        {
            printf("Warning : In M2E(), N-R did not converge sufficiently. Returning as E the last E_i of the method (E_i = %.15lf [rad]) for which |E_i - E_(i-1)| = %.15e.\n", E, fabs(E-E0));
            return wrap_to_2pi(E);
        }
    }
    while (fabs(E - E0) > 1e-15);

    return wrap_to_2pi(E);
}

//Calculate the mean anomaly, given the eccentric anomaly and the eccentricity.
double E2M(const double E, const double e)
{
    double M = E - e*sin(E);
    return wrap_to_2pi(M);
}

//Calculate the hyperbolic anomaly, given the mean anomaly M and eccentricity (e > 1).
//Now it solves M = e*sinh(H) - H, using the Newton–Raphson method.
double M2H(const double M, const double e)
{
    if (fabs(M) < 1e-15)
        return M;

    double H0, H;
    int iter = 0;   
    H = H0 = M/(e - 1.0); //Initial guess, using linearization H0 ~ M/(e - 1).
    do
    {
        H0 = H;
        H = H0 - (e*sinh(H0) - H0 - M)/(e*cosh(H0) - 1.0);
        
        //Loop escape criterion.
        if (++iter > NR_MAX_ITERATIONS)
        {
            printf("Warning : In M2H(), N-R did not converge sufficiently. Returning as H the last H_i of the method (H_i = %.15lf [rad]) for which |H_i - H_(i-1)| = %.15e.\n", H, fabs(H-H0));
            return H;
        }
    }
    while (fabs(H - H0) > 1e-15);
    
    return H;
}

//Calculate the mean anomaly, given the hyperbolic anomaly and the eccentricity.
double H2M(const double H, const double e)
{
    double M = e*sinh(H) - H;
    return M; //Here we do not wrap in 2*PI, because the hyperbolic mean anomaly is not a periodic angle (unlike in the elliptical case).
}

//Convert Keplerian elements to Cartesian.
//Note : a must be nonzero, and e must be in [0,1) or (1,inf). Expect however high sensitivity when e is close to 1 from the right (e.g. e = 1.1), due to M2H().
dvec6 kep2cart(const dvec6 &kep, const double GM)
{
    //Extract into symbols.
    double a  = kep[0];
    double e  = kep[1]; 
    double i  = kep[2];
    double Om = kep[3];
    double w  = kep[4];
    double M  = kep[5];

    double f, p; //True anomaly and semi-latus rectum.
    if (e < 1.0) //Osculating ellipse.
    {
        if (a < 0.0)
            a = -a; //I forgive you and make it positive...
        
        double E = M2E(M, e); //Eccentric anomaly.
        f = 2.0*atan(sqrt((1.0 + e)/(1.0 - e))*tan(E/2.0));
        p = a*(1.0 - e*e);
    }
    else //Osculating hyperbola.
    {
        if (a > 0.0)
            a = -a; //Again I forgive you (a is negative in the hyperbola).

        double H = M2H(M, e); //Hyperbolic anomaly.
        f = 2.0*atan(sqrt((1.0 + e)/(e - 1.0))*tanh(H/2.0));
        p = -a*(e*e - 1.0);
    }

    double sqrt_GM_div_p = sqrt(GM/p);

    //Precompute trigonometric stuff.
    double sini  = sin(i),  cosi  = cos(i);
    double sinOm = sin(Om), cosOm = cos(Om);
    double sinw  = sin(w),  cosw  = cos(w);
    double sinf  = sin(f),  cosf  = cos(f);

    //Cartesian elements defined on the orbital plane (plz = plvz = 0).
    double plx = p*cosf/(1.0 + e*cosf);
    double ply = p*sinf/(1.0 + e*cosf);
    double plvx = -sqrt_GM_div_p*sinf;
    double plvy = sqrt_GM_div_p*(e + cosf);

    //Rotation matrix (3rd column is not needed since plz = plvz = 0).
    double A11 =  cosw*cosOm - sinw*cosi*sinOm;
    double A12 = -sinw*cosOm - cosw*cosi*sinOm;
    double A21 =  cosw*sinOm + sinw*cosi*cosOm;
    double A22 = -sinw*sinOm + cosw*cosi*cosOm;
    double A31 =  sinw*sini;
    double A32 =  cosw*sini;

    //Inertial Cartesian elements.
    double x  = A11*plx  + A12*ply;
    double y  = A21*plx  + A22*ply;
    double z  = A31*plx  + A32*ply;
    double vx = A11*plvx + A12*plvy;
    double vy = A21*plvx + A22*plvy;
    double vz = A31*plvx + A32*plvy;

    return {x,y,z,vx,vy,vz};
}

//Convert Cartesian elements to Keplerian.
dvec6 cart2kep(const dvec6 &cart, const double GM)
{
    double x  = cart[0];
    double y  = cart[1];
    double z  = cart[2];
    double vx = cart[3];
    double vy = cart[4];
    double vz = cart[5];

    double r = sqrt(x*x + y*y + z*z);
    double v2 = vx*vx + vy*vy + vz*vz;
    double rdotv = x*vx + y*vy + z*vz;

    //Eccentricity vector.
    double p1 = (v2 - GM/r)/GM;
    double p2 = -rdotv/GM;
    double ex = p1*x + p2*vx;
    double ey = p1*y + p2*vy;
    double ez = p1*z + p2*vz;

    //Angular momentum vector.
    double hx = y*vz - z*vy;
    double hy = z*vx - x*vz;
    double hz = x*vy - vx*y;
    double h = sqrt(hx*hx + hy*hy + hz*hz);
    
    //Ascending node vector.
    double nx = -hy;
    double ny = hx;
    double n = sqrt(nx*nx + ny*ny);

    //Semi-major axis (1st Keplerian element).
    double a = 1.0/(2.0/r - v2/GM);

    //Eccentricity (2nd Keplerian element).
    double e = sqrt(ex*ex + ey*ey + ez*ez);

    //Inclination (3rd Keplerian element).
    double cosi = hz/h;
    double i = acos(std::clamp(cosi, -1.0, 1.0));

    //Longitude of ascending node (4th Keplerian element).
    double Om;
    if (n > 1e-15)
    {
        double cosOm = nx/n;
        Om = acos(std::clamp(cosOm, -1.0, 1.0));
        if (ny < 0.0)
            Om = 2.0*PI - Om;
    }
    else
        Om = 0.0;

    //Argument of periapsis (5th Keplerian element).
    double w;
    if (e > 1e-15)
    {
        double cosw;
        if (n > 1e-15)
        {
            cosw = (nx*ex + ny*ey)/(e*n);
            w = acos(std::clamp(cosw, -1.0, 1.0));
            if (ez < 0.0)
                w = 2.0*PI - w;
        }
        else
        {
            cosw = ex/e;
            w = acos(std::clamp(cosw, -1.0, 1.0));
        }
    }
    else
        w = 0.0;

    //True anomaly.
    double f, cosf;
    if (n > 1e-15)
    {
        if (e > 1e-15)
        {
            cosf = (ex*x + ey*y + ez*z)/(e*r);
            f = acos(std::clamp(cosf, -1.0, 1.0));
            if (rdotv < 0.0)
                f = 2.0*PI - f;
        }
        else
        {
            cosf = (nx*x + ny*y)/(n*r);
            f = acos(std::clamp(cosf, -1.0, 1.0));
            if (-x*ny + y*nx < 0.0)
                f = 2.0*PI - f;
        }
    }
    else
    {
        cosf = x/r;
        f = acos(std::clamp(cosf, -1.0, 1.0));
        if (y < 0.0)
            f = 2.0*PI - f;
    }

    //Mean anomaly (6th Keplerian element).
    double M;
    if (e < 1.0) //Osculating ellipse.
    {
        //Eccentric anomaly calculation.
        double sinE = sin(f)*sqrt(1.0 - e*e)/(1.0 + e*cosf);
        double cosE = (e + cosf)/(1.0 + e*cosf);
        double E = atan2(sinE, cosE);
        M = E2M(E,e);
    }
    else //Osculating hyperbola.
    {
        double H = 2.0*atanh(sqrt((e - 1.0)/(e + 1.0))*tan(f/2.0));
        M = H2M(H,e);
    }
    
    return {a,e,i,Om,w,M};
}

//Convert cylindrical coordinates to Cartesian.
dvec3 cyl2cart(const dvec3 &cyl)
{
    //x = r*cos(thita)
    //y = r*sin(thita)
    //z = z
    return {cyl[0]*cos(cyl[1]), cyl[0]*sin(cyl[1]), cyl[2]};
}

//Convert Cartesian coordinates to cylindrical.
dvec3 cart2cyl(const dvec3 &cart)
{
    //r = sqrt(x^2 + y^2 + z^2)
    //thita = arctan(y/x)
    //z = z
    return {sqrt(cart[0]*cart[0] + cart[1]*cart[1] + cart[2]*cart[2]), atan2(cart[1],cart[0]), cart[2]};
}

//Convert spherical coordinates to Cartesian.
dvec3 spher2cart(const dvec3 &spher, bool latitude_origin_is_the_north_pole = true)
{
    double r   = spher[0];
    double lon = spher[1];
    double lat = spher[2];

    double x,y,z;
    if (latitude_origin_is_the_north_pole)
    {
        x = r*cos(lon)*sin(lat);
        y = r*sin(lon)*sin(lat);
        z = r*cos(lat);
    }
    else //Latitude's origin is the equator
    {
        x = r*cos(lon)*cos(lat);
        y = r*sin(lon)*cos(lat);
        z = r*sin(lat);
    }

    return {x,y,z};
}

//Convert Cartesian coordinates to spherical.
dvec3 cart2spher(const dvec3 &cart, bool latitude_origin_is_the_north_pole = true)
{
    double x = cart[0];
    double y = cart[1];
    double z = cart[2];

    double r,lon,lat;
    if (latitude_origin_is_the_north_pole)
    {
        r = sqrt(x*x + y*y + z*z);
        lon = atan2(y,x);
        lat = atan2(sqrt(x*x + y*y), z);
        //and the inverse transformation would be : { x = r*cos(lon)*sin(lat), y = r*sin(lon)*sin(lat), z = r*cos(lat) }
    }
    else //Latitude's origin is the equator
    {
        r = sqrt(x*x + y*y + z*z);
        lon = atan2(y,x);
        lat = atan2(z, sqrt(x*x + y*y));
        //and the inverse transformation would be : { x = r*cos(lon)*cos(lat), y = r*sin(lon)*cos(lat), z = r*sin(lat) }
    }

    return {r,lon,lat};
}

//Calculate the scalar term C[n][m], provided the normalized inertial integral tensor N[i][j][k].
//The term C[n][m] will be stage 1 normalized.
//This function is only meant to be called from integrals2stokes().
double Cnm_step(const int n, const int m, const dtens &N)
{
    double Cnm = 0.0;
    for (int p = 0; p < (int)(n/2) + 1; ++p)
    {
        for (int q = 0; q < (int)(m/2) + 1; ++q)
        {
            double coeff = pow(-1.0, p+q)*binomial(n,p)*binomial(2*n-2*p, n)*binomial(m, 2*q)*pochhammer(n-m-2*p+1,m);
            if ((int)coeff != 0)
            {
                double sum = 0.0;
                for (int vx = 0; vx < p+1; ++vx)
                {
                    for (int vy = 0; vy < p-vx+1; ++vy)
                    {
                        sum += (factorial(p)/( factorial(vx)*factorial(vy)*factorial(p-vx-vy) ))*N[m - 2*q + 2*vx][2*q + 2*vy][n - m - 2*vx - 2*vy];
                    }
                }
                Cnm += coeff*sum;
            }
        }
    }
    return pow(2.0, -n)*Cnm;
}

//Calculate the scalar term S[n][m], provided the normalized inertial integral tensor N[i][j][k].
//The term S[n][m] will be stage 1 normalized.
//This function is only meant to be called from integrals2stokes().
double Snm_step(const int n, const int m, const dtens &N)
{
    double Snm = 0.0;
    for (int p = 0; p < (int)(n/2) + 1; ++p)
    {
        for (int q = 0; q < (int)((m-1)/2) + 1; ++q)
        {
            double coeff = pow(-1.0, p+q)*binomial(n,p)*binomial(2*n-2*p, n)*binomial(m, 2*q+1)*pochhammer(n-m-2*p+1,m);
            if ((int)coeff != 0)
            {
                double sum = 0.0;
                for (int vx = 0; vx < p+1; ++vx)
                {
                    for (int vy = 0; vy < p-vx+1; ++vy)
                    {
                        sum += (factorial(p)/( factorial(vx)*factorial(vy)*factorial(p-vx-vy) ))*N[m - 2*q - 1 + 2*vx][2*q + 1 + 2*vy][n - m - 2*vx - 2*vy];
                    }
                }
                Snm += coeff*sum;
            }
        }
    }
    return pow(2.0, -n)*Snm;
}

//Convert the non normalized inertial integral tensor J[i][j][k] to the Stokes coefficient matrices C[n][m], S[n][m].
//If supernormalized = true, then stage 2 normalization (i.e. supernormalization) will be applied to C[n][m] and S[n][m].
//Else only stage 1 normalization will be applied to C[n][m] and S[n][m].
void integrals2stokes(const dtens &J, dmat &C, dmat &S, const double R0, bool supernormalized = true)
{
    int ord = J.size() - 1;

    //Normalization of the inertial integrals.
    dtens N(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) );
    for (int i = 0; i < ord + 1; ++i)
        for (int j = 0; j < ord + 1; ++j)
            for (int k = 0; k < ord + 1; ++k)
                N[i][j][k] = J[i][j][k]/pow(R0, i+j+k)/J[0][0][0];

    //Evaluation of the Stokes coefficients.
    C.clear();
    S.clear();
    for (int n = 0; n < ord + 1; ++n)
    {
        C.push_back(dvec{});
        S.push_back(dvec{});
        for (int m = 0; m < n + 1; ++m)
        {
            double supernormcoeff;
            if (supernormalized)
                supernormcoeff = sqrt( factorial(n+m)/((2.0 - kronecker(0,m))*(2.0*n + 1.0)*factorial(n-m)) );
            else
                supernormcoeff = 1.0;
            double auxcoeff = (2.0 - kronecker(0,m))*factorial(n-m)/factorial(n+m);
            C[n].push_back(supernormcoeff*auxcoeff*Cnm_step(n,m,N)); //C[n][m] = supernormcoeff*auxcoeff*Cnm_step(n,m,N)
            S[n].push_back(supernormcoeff*auxcoeff*Snm_step(n,m,N)); //S[n][m] = supernormcoeff*auxcoeff*Snm_step(n,m,N)
        }
    }
}

#endif

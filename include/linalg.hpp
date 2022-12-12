#ifndef LINALG_HPP
#define LINALG_HPP

#include<cmath>
#include<cstdio>
#include<limits>

#include"typedef.hpp"
#include"constant.hpp"

////////////////////////////////////////////////////////////////////////////////

//Define the operation v1 + v2 (v1,v2 are 3x1 vectors).
dvec3 operator+(const dvec3 &v1, const dvec3 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
}

//Define the operation v1 - v2 (v1,v2 are 3x1 vectors).
dvec3 operator-(const dvec3 &v1, const dvec3 &v2)
{
    return {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}

//Define the operation -v (v is 3x1 vector).
dvec3 operator-(const dvec3 &v)
{
    return {-v[0], -v[1], -v[2]};
}

//Define the operation c*v (c is scalar, v is 3x1 vector).
dvec3 operator*(const double c, const dvec3 &v)
{
    return {c*v[0], c*v[1], c*v[2]};
}

//Define the operation v*c (v is 3x1 vector, c is scalar).
dvec3 operator*(const dvec3 &v, const double c)
{
    return {v[0]*c, v[1]*c, v[2]*c};
}

//Define the operation v/c (v is c is 3x1 vector, c is scalar).
dvec3 operator/(const dvec3 &v, const double c)
{
    return {v[0]/c, v[1]/c, v[2]/c};
}

////////////////////////////////////////////////////////////////////////////////

//Define the operation v1 + v2 (v1,v2 are 4x1 vectors).
dvec4 operator+(const dvec4 &v1, const dvec4 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2], v1[3] + v2[3]};
}

//Define the operation v1 - v2 (v1,v2 are 4x1 vectors).
dvec4 operator-(const dvec4 &v1, const dvec4 &v2)
{
    return {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2], v1[3] - v2[3]};
}

//Define the operation -v (v is 4x1 vector).
dvec4 operator-(const dvec4 &v)
{
    return {-v[0], -v[1], -v[2], -v[3]};
}

//Define the operation c*v (c is scalar, v is 4x1 vector).
dvec4 operator*(const double c, const dvec4 &v)
{
    return {c*v[0], c*v[1], c*v[2], c*v[3]};
}

//Define the operation v*c (v is 4x1 vector, c is scalar).
dvec4 operator*(const dvec4 &v, const double c)
{
    return {v[0]*c, v[1]*c, v[2]*c, v[3]*c};
}

//Define the operation v/c (v is c is 4x1 vector, c is scalar).
dvec4 operator/(const dvec4 &v, const double c)
{
    return {v[0]/c, v[1]/c, v[2]/c, v[3]/c};
}

////////////////////////////////////////////////////////////////////////////////

//Define the operation A1 + A2 (A1,A2 are 3x3 matrices).
dmat3 operator+(const dmat3 &A1, const dmat3 &A2)
{
    return {{{A1[0][0] + A2[0][0], A1[0][1] + A2[0][1], A1[0][2] + A2[0][2]},
             {A1[1][0] + A2[1][0], A1[1][1] + A2[1][1], A1[1][2] + A2[1][2]},
             {A1[2][0] + A2[2][0], A1[2][1] + A2[2][1], A1[2][2] + A2[2][2]}}};
}

//Define the operation A1 - A2 (A1,A2 are 3x3 matrices).
dmat3 operator-(const dmat3 &A1, const dmat3 &A2)
{
    return {{{A1[0][0] - A2[0][0], A1[0][1] - A2[0][1], A1[0][2] - A2[0][2]},
             {A1[1][0] - A2[1][0], A1[1][1] - A2[1][1], A1[1][2] - A2[1][2]},
             {A1[2][0] - A2[2][0], A1[2][1] - A2[2][1], A1[2][2] - A2[2][2]}}};
}

//Define the operation -A (A is 3x3 matrix).
dmat3 operator-(const dmat3 &A)
{
    return {{{-A[0][0], -A[0][1], -A[0][2]},
             {-A[1][0], -A[1][1], -A[1][2]},
             {-A[2][0], -A[2][1], -A[2][2]}}};
}

//Transpose of a 3x3 matrix.
dmat3 transpose(const dmat3 &A)
{
    return {{{A[0][0], A[1][0], A[2][0]},
             {A[0][1], A[1][1], A[2][1]},
             {A[0][2], A[1][2], A[2][2]}}};
}

//Matrix-vector product A*v of a 3x3 matrix and a 3x1 vector.
dvec3 dot(const dmat3 &A, const dvec3 &v)
{
    return { A[0][0]*v[0] + A[0][1]*v[1] + A[0][2]*v[2],
             A[1][0]*v[0] + A[1][1]*v[1] + A[1][2]*v[2],
             A[2][0]*v[0] + A[2][1]*v[1] + A[2][2]*v[2] };
}

//Vector-matrix product v*A of a 1x3 vector and a 3x3 matrix.
dvec3 dot(const dvec3 &v, const dmat3 &A)
{
    return { v[0]*A[0][0] + v[1]*A[1][0] + v[2]*A[2][0],
             v[0]*A[0][1] + v[1]*A[1][1] + v[2]*A[2][1],
             v[0]*A[0][2] + v[1]*A[1][2] + v[2]*A[2][2] };
}

//Matrix-matrix product A*B of two 3x3 matrices.
dmat3 dot(const dmat3 &A, const dmat3 &B)
{
    return {{{ A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0],  A[0][0]*B[0][1] + A[0][1]*B[1][1] + A[0][2]*B[2][1],  A[0][0]*B[0][2] + A[0][1]*B[1][2] + A[0][2]*B[2][2] },
             { A[1][0]*B[0][0] + A[1][1]*B[1][0] + A[1][2]*B[2][0],  A[1][0]*B[0][1] + A[1][1]*B[1][1] + A[1][2]*B[2][1],  A[1][0]*B[0][2] + A[1][1]*B[1][2] + A[1][2]*B[2][2] },
             { A[2][0]*B[0][0] + A[2][1]*B[1][0] + A[2][2]*B[2][0],  A[2][0]*B[0][1] + A[2][1]*B[1][1] + A[2][2]*B[2][1],  A[2][0]*B[0][2] + A[2][1]*B[1][2] + A[2][2]*B[2][2] }}};
}

//Dot product of two 3x1 vectors.
double dot(const dvec3 &v1, const dvec3 &v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

//Cross product of two 3x1 vectors.
dvec3 cross(const dvec3 &v1, const dvec3 &v2)
{
    return { v1[1]*v2[2] - v1[2]*v2[1],
             v1[2]*v2[0] - v1[0]*v2[2],
             v1[0]*v2[1] - v1[1]*v2[0] };
}

//Length of a 3x1 vector.
double length(const dvec3 &v)
{
    return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

//Outer product of two 3x1 vectors.
dmat3 outer(const dvec3 &v1, const dvec3 &v2)
{
    return {{{ v1[0]*v2[0], v1[0]*v2[1], v1[0]*v2[2]},
             { v1[1]*v2[0], v1[1]*v2[1], v1[1]*v2[2]},
             { v1[2]*v2[0], v1[2]*v2[1], v1[2]*v2[2]}}};
}

////////////////////////////////////////////////////////////////////////////////

//Kronecker's delta function.
int kronecker(const int i, const int j)
{
    if (i == j)
        return 1;
    return 0;
}

//Factorial of n.
double factorial(const int n)
{
    if (n < 0)
        return std::numeric_limits<double>::quiet_NaN();

    if (n == 0)
        return 1.0;

    double fact = 1.0;
    for (int i = 1; i <= n; ++i)
        fact *= i;

    return fact;
}

//Binomial coefficient via the gamma function.
double binomial(const int n, const int k)
{
    double log_binom = std::lgamma(n + 1.0) - std::lgamma(n - k + 1.0) - std::lgamma(k + 1.0);
    return std::exp(log_binom);
}

//Pochhammer's symbol.
//(x)_n = x*(x + 1)*(x + 2)*...*(x + n - 1)
double pochhammer(const int x, const int n)
{
    double xn = 1.0;
    for (int i = 0; i < n; ++i)
        xn*= (x + i);
    return xn;
}

//Classical swap() function.
void swap(double &a, double &b)
{
    double temp = a;
    a = b;
    b = temp;
    return;
}

//Root of the linear equation a*x + b = 0, where a,b are reals.
dcomplex solve_linear(const double a, const double b)
{
    if (fabs(a) <= machine_zero)
    {
        //nan + i*0.0
        return dcomplex(std::numeric_limits<double>::quiet_NaN(), 0.0);
    }

    return dcomplex(-b/a, 0.0);
}

//Roots of the quadratic equation a*x^2 + b*x + c = 0, where a,b,c are reals.
dcomplexvec solve_quadratic(const double a, const double b, const double c)
{
    if (fabs(a) <= machine_zero)
    {
        return dcomplexvec({solve_linear(b,c)});
    }

    double D = b*b - 4.0*a*c;
    dcomplex x1 = (-b + sqrt(dcomplex(D)))/(2.0*a);
    dcomplex x2 = (-b - sqrt(dcomplex(D)))/(2.0*a);

    return dcomplexvec({x1,x2});
}

//Roots of the cubic equation a*x^3 + b*x^2 + c*x + d = 0, where a,b,c,d are reals.
dcomplexvec solve_cubic(const double a, const double b, const double c, const double d)
{
    if (fabs(a) <= machine_zero)
    {
        dcomplexvec sol = solve_quadratic(b,c,d);
        if (sol.size() == 2)
        {
            return dcomplexvec({sol[0], sol[1]});
        }
        return dcomplexvec({sol[0]});
    }    

    double D0 = b*b - 3.0*a*c;
    double D1 = 2.0*b*b*b - 9.0*a*b*c + 27.0*a*a*d;

    dcomplex C = pow( (D1 + sqrt(dcomplex(D1*D1 - 4.0*D0*D0*D0)))/2.0 , 1.0/3.0);
    if ( fabs(C.real()) <= machine_zero && fabs(C.imag()) <= machine_zero )
    {
        C = pow( (D1 - sqrt(dcomplex(D1*D1 - 4.0*D0*D0*D0)))/2.0 , 1.0/3.0);
        if ( fabs(C.real()) <= machine_zero && fabs(C.imag()) <= machine_zero )
        {
            return dcomplexvec({-b/(3.0*a), -b/(3.0*a), -b/(3.0*a)}); //triple solution
        }
    }

    dcomplex im = dcomplex(0.0, 1.0); //imaginary unit 'i'
    dcomplex coeff = ( -1.0 + im*sqrt(3.0) )/2.0;

    dcomplex x1 = -(b + C + D0/C )/(3.0*a);
    dcomplex x2 = -(b + coeff*C + D0/(coeff*C) )/(3.0*a);
    dcomplex x3 = -(b + coeff*coeff*C + D0/(coeff*coeff*C) )/(3.0*a);

    return dcomplexvec({x1,x2,x3});
}

//Roots of the quartic equation a*x^4 + b*x^3 + c*x^2 + d*x + e = 0, where a,b,c,d,e are reals.
dcomplexvec solve_quartic(const double a, const double b, const double c, const double d, const double e)
{
    if (fabs(a) <= machine_zero)
    {
        dcomplexvec sol = solve_cubic(b,c,d,e);
        if (sol.size() == 3)
        {
            return dcomplexvec({sol[0], sol[1], sol[2]});
        }
        if (sol.size() == 2)
        {
            return dcomplexvec({sol[0], sol[1]});
        }
        return dcomplexvec({sol[0]});
    }

    double A = -3.0*b*b/(8.0*a*a) + (double)c/a;
    double B = b*b*b/(8.0*a*a*a) - b*c/(2.0*a*a) + (double)d/a;
    double C = -3.0*b*b*b*b/(256.0*a*a*a*a) + c*b*b/(16.0*a*a*a) - b*d/(4.0*a*a) + (double)e/a;

    if (fabs(B) <= machine_zero)
    {
        dcomplex x1 = -b/(4.0*a) + sqrt( 0.5*(-A + sqrt(dcomplex(A*A - 4.0*C))) );
        dcomplex x2 = -b/(4.0*a) - sqrt( 0.5*(-A + sqrt(dcomplex(A*A - 4.0*C))) );
        dcomplex x3 = -b/(4.0*a) + sqrt( 0.5*(-A - sqrt(dcomplex(A*A - 4.0*C))) );
        dcomplex x4 = -b/(4.0*a) - sqrt( 0.5*(-A - sqrt(dcomplex(A*A - 4.0*C))) );
        return dcomplexvec({x1,x2,x3,x4});
    }

    double P = -A*A/12.0 - C;
    double Q = -A*A*A/108.0 + A*C/3.0 - B*B/8.0;
    dcomplex R = -Q/2.0 + sqrt(dcomplex(Q*Q/4.0 + P*P*P/27.0));
    dcomplex U = pow(R, 1.0/3.0);

    dcomplex Y;
    if (fabs(U.real()) <= machine_zero && fabs(U.imag()) <= machine_zero)
    {
        Y = -5.0*A/6.0 - pow(Q, 1.0/3.0);
    }
    else
    {
        Y = -5.0*A/6.0 + U - P/(3.0*U);
    }

    dcomplex W = sqrt(A + 2.0*Y);

    dcomplex x1 = -b/(4.0*a) + 0.5*( W + sqrt(-(3.0*A + 2.0*Y + 2.0*B/W)) );
    dcomplex x2 = -b/(4.0*a) + 0.5*(-W + sqrt(-(3.0*A + 2.0*Y - 2.0*B/W)) );
    dcomplex x3 = -b/(4.0*a) + 0.5*( W - sqrt(-(3.0*A + 2.0*Y + 2.0*B/W)) );
    dcomplex x4 = -b/(4.0*a) + 0.5*(-W - sqrt(-(3.0*A + 2.0*Y - 2.0*B/W)) );

    return dcomplexvec({x1,x2,x3,x4});
}

#endif

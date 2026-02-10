#include<cstdio>

#include"../../source/typedef.h"
#include"../../source/linalg.h"

int main()
{   
    //Find all the roots of [a*x + b = 0].
    double a = 0.1;
    double b = 13.1;
    dcomplex x_lin = solve_linear(a,b);
    printf("\n[%.16lf, %.16lf*i]\n\n", x_lin.real(), x_lin.imag());

    //Find all the roots of [a*x^2 + b*x + c = 0].
    a = -123.0;
    b = 22.0;
    double c = 763;
    dcomplexvec x_quad = solve_quadratic(a,b,c);
    printf("size = %ld\n", x_quad.size());
    printf("[%.16lf, %.16lf*i]\n",   x_quad[0].real(), x_quad[0].imag());
    printf("[%.16lf, %.16lf*i]\n\n", x_quad[1].real(), x_quad[1].imag());

    //Find all the roots of [a*x^3 + b*x^2 + c*x + d = 0].
    a = 123.0;
    b = 0.0;
    c = -0.00003;
    double d = 4;
    dcomplexvec x_cub = solve_cubic(a,b,c,d);
    printf("size = %ld\n", x_cub.size());
    printf("[%.16lf, %.16lf*i]\n",   x_cub[0].real(), x_cub[0].imag());
    printf("[%.16lf, %.16lf*i]\n",   x_cub[1].real(), x_cub[1].imag());
    printf("[%.16lf, %.16lf*i]\n\n", x_cub[2].real(), x_cub[2].imag());

    //Find all the roots of [a*x^4 + b*x^3 + c*x^2 + d*x + e = 0].
    a = 3210.0;
    b = 0.021;
    c = 321.0;
    d = -3111.0;
    double e = -5.1234;
    dcomplexvec x_quart = solve_quartic(a,b,c,d,e);
    printf("size = %ld\n", x_quart.size());
    printf("[%.16lf, %.16lf*i]\n",   x_quart[0].real(), x_quart[0].imag());
    printf("[%.16lf, %.16lf*i]\n",   x_quart[1].real(), x_quart[1].imag());
    printf("[%.16lf, %.16lf*i]\n",   x_quart[2].real(), x_quart[2].imag());
    printf("[%.16lf, %.16lf*i]\n\n", x_quart[3].real(), x_quart[3].imag());

    return 0;
}
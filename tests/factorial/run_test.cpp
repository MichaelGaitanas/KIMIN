#include<cstdio>
#include<cmath>

#include"../../source/linalg.h"

int main()
{
    double f;
    int arg = 8;

    f = factorial(arg);
    printf("factorial(%d) =  %.15lf\n", arg,f);

    f = std::tgamma(arg + 1);
    printf("tgamma(%d + 1) = %.15lf\n\n", arg,f);

    arg = 29;

    f = factorial(arg);
    printf("factorial(%d) =  %.15lf\n", arg,f);

    f = std::tgamma(arg + 1);
    printf("tgamma(%d + 1) = %.15lf\n\n", arg,f);

    arg = 290;

    f = factorial(arg);
    printf("factorial(%d) =  %.15lf\n", arg,f);

    f = std::tgamma(arg + 1);
    printf("tgamma(%d + 1) = %.15lf\n\n", arg,f);

    return 0;
}
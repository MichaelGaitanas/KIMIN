#include<cstdio>

int main()
{
    double j1 = 12;
    double j2 = 12.0;
    double j3 = 12.12345678923423482458734598789;
    double j4 = 12.123e15;
    double j5 = 12.0000;
    double j6 = 12.0000000123121413;

    printf("%.15e\n%.19g\n%.19g\n%.19g\n%.19g\n%.19g\n%.19g\n",j1,j2,j3,j3,j4,j5,j6);

    return 0;
}
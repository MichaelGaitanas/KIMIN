#include<cstdio>

#include"../../source/constant.h"
#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/file.h"
#include"../../source/conversion.h"

int main()
{
    FILE *fp = fopen("inputs_cart.txt","r");
    if (!fp)
    {
        fprintf(stderr, "Error : File 'inputs_cart.txt' was not found. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    double M1,M2;
    dvec6 cart; //x,y,z,vx,vy,vz
    if (find_assignment_operator(fp)) fscanf(fp, "%lf", &M1);
    if (find_assignment_operator(fp)) fscanf(fp, "%lf", &M2);
    for (int i = 0; i < 6; ++i)
        if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart[i]);
    fclose(fp);

    dvec6 kep = cart2kep(cart, G*(M1+M2));
    printf("a  = %.15lf\n", kep[0]);
    printf("e  = %.15lf\n", kep[1]);
    printf("i  = %.15lf\n", kep[2]*180.0/pi);
    printf("Om = %.15lf\n", kep[3]*180.0/pi);
    printf("w  = %.15lf\n", kep[4]*180.0/pi);
    printf("M  = %.15lf\n", kep[5]*180.0/pi);

    return 0;
}
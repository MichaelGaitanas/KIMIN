#include<cstdio>

#include"../../source/constant.h"
#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/file.h"
#include"../../source/conversion.h"

int main()
{
    FILE *fp = fopen("inputs_kep.txt","r");
    if (!fp)
    {
        fprintf(stderr, "Error : File 'inputs_kep.txt' was not found. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    double M1,M2;
    dvec6 kep; //a,e,i,Om,w,M
    if (find_assignment_operator(fp)) fscanf(fp, "%lf", &M1);
    if (find_assignment_operator(fp)) fscanf(fp, "%lf", &M2);
    for (int i = 0; i < 6; ++i)
        if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep[i]);
    fclose(fp);

    printf("\nM1 = %.15e\n", M1);
    printf("M2 = %.15e\n\n", M2);

    printf("a  = %.15lf\n", kep[0]);
    printf("e  = %.15lf\n", kep[1]);
    printf("i  = %.15lf\n", kep[2]);
    printf("Om = %.15lf\n", kep[3]);
    printf("w  = %.15lf\n", kep[4]);
    printf("M  = %.15lf\n\n", kep[5]);

    dvec6 cart = kep2cart({kep[0], kep[1], kep[2]*pi/180.0, kep[3]*pi/180.0, kep[4]*pi/180.0, kep[5]*pi/180.0}, G*(M1+M2));

    printf("x  = %.15lf\n", cart[0]);
    printf("y  = %.15lf\n", cart[1]);
    printf("z  = %.15lf\n", cart[2]);
    printf("vx = %.15lf\n", cart[3]);
    printf("vy = %.15lf\n", cart[4]);
    printf("vz = %.15lf\n\n", cart[5]);

    return 0;
}
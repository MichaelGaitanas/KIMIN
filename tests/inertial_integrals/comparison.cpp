#include<cstdio>
#include<filesystem>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/mascons.h"
#include"../../source/ellipsoid.h"

int main()
{
    mascons masc;
    masc.load_obj_file("../../obj/mascons/didymain2019_R04km_1661_fixed.obj");

    polyhedron poly;
    poly.load_obj_file("../../obj/polyhedra/didymain2019_R04km.obj");

    double M = 123456.0;
    int ord = 2;

    dtens Jmasc = masc.get_inertial_integrals(M,ord);
    dtens Jpoly = poly.get_inertial_integrals_ord2(M);

    for (size_t i = 0; i < Jmasc.size(); ++i)
        for (size_t j = 0; j < Jmasc[i].size(); ++j)
            for (size_t k = 0; k < Jmasc[i][j].size(); ++k)
                printf("J[%llu][%llu][%llu] :  %.10e,  %.10e\n",i,j,k, Jpoly[i][j][k], Jmasc[i][j][k]);

    return 0;
}
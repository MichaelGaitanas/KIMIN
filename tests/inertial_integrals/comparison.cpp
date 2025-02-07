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
    masc.load_obj_file("../../obj/mascons/patroclus_ellipsoid_48723_fixed.obj");

    polyhedron poly;
    poly.load_obj_file("../../obj/patroclus_ellipsoid.obj");

    double M = 123456.0;
    int ord = 3;


    dtens Jell = ell_integrals(M, dvec3{63.5, 58.5, 49.0}, ord);
    dtens Jmasc = masc.get_inertial_integrals(M,ord);
    dtens Jpoly = poly.get_inertial_integrals_ord3(M); //Don't forget the order!

    for (size_t i = 0; i < Jell.size(); ++i)
        for (size_t j = 0; j < Jell[i].size(); ++j)
            for (size_t k = 0; k < Jell[i][j].size(); ++k)
                printf("J[%llu][%llu][%llu] :  %.10e,  %.10e,  %.10e\n",i,j,k, Jell[i][j][k], Jpoly[i][j][k], Jmasc[i][j][k]);

    return 0;
}
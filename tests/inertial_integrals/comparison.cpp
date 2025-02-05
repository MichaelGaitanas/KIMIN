#include<cstdio>
#include<filesystem>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/mascons.h"
#include"../../source/ellipsoid.h"

int main()
{
    double M = 123456.0;

    dtens Jell = ell_integrals(M, dvec3{63.5, 58.5, 49.0}, 2);
    

    /*
    polyhedron poly;
    poly.load_obj_file("../../obj/patroclus_ellipsoid_fixed.obj");
    dtens Jpoly = poly.get_inertial_integrals_ord2(M);
    
    mascons masc;
    masc.load_obj_file("../../obj/mascons/patroclus_ellipsoid_48723_fixed.obj");
    dtens Jmasc = masc.get_inertial_integrals(M,2);
    */

    std::filesystem::create_directory("io");
    FILE *fp = fopen("io/comparison.txt","w");
    fclose(fp);

    return 0;
}
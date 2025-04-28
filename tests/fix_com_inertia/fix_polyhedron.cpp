#include<cstdio>
#include<filesystem>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"

int main()
{
    //Load a polyhedron.
    polyhedron poly;
    printf("Loading polyhedron... ");
    poly.load_obj_file("../../obj/polyhedra/bennu196k_R03km.obj");
    printf("Done.\n\n");

    std::filesystem::create_directory("io");

    dvec3 com = poly.get_com();
    printf("Initial com : \n");
    printf("[ %.15lf  %.15lf  %.15lf ]\n", com[0],com[1],com[2]);

    poly.set_com_zero();
    com = poly.get_com();
    printf("Final com : \n");
    printf("[ %.15e  %.15e  %.15e ]\n\n", com[0],com[1],com[2]);

    double M = 1.0;
    
    dmat3 iner = poly.get_inertia(M);
    printf("Initial inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    poly.set_inertia_diagonal();
    iner = poly.get_inertia(M);
    printf("Final inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    poly.export_obj_file_vf("io/bennu196k_R03km_poly_fixed.obj");

    return 0;
}
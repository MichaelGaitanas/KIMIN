#include<cstdio>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/mascons.h"

int main()
{
    polyhedron poly;
    poly.load_obj_file("../../obj/toutatis3k_radar.obj");
    
    mascons aster;
    printf("Generating mascons... ");
    aster.generate_from_polyhedron(poly, uvec3{25,26,27});
    printf("Done.\n\n");

    aster.export_points_to_obj("io/raycast_mascons.obj");

    double M = 1000000.0;

    dvec3 com = aster.get_com();
    printf("Initial com : \n");
    printf("[ %.15lf  %.15lf  %.15lf ]\n", com[0],com[1],com[2]);

    aster.eliminate_com(com);
    com = aster.get_com();
    printf("Final com : \n");
    printf("[ %.15e  %.15e  %.15e ]\n\n", com[0],com[1],com[2]);
    
    dmat3 iner = aster.get_inertia(M);
    printf("Initial inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    
    aster.diagonalize_inertia(iner);
    iner = aster.get_inertia(M);
    printf("Final inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    
    aster.export_points_to_obj("io/fixed_mascons.obj");

    return 0;
}
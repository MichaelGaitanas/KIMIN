#include<cstdio>
#include<filesystem>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/mascons.h"

int main()
{
    //Load a polyhedron.
    polyhedron poly;
    printf("Loading polyhedron... ");
    poly.load_obj_file("../../obj/toutatis3k_radar.obj");
    printf("Done.\n\n");
    
    //Generate the mascons model from the polyhedron.
    mascons masc;
    printf("Generating mascons... ");
    masc.generate_from_polyhedron(poly, uvec3{25,26,27});
    printf("Done.\n\n");

    std::filesystem::create_directory("io"); //Create directory if not present.

    char buffer[100];
    sprintf(buffer,"io/toutatis3k_radar_%llu.obj",masc.get_total());
    masc.export_obj_file(buffer);

    double M = 1234567.0;

    dvec3 com = masc.get_com();
    printf("Initial com : \n");
    printf("[ %.15lf  %.15lf  %.15lf ]\n", com[0],com[1],com[2]);

    masc.set_com_zero(com);
    com = masc.get_com();
    printf("Final com : \n");
    printf("[ %.15e  %.15e  %.15e ]\n\n", com[0],com[1],com[2]);
    
    dmat3 iner = masc.get_inertia(M);
    printf("Initial inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    masc.set_inertia_diagonal(iner);
    iner = masc.get_inertia(M);
    printf("Final inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    sprintf(buffer,"io/toutatis3k_radar_%llu_fixed.obj",masc.get_total());
    masc.export_obj_file(buffer);

    return 0;
}
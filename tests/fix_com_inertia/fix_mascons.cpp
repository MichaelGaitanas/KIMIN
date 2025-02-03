#include<cstdio>
#include<filesystem>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/mascons.h"

int main()
{
    //Load a polyhedron. We need it to generate the mascons.
    polyhedron poly;
    printf("Loading polyhedron... ");
    poly.load_obj_file("../../obj/kleopatra4k.obj");
    printf("Done.\n\n");
    
    mascons masc;
    printf("Generating mascons... ");
    masc.generate_from_polyhedron(poly, uvec3{71,71,71});
    printf("Done.\n\n");

    std::filesystem::create_directory("io");

    char buffer[100];
    sprintf(buffer,"io/kleopatra4k_%llu.obj",masc.get_total());
    masc.export_obj_file(buffer);

    dvec3 com = masc.get_com();
    printf("Initial com : \n");
    printf("[ %.15lf  %.15lf  %.15lf ]\n", com[0],com[1],com[2]);

    masc.set_com_zero();
    com = masc.get_com();
    printf("Final com : \n");
    printf("[ %.15e  %.15e  %.15e ]\n\n", com[0],com[1],com[2]);

    double M = 1.0; //Total mass of the mascons distro.
    
    dmat3 iner = masc.get_inertia(M);
    printf("Initial inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    masc.set_inertia_diagonal(M);
    iner = masc.get_inertia(M);
    printf("Final inertia : \n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    sprintf(buffer,"io/kleopatra4k_%llu_fixed.obj",masc.get_total());
    masc.export_obj_file(buffer);

    return 0;
}
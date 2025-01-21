#include<cstdio>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/rigidbody.h"

int main()
{
    polyhedron aster;
    aster.load_obj_file("../../obj/cylinder_rad1_h2.obj");
    //aster.eliminate_com_offset(aster.get_com()); //Pull the vertices so that ultimately the center of mass coincides with the cartesian origin (0,0,0).

    double M = 1.0e11; //Mass.
    dmat3 iner = aster.get_inertia(M);
    printf("Inertia initially :\n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);

    return 0;
}
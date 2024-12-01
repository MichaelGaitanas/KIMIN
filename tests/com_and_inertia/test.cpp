#include<cstdio>

#include"../source/typedef.h"
#include"../source/linalg.h"
#include"../source/polyhedron.h"
#include"../source/rigidbody.h"

int main()
{
    polyhedron aster;
    aster.load_obj_file("../obj/didymain2019.obj");
    aster.eliminate_com_offset(aster.get_com()); //Pull the vertices so that ultimately the center of mass coincides with the cartesian origin (0,0,0).

    double M = 1.0; //Mass.
    dmat3 iner = aster.get_inertia(M);
    printf("Inertia initially :\n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);
    
    dvec3 eigvals = inertia_eigvals(iner);
    printf("Inertia eigenvalues :\n");
    printf("l1 = %.15e\n",   eigvals[0]);
    printf("l2 = %.15e\n",   eigvals[1]);
    printf("l3 = %.15e\n\n", eigvals[2]);

    dmat3 eigvecs = inertia_eigvecs(iner);
    printf("Inertia eigenvectors :\n");
    printf("v1 = [ %.15e  %.15e  %.15e ]\n\nv2 = [ %.15e  %.15e  %.15e ]\n\nv3 = [ %.15e  %.15e  %.15e ]\n\n", eigvecs[0][0],eigvecs[0][1],eigvecs[0][2],
                                                                                                               eigvecs[1][0],eigvecs[1][1],eigvecs[1][2],
                                                                                                               eigvecs[2][0],eigvecs[2][1],eigvecs[2][2]);

    return 0;
}
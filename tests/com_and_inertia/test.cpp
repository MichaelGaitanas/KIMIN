#include<cstdio>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"
#include"../../source/rigidbody.h"

int main()
{
    polyhedron aster;
    aster.load_obj_file("../../obj/ellipsoid_a1_b04_c07_sculpted.obj");
    aster.eliminate_com_offset(aster.get_com()); //Pull the vertices so that ultimately the center of mass coincides with the cartesian origin (0,0,0).

    double M = 1.0e9; //Mass.
    dmat3 iner = aster.get_inertia(M);
    printf("Inertia :\n");
    printf("[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n[ %.15e  %.15e  %.15e ]\n\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                            iner[1][0],iner[1][1],iner[1][2],
                                                                                            iner[2][0],iner[2][1],iner[2][2]);

    FILE *fp = fopen("inertia.txt","w");
    fprintf(fp, "%.15e  %.15e  %.15e\n%.15e  %.15e  %.15e\n%.15e  %.15e  %.15e\n", iner[0][0],iner[0][1],iner[0][2],
                                                                                   iner[1][0],iner[1][1],iner[1][2],
                                                                                   iner[2][0],iner[2][1],iner[2][2]);
    fclose(fp);

    dvec3 eigvals = inertia_eigvals(iner);
    std::sort(eigvals.begin(), eigvals.end());
    printf("\nInertia eigenvalues :\n");
    printf("%.15lf  %.15lf  %.15lf\n\n", eigvals[0],eigvals[1],eigvals[2]);

    dmat3 eigmat = inertia_eigvecs(iner);
    printf("Inertia eigenvectors :\n");
    printf("[ %.15lf  %.15lf  %.15lf ]\n[ %.15lf  %.15lf  %.15lf ]\n[ %.15lf  %.15lf  %.15lf ]\n\n", eigmat[0][0],eigmat[0][1],eigmat[0][2],
                                                                                            eigmat[1][0],eigmat[1][1],eigmat[1][2],
                                                                                            eigmat[2][0],eigmat[2][1],eigmat[2][2]);

    dvec3 crossv1v2v = cross(dvec3{eigmat[0][0],eigmat[0][1],eigmat[0][2]}, dvec3{});

    return 0;
}
#include<cstdio>

#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/polyhedron.h"

int main()
{
    polyhedron poly;
    poly.load_obj_file("../../obj/plane.obj");
    printf("%d\n",poly.is_closed_manifold());

    return 0;
}
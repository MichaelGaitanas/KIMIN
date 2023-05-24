
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>
#include<iostream>

#include"../include/typedef.hpp"
#include"../include/obj.hpp"

int main()
{
    bvec vf = Obj::vf_status("../obj/gerasimenko256k.obj");
    Obj cube("../obj/gerasimenko256k.obj");

    dmatnx3 masc = cube.fill_with_masc({21,22,23});
    FILE *fp = fopen("mascons.obj","w");
    for (int i = 0; i < masc.size(); ++i)
        fprintf(fp, "v %lf %lf %lf\n", masc[i][0], masc[i][1], masc[i][2]);
    fclose(fp);


    return 0;
}
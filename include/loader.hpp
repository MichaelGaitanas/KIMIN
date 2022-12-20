#ifndef LOADER_HPP
#define LOADER_HPP

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.hpp"
#include"linalg.hpp"

//Load a nx3 dataset from a txt file.
dmatnx3 loadtxt3d(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    dmatnx3 data; double x,y,z;
    std::string row; const char *format = "%lf %lf %lf";
    while (getline(file, row))
    {
        sscanf(row.c_str(), format, &x, &y, &z);
        data.push_back({x,y,z});
    }

    
    //This is also correct.
    /*
    while (file >> x >> y >> z)
    {
        data.push_back({x,y,z});
    }
    */

    file.close();

    return data;
}

//Parse an obj file and write down if it contains [#,v,f,vn,vt].
bvec obj_contains_ns_v_f_vn_vt(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }
   
    bvec ns_v_f_vn_vt = {false, false, false, false, false};
    str row;
    while (getline(file, row))
    {
        if (row[0] == '#' && !ns_v_f_vn_vt[0])
        {
            ns_v_f_vn_vt[0] = true;
        }
        else if (row[0] == 'v' && row[1] == ' ' && !ns_v_f_vn_vt[1])
        {
            ns_v_f_vn_vt[1] = true;
        }
        else if (row[0] == 'f' && row[1] == ' ' && !ns_v_f_vn_vt[2])
        {
            ns_v_f_vn_vt[2] = true;
        }
        else if (row[0] == 'v' && row[1] == 'n' && row[2] == ' '  && !ns_v_f_vn_vt[3])
        {
            ns_v_f_vn_vt[3] = true;
        }
        else if (row[0] == 'v' && row[1] == 't' && row[2] == ' '  && !ns_v_f_vn_vt[4])
        {
            ns_v_f_vn_vt[4] = true;
        }
    }

    return ns_v_f_vn_vt;
}

//Load an obj file containing only vertices (format : 'v x y z').
dmatnx3 loadobjv(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    dmatnx3 verts; double x,y,z;
    std::string row; const char *format = "v %lf %lf %lf";
    while (getline(file, row))
    {
        if (row[0] == 'v' && row[1] == ' ') //then we have a vertex row
        {
            sscanf(row.c_str(), format, &x, &y, &z);
            verts.push_back({x,y,z});
        }
    }
    file.close();
    return verts;
}

//Load an obj file containing vertices and faces (format : 'v x y z', 'f i1 i2 i3').
void loadobjvf(const char *path, dmatnx3 &verts, imatnx3 &faces)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    //vertices
    double x,y,z;
    const char *vformat = "v %lf %lf %lf";

    //faces
    int i1,i2,i3;
    const char *fformat = "f %d %d %d";

    std::string row;
    while (getline(file, row))
    {
        if (row[0] == 'v' && row[1] == ' ') //then we have a vertex row
        {
            sscanf(row.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (row[0] == 'f' && row[1] == ' ') //then we have a face row
        {
            sscanf(row.c_str(), fformat, &i1,&i2,&i3);
            faces.push_back({i1-1, i2-1, i3-1});
        }
    }
    file.close();

    //verts[][] and faces[][] are now filled
    return;
}

//Load an obj file containing vertices faces and norms (format : 'v x y z', 'f i11//i12 i21//i22 i31//i32', 'vn nx ny nz').
void loadobjvfn(const char *path, dmatnx3 &verts, imatnx6 &faces, dmatnx3 &norms)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    //vertices
    double x,y,z;
    const char *vformat = "v %lf %lf %lf";

    //norms
    double nx,ny,nz;
    const char *nformat = "vn %lf %lf %lf";

    //faces
    int i11,i12, i21,i22, i31,i32;
    const char *fformat = "f %d//%d %d//%d %d//%d";

    std::string row;
    while (getline(file, row))
    {
        if (row[0] == 'v' && row[1] == ' ') //then we have a vertex row
        {
            sscanf(row.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (row[0] == 'v' && row[1] == 'n' && row[2] == ' ') //then we have a norm row
        {
            sscanf(row.c_str(), nformat, &nx, &ny, &nz);
            norms.push_back({nx,ny,nz});
        }
        else if (row[0] == 'f' && row[1] == ' ') //then we have a face row
        {
            sscanf(row.c_str(), fformat, &i11,&i12, &i21,&i22, &i31,&i32);
            faces.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
        }
    }
    file.close();

    //verts[][], norms[][] and faces[][] are now filled
    return;
}

#endif
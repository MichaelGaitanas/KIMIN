#ifndef LOADER_HPP
#define LOADER_HPP

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.hpp"
#include"linalg.hpp"

//Traverse an obj file and write down in a boolean vector if it contains [v,f,n,t].
bvec checkobj(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }
    
    bvec vfnt = {false, false, false, false}; //initially assuming that the obj file contains nothing
    str row;
    while (getline(file, row))
    {
        if (row[0] == 'v' && row[1] == ' ' && !vfnt[0]) //then we have a vertex row
            vfnt[0] = true;
        else if (row[0] == 'f' && row[1] == ' ' && !vfnt[1]) //then we have a face row
            vfnt[1] = true;
        else if (row[0] == 'v' && row[1] == 'n' && row[2] == ' '  && !vfnt[2]) //then we have a normal row
            vfnt[2] = true;
        else if (row[0] == 'v' && row[1] == 't' && row[2] == ' '  && !vfnt[3]) //then we have a texture row
            vfnt[3] = true;
    }
    file.close();

    return vfnt;
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
    str row; const char *format = "v %lf %lf %lf";
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

    str row;
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

    str row;
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

//Load an obj file containing vertices faces and textures (format : 'v x y z', 'f i11/i12 i21/i22 i31/i32', 'vt tx ty').
void loadobjvft(const char *path, dmatnx3 &verts, imatnx6 &faces, dmatnx2 &texs)
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

    //textures
    double tx,ty;
    const char *tformat = "vt %lf %lf";

    //faces
    int i11,i12, i21,i22, i31,i32;
    const char *fformat = "f %d/%d %d/%d %d/%d";

    str row;
    while (getline(file, row))
    {
        if (row[0] == 'v' && row[1] == ' ') //then we have a vertex row
        {
            sscanf(row.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (row[0] == 'v' && row[1] == 't' && row[2] == ' ') //then we have a texture row
        {
            sscanf(row.c_str(), tformat, &tx, &ty);
            texs.push_back({tx,ty});
        }
        else if (row[0] == 'f' && row[1] == ' ') //then we have a face row
        {
            sscanf(row.c_str(), fformat, &i11,&i12, &i21,&i22, &i31,&i32);
            faces.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
        }
    }
    file.close();

    //verts[][], texs[][] and faces[][] are now filled
    return;
}

//Load an obj file containing vertices faces, norms and textures (format : 'v x y z', 'f i11/i12/i13 i21/i22/i23 i31/i32/i33', 'vn nx ny nz', 'vt tx ty').
void loadobjvfnt(const char *path, dmatnx3 &verts, imatnx9 &faces, dmatnx3 &norms, dmatnx2 &texs)
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

    //textures
    double tx,ty;
    const char *tformat = "vt %lf %lf";

    //faces
    int i11,i12,i13, i21,i22,i23, i31,i32,i33;
    const char *fformat = "f %d/%d/%d %d/%d/%d %d/%d/%d";

    str row;
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
        else if (row[0] == 'v' && row[1] == 't' && row[2] == ' ') //then we have a texture row
        {
            sscanf(row.c_str(), tformat, &tx, &ty);
            texs.push_back({tx,ty});
        }
        else if (row[0] == 'f' && row[1] == ' ') //then we have a face row
        {
            sscanf(row.c_str(), fformat, &i11,&i12,&i13, &i21,&i22,&i23, &i31,&i32,&i33);
            faces.push_back({i11-1, i12-1, i13-1, i21-1, i22-1, i23-1, i31-1, i32-1, i33-1});
        }
    }
    file.close();

    //verts[][], norms[][], texs[][] and faces[][] are now filled
    return;
}

#endif
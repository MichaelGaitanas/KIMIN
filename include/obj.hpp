#ifndef OBJ_HPP
#define OBJ_HPP

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.hpp"

//Traverse an obj file and write down in a boolean vector if it contains [v,f,n,t] elements.
bvec checkobj(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }
    
    bvec vfnt = {false, false, false, false}; //initially assuming that the obj file contains nothing
    str line;
    while (getline(file, line))
    {
        if (line[0] == 'v' && line[1] == ' ' && !vfnt[0]) //then we have a vertex line for the first time
            vfnt[0] = true;
        else if (line[0] == 'f' && line[1] == ' ' && !vfnt[1]) //then we have a face line for the first time
            vfnt[1] = true;
        else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' '  && !vfnt[2]) //then we have a normal line for the first time
            vfnt[2] = true;
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' '  && !vfnt[3]) //then we have a texture line for the first time
            vfnt[3] = true;
    }
    file.close();

    return vfnt;
}

//Load the vertices (format : 'v x y z') from an obj file.
dmatnx3 loadobjv(const char *path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    dmatnx3 verts;
    double x,y,z;
    const char *format = "v %lf %lf %lf";

    str line;
    while (getline(file, line))
    {
        if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
        {
            sscanf(line.c_str(), format, &x, &y, &z);
            verts.push_back({x,y,z});
        }
    }
    file.close();
    return verts;
}

//Load the vertices and the faces (format : 'v x y z', 'f i1 i2 i3') from an obj file.
void loadobjvf(const char *path, dmatnx3 &verts, imatnx3 &faces)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    verts.clear();
    faces.clear();

    //vertices
    double x,y,z;
    const char *vformat = "v %lf %lf %lf";

    //faces
    int i1,i2,i3;
    const char *fformat = "f %d %d %d";

    str line;
    while (getline(file, line))
    {
        if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
        {
            sscanf(line.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (line[0] == 'f' && line[1] == ' ') //then we have a face line
        {
            sscanf(line.c_str(), fformat, &i1,&i2,&i3);
            faces.push_back({i1-1, i2-1, i3-1});
        }
    }
    file.close();

    //verts[][] and faces[][] are now filled
    return;
}

//Load the vertices, the faces and the norms (format : 'v x y z', 'f i11//i12 i21//i22 i31//i32', 'vn nx ny nz') from an obj file.
void loadobjvfn(const char *path, dmatnx3 &verts, imatnx6 &faces, dmatnx3 &norms)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    verts.clear();
    faces.clear();
    norms.clear();

    //vertices
    double x,y,z;
    const char *vformat = "v %lf %lf %lf";

    //norms
    double nx,ny,nz;
    const char *nformat = "vn %lf %lf %lf";

    //faces
    int i11,i12, i21,i22, i31,i32;
    const char *fformat = "f %d//%d %d//%d %d//%d";

    str line;
    while (getline(file, line))
    {
        if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
        {
            sscanf(line.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') //then we have a norm line
        {
            sscanf(line.c_str(), nformat, &nx, &ny, &nz);
            norms.push_back({nx,ny,nz});
        }
        else if (line[0] == 'f' && line[1] == ' ') //then we have a face line
        {
            sscanf(line.c_str(), fformat, &i11,&i12, &i21,&i22, &i31,&i32);
            faces.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
        }
    }
    file.close();

    //verts[][], norms[][] and faces[][] are now filled
    return;
}

//Load the vertices, the faces and texs (format : 'v x y z', 'f i11/i12 i21/i22 i31/i32', 'vt tx ty') from an obj file.
void loadobjvft(const char *path, dmatnx3 &verts, imatnx6 &faces, dmatnx2 &texs)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    verts.clear();
    faces.clear();
    texs.clear();

    //vertices
    double x,y,z;
    const char *vformat = "v %lf %lf %lf";

    //textures
    double tx,ty;
    const char *tformat = "vt %lf %lf";

    //faces
    int i11,i12, i21,i22, i31,i32;
    const char *fformat = "f %d/%d %d/%d %d/%d";

    str line;
    while (getline(file, line))
    {
        if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
        {
            sscanf(line.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') //then we have a texture line
        {
            sscanf(line.c_str(), tformat, &tx, &ty);
            texs.push_back({tx,ty});
        }
        else if (line[0] == 'f' && line[1] == ' ') //then we have a face line
        {
            sscanf(line.c_str(), fformat, &i11,&i12, &i21,&i22, &i31,&i32);
            faces.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
        }
    }
    file.close();

    //verts[][], texs[][] and faces[][] are now filled
    return;
}

//Load the vertices the faces, the norms and texs (format : 'v x y z', 'f i11/i12/i13 i21/i22/i23 i31/i32/i33', 'vn nx ny nz', 'vt tx ty') from an obj file.
void loadobjvfnt(const char *path, dmatnx3 &verts, imatnx9 &faces, dmatnx3 &norms, dmatnx2 &texs)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("'%s' not found. Exiting...\n", path);
        exit(EXIT_FAILURE);
    }

    verts.clear();
    faces.clear();
    norms.clear();
    texs.clear();

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

    str line;
    while (getline(file, line))
    {
        if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
        {
            sscanf(line.c_str(), vformat, &x, &y, &z);
            verts.push_back({x,y,z});
        }
        else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') //then we have a norm line
        {
            sscanf(line.c_str(), nformat, &nx, &ny, &nz);
            norms.push_back({nx,ny,nz});
        }
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') //then we have a texture line
        {
            sscanf(line.c_str(), tformat, &tx, &ty);
            texs.push_back({tx,ty});
        }
        else if (line[0] == 'f' && line[1] == ' ') //then we have a face line
        {
            sscanf(line.c_str(), fformat, &i11,&i12,&i13, &i21,&i22,&i23, &i31,&i32,&i33);
            faces.push_back({i11-1, i12-1, i13-1, i21-1, i22-1, i23-1, i31-1, i32-1, i33-1});
        }
    }
    file.close();

    //verts[][], norms[][], texs[][] and faces[][] are now filled
    return;
}

#endif
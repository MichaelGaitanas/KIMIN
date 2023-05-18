#ifndef OBJ_HPP
#define OBJ_HPP

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.hpp"
#include"polyhedron.hpp"

class Obj
{

public:

    str path;
    bvec vf;
    dmatnx3 verts;
    imatnx3 faces;
    dmatnx3 norms;

    Obj(const char *cpath) : path(cpath),
                             vf({false, false})
    {
        std::ifstream file(cpath);
        if (!file.is_open())
        {
            printf("'%s' not found. Exiting...\n", cpath);
            exit(EXIT_FAILURE); //Handle it better... Don't just kill the whole app.
        }
        
        //Traverse the obj file and write down in the vf[] vector if it contains [v,f] elements.
        str line;
        while (getline(file, line))
        {
            if (line[0] == 'v' && line[1] == ' ' && !vf[0]) //then we have a vertex line for the first time
                vf[0] = true;
            else if (line[0] == 'f' && line[1] == ' ' && !vf[1]) //then we have a face line for the first time
                vf[1] = true;
        }
        file.close();
    }

    //In case we want to load only the vertices, we call the following function.
    void loadv()
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            printf("'%s' not found. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        verts.clear();

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
        return;
    }

    void loadvf()
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

        norms = poly_norms(verts, faces);
        //norms[][] is filled too

        return;
    }

    //what?
    void loadvfn(const char *path, dmatnx3 &verts, imatnx6 &faces, dmatnx3 &norms)
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

};







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
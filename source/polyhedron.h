#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.h"
#include"constant.h"
#include"linalg.h"

class polyhedron
{
public:
    dmatnx3 verts;
    umatnx3 faces;
    dmatnx3 norms;
    
    //Load the .obj file assuming it has the classical form 'v x y z' and 'f i j k'.
    polyhedron(const char *path)
    {
        verts.clear();
        faces.clear();

        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            //This must never happen coz the gui will pre-detect all available .obj files in the obj/ directory.
            fprintf(stderr, "Error : '%s' could not be opened. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        double x,y,z; //Vertices.
        unsigned int vi1,vi2,vi3; //Faces (vertex indices).

        str line;
        while (getline(objfile, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                sscanf(line.c_str(), "v %lf %lf %lf", &x, &y, &z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'f' && line[1] == ' ') //Then we have a face line.
            {
                sscanf(line.c_str(), "f %u %u %u", &vi1,&vi2,&vi3);
                faces.push_back({vi1-1, vi2-1, vi3-1});
            }
        }
        objfile.close();
        //verts[][] and faces[][] are now filled
    }

    //Generate the polyhderon's normals.
    void gen_norms()
    {
        norms.clear();
        norms.resize(faces.size());
        dvec3 perp;
        for (size_t i = 0; i < norms.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            perp = cross(p1-p0, p2-p1);
            norms[i] = perp/length(perp);
        }
        //norms[][] is now updated.
    }
};

#endif
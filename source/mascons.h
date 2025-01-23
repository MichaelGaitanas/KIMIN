#ifndef MASCONS_H
#define MASCONS_H

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.h"
#include"constant.h"
#include"linalg.h"
#include"rigidbody.h"

class mascons
{
private:
    dmatnx3 verts;

public:
    //Load an .obj file, exclusively with the format 'v x y z' (mascons).
    void load_obj_file(const char *path)
    {
        verts.clear();

        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            fprintf(stderr, "Error : '%s' could not be opened. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        double x,y,z; //Vertices.

        str line;
        while (getline(objfile, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                sscanf(line.c_str(), "v %lf %lf %lf", &x, &y, &z);
                verts.push_back({x,y,z});
            }
        }
        objfile.close();
    }

    dvec3 get_com()
    {
        dvec3 com = {0.0,0.0,0.0};
        for (int i = 0; i < verts.size(); ++i)
            com = com + verts[i];

        return com/verts.size();
    }

    dmat3 get_inertia(const double M)
    {
        double m = (double)M/verts.size(); //Mass of each mascon.

        double Ixx,Iyy,Izz,Ixy,Ixz,Iyz;
        Ixx = Iyy = Izz = Ixy = Ixz = Iyz = 0.0;

        for (int i = 0; i < verts.size(); ++i)
        {
            Ixx += ( pow(verts[i][1], 2) + pow(verts[i][2], 2) ); //y[i]^2 + z[i]^2
            Iyy += ( pow(verts[i][0], 2) + pow(verts[i][2], 2) ); //x[i]^2 + z[i]^2
            Izz += ( pow(verts[i][0], 2) + pow(verts[i][1], 2) ); //x[i]^2 + y[i]^2
            Ixy -= verts[i][0]*verts[i][1]; //-x[i]*y[i]
            Ixz -= verts[i][0]*verts[i][2]; //-x[i]*z[i]
            Iyz -= verts[i][1]*verts[i][2]; //-y[i]*z[i]
        }

        //Ixy = Iyx, Ixz = Izx, Iyz = Izy by definition.
        return {{{m*Ixx, m*Ixy, m*Ixz},
                 {m*Ixy, m*Iyy, m*Iyz},
                 {m*Ixz, m*Iyz, m*Izz}}};
    }

    //Shift the center of mass of the polyhedron, so that it coincides with O(0,0,0).
    void eliminate_com_offset(const dvec3 &com)
    {
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = verts[i] - com;
    }

    //Rotate the vertices, so that the inertia matrix becomes diagonal.
    void align_principal_axes_to_basis(const double M)
    {
        dmat3 I = get_inertia(M);

        dmat3 eigvecs = inertia_eigvecs(I); //Three real and normalized vectors that form a right handed Cartesian basis.

        //Multiply each vertex vector with the rotation matrix.
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = dot(eigvecs, verts[i]);

        //Now the vertices are correct. Vertex connectivity (face indices) should remain the same.
    
        //But the norms are wrong! They must be recomputed.
        norms_exist = false;
        gen_norms();
    }

    
};

#endif
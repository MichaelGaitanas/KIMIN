#ifndef OBJ_HPP
#define OBJ_HPP

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"

class Obj
{

public:

    dmatnx3 verts;
    imatnx3 faces;
    dmatnx3 norms;

    //This function will be used only after the 'Run' button to check if the .obj file is ok to be loaded.
    static bvec vf_status(const char *path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            printf("'%s' not found. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }
        
        //Traverse the obj file and write down in the vf[] vector if it contains 'v x y z' and 'f i j k' lines.
        str line;
        bvec vf = {false, false};
        while (getline(file, line))
        {
            if (line[0] == 'v' && line[1] == ' ' && !vf[0]) //then we have a vertex line for the first time
                vf[0] = true;
            else if (line[0] == 'f' && line[1] == ' ' && !vf[1]) //then we have a face line for the first time
                vf[1] = true;
        }
        file.close();

        return vf; //and call it like : bvec vf = Obj::vf_status(path); WITHOUT having an Obj instance of the class
    }

    //Constructor : Load the .obj file assuming it has the classical form 'v x y z' and 'f i j k'.
    //Any other .obj content (comments, normals, textures, etc...) is ignored.
    Obj(const char *path)
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

        //Finally, compute the normals.
        norms.resize(faces.size());
        dvec3 perp;
        for (int i = 0; i < norms.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            perp = cross(p1-p0, p2-p1);
            norms[i] = perp/length(perp);
        }
    }

    //Decide whether a point r(x,y,z) is inside the obj polyhedron or not by raycasting.
    bool is_inside(const dvec3 &r)
    {
        int intersections = 0;

        //loop through all the triangulated faces in search for intersection
        for (int j = 0; j < faces.size(); ++j)
        {
            //define the triangle j from 3 points p0,p1,p2
            dvec3 p0 = verts[faces[j][0]];
            dvec3 p1 = verts[faces[j][1]];
            dvec3 p2 = verts[faces[j][2]];
            dvec3 pj = { p0[0] + ( (p0[1] - r[1])*norms[j][1] + (p0[2] - r[2])*norms[j][2] )/norms[j][0], //This is very dangerous...
                         r[1],
                         r[2] };

            //form the following 3 triangles and calculate their area
            double Aj01 = 0.5*length(cross(p0-pj, p1-p0)); //pj -> p0 -> p1
            double Aj12 = 0.5*length(cross(p1-pj, p2-p1)); //pj -> p1 -> p2
            double Aj20 = 0.5*length(cross(p2-pj, p0-p2)); //pj -> p2 -> p0
            double A012 = 0.5*length(cross(p1-p0, p2-p1)); //p0 -> p1 -> p2

            //if the sum of the 3 areas is equal to the area of the surface triangle, then pj sits upon the surface of the triangle.
            if ( fabs(Aj01 + Aj12 + Aj20 - A012) <= machine_zero && pj[0] > r[0] )
                ++intersections;
        }
        
        //odd  : (x,y,z) is inside the obj
        //even : (x,y,z) is outside the obj
        if (intersections%2 == 1)
            return true;

        return false;
    }

    //Fill the obj polyhedron(verts, faces) with mascons assuming 'grid_reso' Cartesian grid resolution.
    dmatnx3 fill_with_masc(const ivec3 &grid_reso)
    {
        dvec3 rmin = verts[0], rmax = verts[0];
        for (int i = 1; i < verts.size(); ++i)
        {
            if (verts[i][0] < rmin[0]) rmin[0] = verts[i][0];
            if (verts[i][1] < rmin[1]) rmin[1] = verts[i][1];
            if (verts[i][2] < rmin[2]) rmin[2] = verts[i][2];

            if (verts[i][0] > rmax[0]) rmax[0] = verts[i][0];
            if (verts[i][1] > rmax[1]) rmax[1] = verts[i][1];
            if (verts[i][2] > rmax[2]) rmax[2] = verts[i][2];
        }

        double xmin = rmin[0] - 100.0*machine_zero;
        double ymin = rmin[1] - 200.0*machine_zero;
        double zmin = rmin[2] - 300.0*machine_zero;

        double xmax = rmax[0] + 400.0*machine_zero;
        double ymax = rmax[1] + 500.0*machine_zero;
        double zmax = rmax[2] + 600.0*machine_zero;

        dmatnx3 masc;
        for (int i = 0; i < grid_reso[0]; ++i)
        {
            double x = xmin + i*(xmax - xmin)/((double)grid_reso[0] - 1.0);
            for (int j = 0; j < grid_reso[1]; ++j)
            {
                double y = ymin + j*(ymax - ymin)/((double)grid_reso[1] - 1.0);
                for (int k = 0; k < grid_reso[2]; ++k)
                {
                    double z = zmin + k*(zmax - zmin)/((double)grid_reso[2] - 1.0);
                    dvec3 r = {x,y,z}; //current point of the grid
                    if (is_inside(r))
                        masc.push_back(r);
                }
            }
        }
        return masc;
    }
};

#endif
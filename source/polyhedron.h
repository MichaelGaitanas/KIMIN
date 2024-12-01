#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include"typedef.h"
#include"constant.h"
#include"linalg.h"
#include"rigidbody.h"

class polyhedron
{
private:
    bool norms_exist;
    bool vol_exists;
    dmatnx3 verts;
    umatnx3 faces;
    dmatnx3 norms;
    double vol; //Polyhedron's total volume.

public:
    //Load the .obj file assuming it has the classical form 'v x y z' and 'f i j k'.
    void load_obj_file(const char *path)
    {
        norms_exist = false;
        vol_exists = false;
        verts.clear();
        faces.clear();
        norms.clear();

        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            //This must never happen coz the gui will pre-detect all available .obj files in the obj/ directory.
            fprintf(stderr, "Error : '%s' could not be opened. Exiting...\n", path);
            exit(EXIT_FAILURE); //I will fix that...
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
    }

    //Generate the polyhderon's normals.
    void gen_norms()
    {
        if (norms_exist)
            return;
        
        norms.clear();
        norms.resize(faces.size());
        dvec3 perp;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            perp = cross(p1-p0, p2-p1);
            norms[i] = perp/length(perp);
        }
        norms_exist = true;
    }

    //Polyhderon's volume. This function is essential for the inertial integrals evaluation.
    double get_vol()
    {
        if (vol_exists)
            return vol;

        gen_norms(); //This will only evaluate the norms if norms_exist is false.

        vol = 0.0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            double d = p0[0]*norms[i][0] + p0[1]*norms[i][1] + p0[2]*norms[i][2]; //x*nx + y*ny + z*nz - d = 0 (plane equation)
            double A = 0.5*length(cross(p1-p0, p2-p1));
            vol += d*A/3.0;
        }
        vol_exists = true;
        return vol;
    }

    //Calculate the center of mass of an arbitrary homogeneous closed surface polyhedron.
    dvec3 get_com()
    {
        gen_norms(); //This will only evaluate the norms if norms_exist is false.

        dvec3 com = {0.0,0.0,0.0};
        for (size_t i = 0; i < faces.size(); ++i)
        {
            double x1 = verts[faces[i][0]][0];
            double y1 = verts[faces[i][0]][1];
            double z1 = verts[faces[i][0]][2];
            
            double x2 = verts[faces[i][1]][0];
            double y2 = verts[faces[i][1]][1];
            double z2 = verts[faces[i][1]][2];

            double x3 = verts[faces[i][2]][0];
            double y3 = verts[faces[i][2]][1];
            double z3 = verts[faces[i][2]][2];

            double x21 = x2 - x1;
            double x31 = x3 - x1;
            double y21 = y2 - y1;
            double y31 = y3 - y1;
            double z21 = z2 - z1;
            double z31 = z3 - z1;

            double nx = norms[i][0];
            double ny = norms[i][1];
            double nz = norms[i][2];
            double d = x1*nx + y1*ny + z1*nz; //x*nx + y*ny + z*nz - d = 0 (plane equation)
            
            if (fabs(nz) > machine_zero)
            {
                double Jacxy = fabs(x21*y31 - x31*y21);
                double coeffnz = d*Jacxy/fabs(nz);
                com[0] += coeffnz*(3.0*x1 + x21 + x31)/6.0;
                com[1] += coeffnz*(3.0*y1 + y21 + y31)/6.0;
                com[2] += coeffnz*(3.0*d - nx*(3.0*x1 + x21 + x31) - ny*(3.0*y1 + y21 + y31))/(6.0*nz);
            }
            else
            {
                if (fabs(ny) > machine_zero)
                {
                    double Jacxz = fabs(x21*z31 - x31*z21);
                    double coeffny = d*Jacxz/fabs(ny);
                    com[0] += coeffny*(3.0*x1 + x21 + x31)/6.0;
                    com[1] += coeffny*(3.0*d - nx*(3.0*x1 + x21 + x31) - nz*(3.0*z1 + z21 + z31))/(6.0*ny);
                    com[2] += coeffny*(3.0*z1 + z21 + z31)/6.0;
                }
                else //fabs(nx) > machine_zero
                {
                    double Jacyz = fabs(y21*z31 - y31*z21);
                    double coeffnx = d*Jacyz/fabs(nx);
                    com[0] += coeffnx*(3.0*d - ny*(3.0*y1 + y21 + y31) - nz*(3.0*z1 + z21 + z31))/(6.0*nx);
                    com[1] += coeffnx*(3.0*y1 + y21 + y31)/6.0;
                    com[2] += coeffnx*(3.0*z1 + z21 + z31)/6.0;
                }
            }
        }

        return com/(4.0*get_vol());
    }

    //Calculate the inertia matrix of an arbitrary homogeneous closed surface polyhedron.
    dmat3 get_inertia(const double M)
    {
        gen_norms();

        double Jxx = 0.0, Jyy = 0.0, Jzz = 0.0, Jxy = 0.0, Jxz = 0.0, Jyz = 0.0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            double x1 = verts[faces[i][0]][0];
            double y1 = verts[faces[i][0]][1];
            double z1 = verts[faces[i][0]][2];
            
            double x2 = verts[faces[i][1]][0];
            double y2 = verts[faces[i][1]][1];
            double z2 = verts[faces[i][1]][2];

            double x3 = verts[faces[i][2]][0];
            double y3 = verts[faces[i][2]][1];
            double z3 = verts[faces[i][2]][2];

            double x21 = x2 - x1;
            double x31 = x3 - x1;
            double y21 = y2 - y1;
            double y31 = y3 - y1;
            double z21 = z2 - z1;
            double z31 = z3 - z1;

            double nx = norms[i][0];
            double ny = norms[i][1];
            double nz = norms[i][2];
            double d = x1*nx + y1*ny + z1*nz; //x*nx + y*ny + z*nz - d = 0 (plane equation)
            
            if (fabs(nz) > machine_zero)
            {
                double Jacxy = fabs(x21*y31 - x31*y21);
                double coeffnz = d*Jacxy/fabs(nz);
                Jxx +=  coeffnz*(6.0*x1*x1 + x21*x21 + x21*x31 + x31*x31 + 4.0*x1*(x21 + x31))/12.0;
                Jyy +=  coeffnz*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                Jzz +=  coeffnz*(6*pow(d,2) + pow(nx,2)*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) - 4*d*(nx*(3*x1 + x21 + x31) + ny*(3*y1 + y21 + y31)) + nx*ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(12.*pow(nz,2));
                Jxy +=  coeffnz*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))/24.;
                Jxz += -coeffnz*(-4*d*(3*x1 + x21 + x31) + 2*nx*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(24.*nz);
                Jyz += -coeffnz*(-4*d*(3*y1 + y21 + y31) + 2*ny*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + nx*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(24.*nz);
            }
            else
            {
                if (fabs(ny) > machine_zero)
                {
                    double Jacxz = fabs(x21*z31 - x31*z21);
                    double coeffny = d*Jacxz/fabs(ny);
                    Jxx +=  coeffny*(6.0*x1*x1 + x21*x21 + x21*x31 + x31*x31 + 4.0*x1*(x21 + x31))/12.0;
                    Jyy +=  coeffny*(6*pow(d,2) + pow(nx,2)*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(nx*(3*x1 + x21 + x31) + nz*(3*z1 + z21 + z31)) + nx*nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(12.*pow(ny,2));
                    Jzz +=  coeffny*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                    Jxy += -coeffny*(-4*d*(3*x1 + x21 + x31) + 2*nx*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(24.*ny);
                    Jxz +=  coeffny*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))/24.;
                    Jyz += -coeffny*(-4*d*(3*z1 + z21 + z31) + 2*nz*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) + nx*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(24.*ny);
                }
                else //fabs(nx) > machine_zero
                {
                    double Jacyz = fabs(y21*z31 - y31*z21);
                    double coeffnx = d*Jacyz/fabs(nx);
                    Jxx +=  coeffnx*(6*pow(d,2) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31)) + ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(12.*pow(nx,2));
                    Jyy +=  coeffnx*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                    Jzz +=  coeffnx*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                    Jxy += -coeffnx*(-4*d*(3*y1 + y21 + y31) + 2*ny*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(24.*nx);
                    Jxz += -coeffnx*(-4*d*(3*z1 + z21 + z31) + 2*nz*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) + ny*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(24.*nx);
                    Jyz +=  coeffnx*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))/24.;
                }
            }
        }

        double ord2_coeff = M/(5.0*get_vol());
        Jxx *= ord2_coeff;
        Jyy *= ord2_coeff;
        Jzz *= ord2_coeff;
        Jxy *= ord2_coeff;
        Jxz *= ord2_coeff;
        Jyz *= ord2_coeff;

        return {{{Jyy + Jzz,    Jxy,       Jxz   },
                 {   Jxy,    Jxx + Jzz,    Jyz   },
                 {   Jxz,       Jyz,    Jxx + Jyy}}};
    }

    //Shift the center of mass of the polyhedron, so that it coincides with O(0,0,0).
    void eliminate_com_offset(const dvec3 &com)
    {
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = verts[i] - com;
    }

    //Rotate the vertices, so that the inertia matrix becomes diagonal.
    /*
    void align_principal_axes_to_basis(const double M)
    {
        dmat3 I = get_inertia(M);
        //if (inertia_is_diagonal(I, 1.0e-12));
        //    return;

        dmat3 eigvecs = inertia_eigvecs(I); //Three real and normalized vectors that form a right handed Cartesian basis.

        //Define the default Cartesian coordinate system.
        dvec3 xaxis = {1.0,0.0,0.0};
        dvec3 yaxis = {0.0,1.0,0.0};
        dvec3 zaxis = {0.0,0.0,1.0};

        dmat3 rot; //Direction cosine matrix.
        rot[0][0] = dot(xaxis, eigvecs[0]);
        rot[0][1] = dot(xaxis, eigvecs[1]);
        rot[0][2] = dot(xaxis, eigvecs[2]);
        rot[1][0] = dot(yaxis, eigvecs[0]);
        rot[1][1] = dot(yaxis, eigvecs[1]);
        rot[1][2] = dot(yaxis, eigvecs[2]);
        rot[2][0] = dot(zaxis, eigvecs[0]);
        rot[2][1] = dot(zaxis, eigvecs[1]);
        rot[2][2] = dot(zaxis, eigvecs[2]);
        rot = transpose(rot);

        //Multiply each vertex vector with the rotation matrix.
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = dot(rot, verts[i]);
    }
    */

    //Farthest vertex distance with respect to the local coordinate system.
    double get_farthest_vertex_distance()
    {
        double farthest = length(verts[0]); //Assume that the farthest vertex distance is the first one.
        for (size_t i = 1; i < verts.size(); ++i)
        {
            double dist = length(verts[i]);
            if (dist > farthest)
                farthest = dist;
        }
        return farthest;
    }

    //Nearest vertex distance with respect to the local coordinate system.
    double get_nearest_vertex_distance()
    {
        double nearest = length(verts[0]); //Assume that the nearest vertex distance is the first one.
        for (size_t i = 1; i < verts.size(); ++i)
        {
            double dist = length(verts[i]);
            if (dist < nearest)
                nearest = dist;
        }
        return nearest;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Polyhedron's edge indices (no duplicates) along with their current face index and their neighboring face index.
//Result is : [edge 1, edge 2, face in which it belongs, neighbour face].
/*
umatnx4 poly_edges(const umatnx3 &faces)
{
    umatnx4 edges;
    for (size_t i = 0; i < faces.size(); ++i)
    {
        unsigned int p0 = faces[i][0];
        unsigned int p1 = faces[i][1];
        unsigned int p2 = faces[i][2];
        edges.push_back({p0,p1, (unsigned int)i, std::numeric_limits<unsigned int>::quiet_NaN()});
        edges.push_back({p1,p2, (unsigned int)i, std::numeric_limits<unsigned int>::quiet_NaN()});
        edges.push_back({p2,p0, (unsigned int)i, std::numeric_limits<unsigned int>::quiet_NaN()});
    }

    for (size_t i = 0; i < edges.size() - 1; ++i)
    {
        for (size_t j = i + 1; j < edges.size(); ++j)
        {
            if ( (edges[i][0] == edges[j][0] || edges[i][0] == edges[j][1]) &&
                 (edges[i][1] == edges[j][0] || edges[i][1] == edges[j][1]) )
            {
                edges[i][3] = edges[j][2];
                edges.erase(edges.begin() + j, edges.begin() + j + 1); //Erase the j-th row of edges[][] matrix.
                break;
            }
        }
    }

    return edges;
}
*/

//Werner and Scheeres paper.
/*
void poly_EF(const dmatnx3 &verts, const umatnx4 &edges, const dmatnx3 &norms, dtensnx3x3 &E, dtensnx3x3 &F)
{
    //Precompute all stuff related to the edges sum.
    for (size_t i = 0; i < edges.size(); ++i)
    {
        dvec3 nf1 = norms[edges[i][2]];
        dvec3 nf2 = norms[edges[i][3]];
        dvec3 n12f1 = cross(verts[edges[i][1]] - verts[edges[i][0]], nf1);
        dvec3 n21f2 = cross(verts[edges[i][0]] - verts[edges[i][1]], nf2);
        n12f1 = n12f1/length(n12f1);
        n21f2 = n21f2/length(n21f2);
        E.push_back( outer(nf1,n12f1) + outer(nf2,n21f2) );
    }

    //Precompute all stuff related to the faces sum.
    for (size_t i = 0; i < norms.size(); ++i)
        F.push_back( outer(norms[i], norms[i]) );
}
*/

//Decide whether a point r(x,y,z) is inside an arbitrary polyhedron(verts, faces) or not by raycasting.
/*
bool in_poly(const dvec3 &r, const dmatnx3 &verts, const imatnx3 &faces)
{
    int intersections = 0;
    dmatnx3 norms = poly_norms(verts, faces);

    //loop through all the triangulated faces in search for intersection
    for (int j = 0; j < faces.size(); ++j)
    {
        //define the triangle j from 3 points p0,p1,p2
        dvec3 p0 = verts[faces[j][0]];
        dvec3 p1 = verts[faces[j][1]];
        dvec3 p2 = verts[faces[j][2]];
        dvec3 pj = { p0[0] + ( (p0[1] - r[1])*norms[j][1] + (p0[2] - r[2])*norms[j][2] )/norms[j][0],
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
    
    //odd : (x,y,z) is inside the polyhdernon
    //even : (x,y,z) is outside the polyhdernon
    if (intersections%2 == 1)
        return true;

    return false;
}
*/

//Fill a polyhedron(verts, faces) with mascons assuming 'grid_reso' Cartesian grid resolution.
//Note that (in general) the final mascon model will not have zero center of mass and diagonal inertia matrix.
/*
dmatnx3 fill_poly_with_masc(const dmatnx3 &verts, const imatnx3 &faces, const ivec3 &grid_reso)
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

    double xmin = rmin[0] + 100.0*machine_zero;
    double ymin = rmin[1] + 200.0*machine_zero;
    double zmin = rmin[2] + 300.0*machine_zero;

    double xmax = rmax[0] - 400.0*machine_zero;
    double ymax = rmax[1] - 500.0*machine_zero;
    double zmax = rmax[2] - 600.0*machine_zero;

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
                if (in_poly(r, verts,faces))
                {
                    masc.push_back(r);
                }
            }
        }
    }

    return masc;
}
*/

/*
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
*/

/*
//Center of mass of a mascon distribution with constant density.
dvec3 masc_com(const dmatnx3 &masc)
{
    dvec3 com = {0.0,0.0,0.0};
    for (int i = 0; i < masc.size(); ++i)
    {
        com = com + masc[i];
    }

    return com/masc.size();
}

//Farthest mascon distance with respect to the local coordinate system.
double masc_farthest(const dmatnx3 &masc)
{
    double farthest = length(masc[0]);
    for (int i = 1; i < masc.size(); ++i)
    {
        double dist = length(masc[i]);
        if (dist > farthest)
            farthest = dist;
    }
    return farthest;
}

//Nearest mascon distance with respect to the local coordinate system.
double masc_nearest(const dmatnx3 &masc)
{
    double nearest = length(masc[0]);
    for (int i = 1; i < masc.size(); ++i)
    {
        double dist = length(masc[i]);
        if (dist < nearest)
            nearest = dist;
    }
    return nearest;
}

//Shift the center of mass of a mascon distribution with constant density, so that it coincides with O(0,0,0).
void correct_masc_com(dmatnx3 &masc)
{
    dvec3 com = masc_com(masc);
    for (int i = 0; i < masc.size(); ++i)
    {
        masc[i] = masc[i] - com;
    }

    return;
}

//Moment of inertia of a mascon distribution with constant density.
dmat3 masc_inertia(const double M, const dmatnx3 &masc)
{
    double m = (double)M/masc.size(); //mass of each mascon

    double Ixx,Iyy,Izz,Ixy,Ixz,Iyz;
    Ixx = Iyy = Izz = Ixy = Ixz = Iyz = 0.0;

    for (int i = 0; i < masc.size(); ++i)
    {
        Ixx += ( pow(masc[i][1], 2) + pow(masc[i][2], 2) ); //y[i]^2 + z[i]^2
        Iyy += ( pow(masc[i][0], 2) + pow(masc[i][2], 2) ); //x[i]^2 + z[i]^2
        Izz += ( pow(masc[i][0], 2) + pow(masc[i][1], 2) ); //x[i]^2 + y[i]^2
        Ixy -= masc[i][0]*masc[i][1]; //-x[i]*y[i]
        Ixz -= masc[i][0]*masc[i][2]; //-x[i]*z[i]
        Iyz -= masc[i][1]*masc[i][2]; //-y[i]*z[i]
    }

    //Ixy = Iyx, Ixz = Izx, Iyz = Izy by definition
    return {{{m*Ixx, m*Ixy, m*Ixz},
             {m*Ixy, m*Iyy, m*Iyz},
             {m*Ixz, m*Iyz, m*Izz}}};
}

//Align the principal axes of a mascon distribution with constant density, so that they coincide with the basis {{1,0,0}, {0,1,0}, {0,0,1}}.
void correct_masc_inertia(const double M, dmatnx3 &masc)
{
    dmat3 I = masc_inertia(M, masc);
    dmat3 eigvecs = inertia_eigvecs(I); //3 3x1 real and normalized vectors that form a right handed Cartesian basis.

    //instantiate the (unit) coordinate system
    dvec3 xaxis = {1.0,0.0,0.0};
    dvec3 yaxis = {0.0,1.0,0.0};
    dvec3 zaxis = {0.0,0.0,1.0};

    dmat3 rot; //direction cosine matrix
    rot[0][0] = dot(xaxis,eigvecs[0]);
    rot[0][1] = dot(xaxis,eigvecs[1]);
    rot[0][2] = dot(xaxis,eigvecs[2]);
    rot[1][0] = dot(yaxis,eigvecs[0]);
    rot[1][1] = dot(yaxis,eigvecs[1]);
    rot[1][2] = dot(yaxis,eigvecs[2]);
    rot[2][0] = dot(zaxis,eigvecs[0]);
    rot[2][1] = dot(zaxis,eigvecs[1]);
    rot[2][2] = dot(zaxis,eigvecs[2]);
    rot = transpose(rot);

    //multiply the mascon vectors with the rotation matrix
    for (int i = 0; i < masc.size(); ++i)
    {
        masc[i] = dot(rot, masc[i]);
    }

    return;
}

//Non normalized inertial integral tensor of arbitrary order of a mascon distribution with constant density.
dtens masc_integrals(const double M, const dmatnx3 &masc, const int ord)
{
    double m = (double)M/masc.size(); //mass of each mascon
    dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //initialize the tensor with zeros
    for (int i = 0; i < ord + 1; ++i)
        for (int j = 0; j < ord + 1; ++j)
            for (int k = 0; k < ord + 1; ++k)
                for (int n = 0; n < masc.size(); ++n)
                    J[i][j][k] += m*pow(masc[n][0], i)*pow(masc[n][1], j)*pow(masc[n][2], k); //m*(x[n]^i)*(y[n]^j)*(z[n]^k)
    return J;
}

//Generate the Stokes coefficient matrices C[n][m], S[n][m] of a mascon distribution with constant density.
//If supernormalized = true, then stage 2 normalization (i.e. supernormalization) will be applied to C[n][m] and S[n][m].
//Else only stage 1 normalization will be applied to C[n][m] and S[n][m].
void masc_stokes(dmat &C, dmat &S, const double M, const dmatnx3 &masc, const double R0, const int ord, bool supernormalized = true)
{
    double mass = (double)M/masc.size(); //mass of each mascon
    C.clear();
    S.clear();
    for (int n = 0; n < ord + 1; ++n)
    {
        C.push_back(dvec{});
        S.push_back(dvec{});
        for (int m = 0; m < n + 1; ++m)
        {
            C[n].push_back(0.0);
            S[n].push_back(0.0);

            double normcoeff;
            if (supernormalized)
                normcoeff = sqrt( factorial(n+m)/((2.0 - kronecker(0,m))*(2.0*n + 1.0)*factorial(n-m)) );
            else
                normcoeff = 1.0;
            double auxcoeff = (2.0 - kronecker(0,m))*factorial(n-m)/factorial(n+m)/M;
            
            for (int i = 0; i < masc.size(); ++i)
            {
                dvec3 spher = cart2spher(masc[i]);
                double r = spher[0], lon = spher[1], lat = spher[2];
                C[n][m] += normcoeff*auxcoeff*mass*pow(r/R0, n)*std::assoc_legendre(n,m, cos(lat))*cos(m*lon);
                S[n][m] += normcoeff*auxcoeff*mass*pow(r/R0, n)*std::assoc_legendre(n,m, cos(lat))*sin(m*lon);
            }
        }
    }

    return;
}
*/

#endif
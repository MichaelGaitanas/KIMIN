#ifndef POLYHEDRON_HPP
#define POLYHEDRON_HPP

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Calculate the normal vectors of a polyhedron defined by verts and faces.
dmatnx3 poly_norms(const dmatnx3 &verts, const imatnx3 &faces)
{
    dmatnx3 norms(faces.size()); //same as : dmatnx3 norms(faces.size(), dvec3{0.0,0.0,0.0});
    dvec3 perp;
    for (int i = 0; i < norms.size(); ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        perp = cross(p1-p0, p2-p1);
        norms[i] = perp/length(perp);
    }
    return norms;
}

//Decide whether a point r(x,y,z) is inside an arbitrary polyhedron(verts, faces) or not by raycasting.
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

//Fill a polyhedron(verts, faces) with mascons assuming 'grid_reso' Cartesian grid resolution.
//Note that (in general) the final mascon model will not have zero center of mass and diagonal inertia matrix.
//Use correct_masc_com() and correct_masc_inertia() for the aforementioned.
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

    double xmin = rmin[0] + machine_zero;
    double ymin = rmin[1] + machine_zero;
    double zmin = rmin[2] + machine_zero;

    double xmax = rmax[0] - machine_zero;
    double ymax = rmax[1] - machine_zero;
    double zmax = rmax[2] - machine_zero;

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

#endif
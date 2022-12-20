#ifndef POLYHEDRON_HPP
#define POLYHEDRON_HPP

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"

//Calculate the perpendicular vectors of a polyhedron defined by verts and faces.
dmatnx3 poly_perps(const dmatnx3 &verts, const imatnx3 &faces)
{
    dmatnx3 perps(faces.size()); //same as : dmatnx3 perps(faces.size(), dvec3{0.0,0.0,0.0});
    for (int i = 0; i < perps.size(); ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        perps[i] = cross(p1-p0, p2-p1);
    }
    return perps;
}

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

//Polyhedron's edge indices (no duplicates) along with their current face index and their neighboring face index.
//[edge 1, edge 2, face in which it belongs, neighbour face]
imatnx4 poly_edges(const imatnx3 &faces)
{
    imatnx4 edges;
    for (int i = 0; i < faces.size(); ++i)
    {
        int p0 = faces[i][0];
        int p1 = faces[i][1];
        int p2 = faces[i][2];
        edges.push_back({p0,p1, i, std::numeric_limits<int>::quiet_NaN()});
        edges.push_back({p1,p2, i, std::numeric_limits<int>::quiet_NaN()});
        edges.push_back({p2,p0, i, std::numeric_limits<int>::quiet_NaN()});
    }

    for (int i = 0; i < edges.size() - 1; ++i)
    {
        for (int j = i + 1; j < edges.size(); ++j)
        {
            if ( (edges[i][0] == edges[j][0] || edges[i][0] == edges[j][1]) &&
                 (edges[i][1] == edges[j][0] || edges[i][1] == edges[j][1]) )
            {
                edges[i][3] = edges[j][2];
                edges.erase(edges.begin() + j, edges.begin() + j + 1); //erase the j-th row of edges[][] matrix
                break;
            }
        }
    }

    return edges;
}

void poly_EF(const dmatnx3 &verts, const imatnx4 &edges, const dmatnx3 &norms, dtensnx3x3 &E, dtensnx3x3 &F)
{
    //precompute all stuff related to the edges sum
    for (int i = 0; i < edges.size(); ++i)
    {
        dvec3 nf1 = norms[edges[i][2]];
        dvec3 nf2 = norms[edges[i][3]];
        dvec3 n12f1 = cross(verts[edges[i][1]] - verts[edges[i][0]], nf1);
        dvec3 n21f2 = cross(verts[edges[i][0]] - verts[edges[i][1]], nf2);
        n12f1 = n12f1/length(n12f1);
        n21f2 = n21f2/length(n21f2);
        E.push_back( outer(nf1,n12f1) + outer(nf2,n21f2) );
    }

    //precompute all stuff related to the faces sum
    for (int i = 0; i < norms.size(); ++i)
    {
        F.push_back( outer(norms[i], norms[i]) );
    }

    return;
}

//This function computes the total volume of a polyhedron assuming that the latter is a bunch of
//attached tetrahedra with common vertex at the O(0,0,0). The total volume then, is the sum of
//the individual tetrahedra volumes. This function does NOT work for all polyhedra. Use poly_vol() instead.
double poly_vol_tetr(const dmatnx3 &verts, const imatnx3 &faces)
{
    double vol = 0.0;
    for (int i = 0; i < faces.size(); ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        vol += fabs(dot(p0, cross(p1,p2)))/6.0;
    }
    return vol;
}

//Calculate the volume of an arbitrary polyhedron.
double poly_vol(const dmatnx3 &verts, const imatnx3 &faces)
{
    dmatnx3 norms = poly_norms(verts, faces);

    double vol = 0.0;
    for (int i = 0; i < faces.size(); ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        double d = p0[0]*norms[i][0] + p0[1]*norms[i][1] + p0[2]*norms[i][2]; //x*nx + y*ny + z*nz - d = 0 (plane equation)
        double A = 0.5*length(cross(p1-p0, p2-p1));
        vol += d*A/3.0;
    }
    return vol;
}

//This function computes the center of mass of a polyhedron assuming that the latter is a bunch of homogeneous
//attached tetrahedra with common vertex at the O(0,0,0). The latter is assumed to be the center of mass
//of the individual tetrahedra centers of mass. This function does NOT work for all polyhedra. Use poly_com() instead.
dvec3 poly_com_tetr(const dmatnx3 &verts, const imatnx3 &faces)
{
    dvec3 poly_com = {0.0,0.0,0.0}; //final center of mass of the polyhedron

    dmatnx3 tetr_coms(faces.size()); //individual centers of mass of the tetrahedra
    for (int i = 0; i < tetr_coms.size(); ++i)
    {
        dvec3 p0 = verts[faces[i][0]];
        dvec3 p1 = verts[faces[i][1]];
        dvec3 p2 = verts[faces[i][2]];
        tetr_coms[i] = (p0 + p1 + p2)/4.0; //center of mass of the i-tetrahedron

        poly_com = poly_com + tetr_coms[i];
    }
    return poly_com/tetr_coms.size();
}

//Calculate the center of mass of an arbitrary homogeneous polyhedron.
dvec3 poly_com(const dmatnx3 &verts, const imatnx3 &faces)
{
    dmatnx3 norms = poly_norms(verts,faces);

    dvec3 com = {0.0,0.0,0.0};
    for (int i = 0; i < faces.size(); ++i)
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
        double n = sqrt(nx*nx + ny*ny + nz*nz);

        double d = x1*nx + y1*ny + z1*nz; //x*nx + y*ny + z*nz - d = 0 (plane equation)

        //double integral values after the affine transformation
        double integx = (3.0*x1 + x21 + x31)/6.0;
        double integy = (3.0*y1 + y21 + y31)/6.0;
        double integz = (3.0*z1 + z21 + z31)/6.0;

        //Jacobian determinant that arises due to the affine transformation
        double Jacxy = fabs(x21*y31 - x31*y21);
        double Jacxz = fabs(x21*z31 - x31*z21);
        double Jacyz = fabs(y21*z31 - y31*z21);

        double area = 0.5*length(cross(dvec3{x2,y2,z2} - dvec3{x1,y1,z1}, dvec3{x3,y3,z3} - dvec3{x2,y2,z2}));
        
        //comx
        if (fabs(nz) > machine_zero)
            com[0] += d*Jacxy*integx*n/fabs(nz);
        else
            if (fabs(ny) > machine_zero)
                com[0] += d*Jacxz*integx*n/fabs(ny);
            else
                com[0] += d*x1*area*n/fabs(nx); //com[0] += d*d*area*n/(nx*fabs(nx));

        //comy
        if (fabs(nz) > machine_zero)
            com[1] += d*Jacxy*integy*n/fabs(nz);
        else
            if (fabs(nx) > machine_zero)
                com[1] += d*Jacyz*integy*n/fabs(nx);
            else
                com[1] += d*y1*area*n/fabs(ny); //com[1] += d*d*area*n/(ny*fabs(ny));

        //comz
        if (fabs(ny) > machine_zero)
            com[2] += d*Jacxz*integz*n/fabs(ny);
        else
            if (fabs(nx) > machine_zero)
                com[2] += d*Jacyz*integz*n/fabs(nx);
            else
                com[2] += d*z1*area*n/fabs(nz); //com[2] += d*d*area*n/(nz*fabs(nz));
    }

    return com/(4.0*poly_vol(verts,faces));
}

//Shift the center of mass of a polyhedron with constant density, so that it coincides with O(0,0,0).
void correct_poly_com(dmatnx3 &verts, const imatnx3 &faces)
{
    dvec3 com = poly_com(verts,faces);
    for (int i = 0; i < verts.size(); ++i)
    {
        verts[i] = verts[i] - com;
    }

    return;
}

/* Write the corresponding functions for the moment of inertia of an arbitrary polyhedron */

//Decide whether a point r(x,y,z) is inside an arbitrary polyhedron(verts, faces) or not by raycasting.
bool in_poly(const dvec3 &r, const dmatnx3 &verts, const imatnx3 &faces)
{
    int intersections = 0;
    dmatnx3 norms = poly_perps(verts, faces);
    //dmatnx3 norms = poly_norms(verts, faces);

    //loop through all the triangulated faces in search for intersection
    for (int j = 0; j < faces.size(); ++j)
    {
        //define the triangle j from 3 points p0,p1,p2
        dvec3 p0 = verts[faces[j][0]];
        dvec3 p1 = verts[faces[j][1]];
        dvec3 p2 = verts[faces[j][2]];
        dvec3 pj = { p0[0] + (p0[1]*norms[j][1] + p0[2]*norms[j][2] - r[1]*norms[j][1] - r[2]*norms[j][2])/norms[j][0], r[1], r[2] };

        //form the following 3 triangles and calculate their area
        double Aj01 = 0.5*length(cross(p0-pj, p1-p0)); //pj -> p0 -> p1
        double Aj12 = 0.5*length(cross(p1-pj, p2-p1)); //pj -> p1 -> p2
        double Aj20 = 0.5*length(cross(p2-pj, p0-p2)); //pj -> p2 -> p0
        double A012 = 0.5*length(cross(p1-p0, p2-p1)); //p0 -> p1 -> p2

        //if the sum of the 3 areas is equal to the area of the surface triangle, then pj sits on the surface of the triangle.
        if ( fabs(Aj01 + Aj12 + Aj20 - A012) <= machine_zero && pj[0] > r[0])
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

    double xmin = rmin[0];
    double ymin = rmin[1];
    double zmin = rmin[2];

    double xmax = rmax[0];
    double ymax = rmax[1];
    double zmax = rmax[2];

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
#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include<Eigen/Dense>

#include"typedef.h"
#include"constant.h"
#include"linalg.h"

class polyhedron
{
private:
    bool norms_exist;
    bool vol_exists;
    dmatnx3 verts;
    umatnx3 faces;
    dmatnx3 norms;
    double vol;

public:
    //Load the .obj file assuming it contains vertcies and faces ('v x y z' and 'f i j k').
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
            fprintf(stderr, "Error : '%s' could not be opened. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        double x,y,z; //Vertices.
        unsigned int vi1,vi2,vi3; //Faces.

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

    //Export to an .obj file the current vertices and faces of the polyhedron mesh ('v x y z' and 'f i j k).
    void export_obj_file_vf(const char *path)
    {
        FILE *fp = fopen(path,"w");
        for (size_t i = 0; i < verts.size(); ++i)
            fprintf(fp, "v %.15lf %.15lf %.15lf\n", verts[i][0],verts[i][1],verts[i][2]);
        for (size_t i = 0; i < faces.size(); ++i)
            fprintf(fp, "f %u %u %u\n", faces[i][0]+1,faces[i][1]+1,faces[i][2]+1);
        fclose(fp);
    }

    //Generate the polyhderon's normals.
    void gen_norms()
    {
        if (norms_exist)
            return;

        if (faces.empty())
        {
            printf("Warning : faces.empty() = true. No normals are generated.\n");
            return;
        }
        
        norms.clear();
        norms.resize(faces.size());
        dvec3 perp;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            perp = cross(p1-p0, p2-p1);
            double len = length(perp);
            if (len > machine_zero)
                norms[i] = perp/len;
            else
                norms[i] = dvec3{0.0,0.0,0.0}; //Degenerate case...
        }
        norms_exist = true;
    }

    //Calculate the polyhderon's total volume.
    double get_vol()
    {
        if (vol_exists)
            return vol;

        gen_norms();

        vol = 0.0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            double d = p0[0]*norms[i][0] + p0[1]*norms[i][1] + p0[2]*norms[i][2]; //x*nx + y*ny + z*nz - d = 0 (plane equation).
            double A = 0.5*length(cross(p1-p0, p2-p1));
            vol += d*A/3.0;
        }
        vol_exists = true;
        return vol;
    }

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

    //Get (xmax, ymax, zmax) of the polyhedron with respect to the local coordinate system.
    dvec3 get_farthest_coordinates_per_axis()
    {
        dvec3 rmax = verts[0];
        for (size_t i = 1; i < verts.size(); ++i)
        {
            if (verts[i][0] > rmax[0]) rmax[0] = verts[i][0];
            if (verts[i][1] > rmax[1]) rmax[1] = verts[i][1];
            if (verts[i][2] > rmax[2]) rmax[2] = verts[i][2];
        }
        return rmax;
    }

    //Get (xmin, ymin, zmin) of the polyhedron with respect to the local coordinate system.
    dvec3 get_nearest_coordinates_per_axis()
    {
        dvec3 rmin = verts[0];
        for (size_t i = 1; i < verts.size(); ++i)
        {
            if (verts[i][0] < rmin[0]) rmin[0] = verts[i][0];
            if (verts[i][1] < rmin[1]) rmin[1] = verts[i][1];
            if (verts[i][2] < rmin[2]) rmin[2] = verts[i][2];
        }
        return rmin;
    }

    //This function decides whether or not a given point in space (r) is inside the polyhderon's surface via raycasting.
    //A ray is casted from the point of examination (r) up to a destination point (pdest), which must be outside the
    //polyhedron's surface. Then we count the number of intersections between the ray and the polyhedron. If the number of intersections
    //is odd, then r is inside the polyhedron. Otherwise it is outside.
    bool encloses_point(const dvec3 &r, const double tolerance = 1.0e-12)
    {
        gen_norms();

        //Ray's destination point. It is assumed to be very far away, aiming to be outside of the polyhedron.
        dvec3 pdest = 10000000.0*dvec3{pi, std::exp(1.0), std::sqrt(2.0)};
        
        size_t intersections = 0;

        //The polyhderon is basically a collection of triangles. To find intersections between the ray and the polyhderon,
        //we essentially loop through all the faces and check.
        for (size_t j = 0; j < faces.size(); ++j)
        {
            //Define the triangle j from the 3 vertices p0,p1,p2.
            dvec3 p0 = verts[faces[j][0]];
            dvec3 p1 = verts[faces[j][1]];
            dvec3 p2 = verts[faces[j][2]];
            //By solving the equation of a line and a plane, we find intersection point pj.
            double lam = ( (p0[0] - r[0])*norms[j][0] + (p0[1] - r[1])*norms[j][1] + (p0[2] - r[2])*norms[j][2] )/( (pdest[0] - r[0])*norms[j][0] + (pdest[1] - r[1])*norms[j][1] + (pdest[2] - r[2])*norms[j][2] );
            dvec3 pj = dvec3{ r[0] + lam*(pdest[0] - r[0]),
                              r[1] + lam*(pdest[1] - r[1]),
                              r[2] + lam*(pdest[2] - r[2]) };

            //We must check however if the ray intersects the triangle j and not the whole extended mathematical plane.

            //Form the following 3 triangles and calculate their area.
            double Aj01 = 0.5*length(cross(p0-pj, p1-p0)); //pj -> p0 -> p1
            double Aj12 = 0.5*length(cross(p1-pj, p2-p1)); //pj -> p1 -> p2
            double Aj20 = 0.5*length(cross(p2-pj, p0-p2)); //pj -> p2 -> p0
            double A012 = 0.5*length(cross(p1-p0, p2-p1)); //p0 -> p1 -> p2

            //If the sum of the 3 areas is equal to the area of the surface triangle, then pj sits upon the surface of the triangle.
            if ( fabs(Aj01 + Aj12 + Aj20 - A012) <= tolerance && pj[0] > r[0] && pj[1] > r[1] && pj[2] > r[2] )
                ++intersections;
        }
        
        if (intersections%2 == 1)
            return true; //Odd : (x,y,z) is inside the polyhdernon.

        return false; //Even : (x,y,z) is outside the polyhdernon.
    }

    //Calculate the center of mass of the surface vertices of the polyhedron, assuming homogeneous mass density.
    dvec3 get_com_vertices()
    {
        dvec3 com = dvec3{0.0,0.0,0.0};
        for (size_t i = 0; i < verts.size(); ++i)
            com = com + verts[i];
        return com/verts.size();
    }

    //Calculate the center of mass of the polyhedron, assuming homogeneous mass density (volume -> surface integrals via Gauss theorem).
    dvec3 get_com()
    {
        gen_norms();

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

    //Calculate the inertia matrix of the polyhedron, assuming homogeneous mass density (volume -> surface integrals via Gauss theorem).
    dmat3 get_inertia(const double M, bool inertial_integrals = false)
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
                Jxy -=  coeffnz*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))/24.;
                Jxz -= -coeffnz*(-4*d*(3*x1 + x21 + x31) + 2*nx*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(24.*nz);
                Jyz -= -coeffnz*(-4*d*(3*y1 + y21 + y31) + 2*ny*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + nx*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(24.*nz);
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
                    Jxy -= -coeffny*(-4*d*(3*x1 + x21 + x31) + 2*nx*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(24.*ny);
                    Jxz -=  coeffny*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))/24.;
                    Jyz -= -coeffny*(-4*d*(3*z1 + z21 + z31) + 2*nz*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) + nx*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(24.*ny);
                }
                else //fabs(nx) > machine_zero
                {
                    double Jacyz = fabs(y21*z31 - y31*z21);
                    double coeffnx = d*Jacyz/fabs(nx);
                    Jxx +=  coeffnx*(6*pow(d,2) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31)) + ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(12.*pow(nx,2));
                    Jyy +=  coeffnx*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                    Jzz +=  coeffnx*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                    Jxy -= -coeffnx*(-4*d*(3*y1 + y21 + y31) + 2*ny*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(24.*nx);
                    Jxz -= -coeffnx*(-4*d*(3*z1 + z21 + z31) + 2*nz*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) + ny*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(24.*nx);
                    Jyz -=  coeffnx*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))/24.;
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

        if (inertial_integrals)
            return {{{Jxx,  0.0,  0.0},
                     {0.0,  Jyy,  0.0},
                     {0.0,  0.0,  Jzz}}};

        return {{{Jyy + Jzz,    Jxy,       Jxz   },
                 {   Jxy,    Jxx + Jzz,    Jyz   },
                 {   Jxz,       Jyz,    Jxx + Jyy}}};
    }

    dtens get_inertial_integrals_ord2(const double M)
    {
        const int ord = 2;
        dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //Initialize the tensor with zeros.
        dmat3 Jint = get_inertia(M, true);
        J[2][0][0] = Jint[0][0]; //Jxx
        J[0][2][0] = Jint[1][1]; //Jyy
        J[0][0][2] = Jint[2][2]; //Jzz
        return J;
    }

    //This function translates all the vertices, such that the resulted center of mass coincides with the local origin (zero). Homogeneous mass density is assumed.
    void set_com_zero()
    {
        dvec3 com = get_com();
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = verts[i] - com;
    }

    //This function rotates all the surface vertices, such that the resulted inertia matrix becomes diagonal. The rotation happens via left-multiplication of all
    //the vertices with a rotation matrix, which is basically the eigenvectors of the inertia matrix. Again homogeneous mass density is assumed.
    void set_inertia_diagonal(const double M)
    {
        dmat3 I = get_inertia(M);

        //Convert the dmat3 datatype to Eigen's.
        Eigen::Matrix3d eigen_I;
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                eigen_I(row, col) = I[row][col];

        //Solve the eigensystem (3x3, real and symmetric matrix).
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(eigen_I);
        if (solver.info() != Eigen::Success)
        {
            fprintf(stderr, "Error : Inertia eigenvalue decomposition failed. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        Eigen::Vector3d eigenvalues = solver.eigenvalues(); //By default, Eigen sorts them in ascending order : eigenvalues[0] <= eigenvalues[1] <= eigenvalues[2]
        Eigen::Matrix3d eigenvectors = solver.eigenvectors();

        //Extract the diagonal elements.
        double Ixx = I[0][0];
        double Iyy = I[1][1];
        double Izz = I[2][2];
        //Reorder the eigenvalues appropriately.
        uvec3 indices; //Order by which the eigenvalues will be sorted (and thus order of the eigenvectors in the final rotation matrix).
        if (Ixx < Iyy && Iyy < Izz) //Case : Ixx < Iyy < Izz
            indices = uvec3{0,1,2}; //default by Eigen.
        else if (Ixx < Izz && Izz < Iyy) //Case : Ixx < Izz < Iyy
            indices = uvec3{0,2,1};
        else if (Iyy < Ixx && Ixx < Izz) //Case : Iyy < Ixx < Izz
            indices = uvec3{1,0,2};
        else if (Iyy < Izz && Izz < Ixx) //Case : Iyy < Izz < Ixx
            indices = uvec3{2,0,1};
        else if (Izz < Iyy && Iyy < Ixx) //Case : Izz < Iyy < Ixx
            indices = uvec3{2,1,0};
        else //Case : Izz < Ixx < Iyy
            indices = uvec3{1,2,0};

        Eigen::Vector3d reordered_eigenvalues;
        Eigen::Matrix3d reordered_eigenvectors;
        //Reorder eigenvalues and eigenvectors according to indices[].
        for (size_t i = 0; i < 3; ++i)
        {
            reordered_eigenvalues(i) = eigenvalues(indices[i]);
            reordered_eigenvectors.col(i) = eigenvectors.col(indices[i]);
        }

        Eigen::Vector3d x_axis(1,0,0);
        Eigen::Vector3d y_axis(0,1,0);
        Eigen::Vector3d z_axis(0,0,1);

        Eigen::Vector3d eigvec_x_axis = reordered_eigenvectors.col(0);
        Eigen::Vector3d eigvec_y_axis = reordered_eigenvectors.col(1);
        Eigen::Vector3d eigvec_z_axis = reordered_eigenvectors.col(2);

        if (eigvec_x_axis.dot(x_axis) < 0) 
            eigvec_x_axis = -eigvec_x_axis;
        if (eigvec_y_axis.dot(y_axis) < 0) 
            eigvec_y_axis = -eigvec_y_axis;
        if (eigvec_z_axis.dot(z_axis) < 0)
            eigvec_z_axis = -eigvec_z_axis;

        //Rebuild into a 3x3 matrix.
        Eigen::Matrix3d eigen_rot_mat;
        eigen_rot_mat.col(0) = eigvec_x_axis;
        eigen_rot_mat.col(1) = eigvec_y_axis;
        eigen_rot_mat.col(2) = eigvec_z_axis;
        if (eigen_rot_mat.determinant() < 0)
            eigen_rot_mat.col(2) = -eigen_rot_mat.col(2); //Flip just one column (here the last, but any one would do). This renders the eigenvectors as right-handed coordinate system.

        eigen_rot_mat.transposeInPlace();

        //Now convert the Eigen variable eigen_rot_mat to dmat3 (fin_rot_mat).
        dmat3 fin_rot_mat;
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                fin_rot_mat[row][col] = eigen_rot_mat(row, col);

        //Apply the rotation to all the vertices of the rigid body.
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = dot(fin_rot_mat, verts[i]);

        //Since the vertices rotated, either we have to rotate the normals as well, or just recompute them...
        norms_exist = false;
        gen_norms();
    }
};

#endif
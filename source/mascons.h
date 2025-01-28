#ifndef MASCONS_H
#define MASCONS_H

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>
#include<Eigen/Dense>

#include"typedef.h"
#include"linalg.h"
#include"rigidbody.h"
#include"polyhedron.h"

class mascons
{
private:
    bool points_exist;
    dmatnx3 points;

public:
    //Load an .obj file, exclusively with the format 'v x y z' (point mascons).
    void load_obj_file(const char *path)
    {
        points_exist = false;
        points.clear();

        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            fprintf(stderr, "Error : '%s' could not be opened. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        double x,y,z; //Mascon (point) coordinates.

        str line;
        while (getline(objfile, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                sscanf(line.c_str(), "v %lf %lf %lf", &x, &y, &z);
                points.push_back({x,y,z});
            }
        }
        objfile.close();
        points_exist = true;
    }

    //Farthest point-mascon distance with respect to the local coordinate system.
    double get_farthest_point_distance()
    {
        double farthest = length(points[0]); //Assume that the farthest point distance corresponds to the first mascon.
        for (size_t i = 1; i < points.size(); ++i)
        {
            double dist = length(points[i]);
            if (dist > farthest)
                farthest = dist;
        }
        return farthest;
    }

    //Nearest point-mascon distance with respect to the local coordinate system.
    double get_nearest_point_distance()
    {
        double nearest = length(points[0]); //Assume that the farthest point distance corresponds to the first mascon.
        for (size_t i = 1; i < points.size(); ++i)
        {
            double dist = length(points[i]);
            if (dist < nearest)
                nearest = dist;
        }
        return nearest;
    }

    dvec3 get_com()
    {
        dvec3 com = dvec3{0.0,0.0,0.0};
        for (size_t i = 0; i < points.size(); ++i)
            com = com + points[i];
        return com/points.size();
    }

    dmat3 get_inertia(const double M)
    {
        double m = (double)M/points.size(); //Mass of each mascon.
        double Ixx = 0.0, Iyy = 0.0, Izz = 0.0, Ixy = 0.0, Ixz = 0.0, Iyz = 0.0;
        for (size_t i = 0; i < points.size(); ++i)
        {
            Ixx += ( pow(points[i][1], 2) + pow(points[i][2], 2) ); //y[i]^2 + z[i]^2
            Iyy += ( pow(points[i][0], 2) + pow(points[i][2], 2) ); //x[i]^2 + z[i]^2
            Izz += ( pow(points[i][0], 2) + pow(points[i][1], 2) ); //x[i]^2 + y[i]^2
            Ixy -= points[i][0]*points[i][1]; //-x[i]*y[i]
            Ixz -= points[i][0]*points[i][2]; //-x[i]*z[i]
            Iyz -= points[i][1]*points[i][2]; //-y[i]*z[i]
        }
        //Because the matrix is real and symmetric, Ixy = Iyx, Ixz = Izx, Iyz = Izy.
        return {{{m*Ixx, m*Ixy, m*Ixz},
                 {m*Ixy, m*Iyy, m*Iyz},
                 {m*Ixz, m*Iyz, m*Izz}}};
    }

    //Non normalized inertial integral tensor of arbitrary order of the mascons distribution (points), assuming constant density.
    dtens get_inertial_integrals(const double M, const int ord)
    {
        double m = (double)M/points.size(); //Mass of each mascon.
        dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //Initialize the tensor with zeros.
        for (int i = 0; i < ord + 1; ++i)
            for (int j = 0; j < ord + 1; ++j)
                for (int k = 0; k < ord + 1; ++k)
                    for (int n = 0; n < points.size(); ++n)
                        J[i][j][k] += m*pow(points[n][0], i)*pow(points[n][1], j)*pow(points[n][2], k); //J_ijk = m*(x[n]^i)*(y[n]^j)*(z[n]^k)
        return J;
    }

    void eliminate_com(const dvec3 &com)
    {
        for (size_t i = 0; i < points.size(); ++i)
            points[i] = points[i] - com;
    }

    void diagonalize_inertia(const dmat3 &I)
    {
        double Ixx = I[0][0];
        double Iyy = I[1][1];
        double Izz = I[2][2];

        Eigen::Matrix3d eigen_iner;
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                eigen_iner(row, col) = I[row][col];

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(eigen_iner);
        if (solver.info() != Eigen::Success)
        {
            fprintf(stderr, "Error : Inertia eigenvalue decomposition failed. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        Eigen::Vector3d eigenvalues = solver.eigenvalues(); // (lambda0 <= lambda1 <= lambda2)
        Eigen::Matrix3d eigenvectors = solver.eigenvectors();

        uvec3 indices;
        if (Ixx < Iyy && Iyy < Izz)
            indices = uvec3{0,1,2};
        else if (Ixx < Izz && Izz < Iyy)
            indices = uvec3{0,2,1};
        else if (Iyy < Ixx && Ixx < Izz)
            indices = uvec3{1,0,2};
        else if (Iyy < Izz && Izz < Ixx)
            indices = uvec3{1,2,0};
        else if (Izz < Iyy && Iyy < Ixx)
            indices = uvec3{2,1,0};
        else if (Izz < Ixx && Ixx < Iyy)
            indices = uvec3{2,0,1};
        else
        {
            printf("Equality! Exiting...\n");
            exit(EXIT_FAILURE);
        }

        Eigen::Vector3d sortedEigenvalues;
        Eigen::Matrix3d sortedEigenvectors;
        //Reorder eigenvalues and eigenvectors according to indices.
        for (int i = 0; i < 3; ++i)
        {
            sortedEigenvalues(i) = eigenvalues(indices[i]);
            sortedEigenvectors.col(i) = eigenvectors.col(indices[i]);
        }

        dmat3 eigvecs;
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                eigvecs[row][col] = sortedEigenvectors(row, col);

        dmat3 ev = transpose(eigvecs);

        //dmat3 eigvecs = transpose(inertia_eigvecs(I));
        for (size_t i = 0; i < points.size(); ++i)
            points[i] = dot(ev, points[i]);
    }

    //This function fills with point-mascons the interior of a given polyhedron surface (poly), in accordance with a given grid resolution (grid_reso).
    //The function assumes a (slightly scaled) circumscribed cuboid around the polyhedron, which traverses along x,y,z axes and at each point
    //checks whether or not the current (x,y,z) point is inside the polyhedron. if yes, the grid point is appended as point-mascon to the member variable 'points'.
    void generate_from_polyhedron(polyhedron &poly, const uvec3 &grid_reso)
    {
        //Get (xmax, ymax, zmax) and (xmin, ymin, zmin) of the polyhedron vertices to establish the cuboid.
        //Then, scale it, so that the polyhedron is 'conveniently' contained.
        dvec3 rmax = 1.1*poly.get_farthest_coordinates_per_axis();
        dvec3 rmin = 1.1*poly.get_nearest_coordinates_per_axis();

        double xmax = rmax[0];
        double ymax = rmax[1];
        double zmax = rmax[2];
        double xmin = rmin[0];
        double ymin = rmin[1];
        double zmin = rmin[2];

        //Now traverse the grid and decide whether a point is inside the polyhedron or not.
        for (unsigned int i = 0; i < grid_reso[0]; ++i)
        {
            double x = xmin + i*(xmax - xmin)/((double)grid_reso[0] - 1.0);
            for (unsigned int j = 0; j < grid_reso[1]; ++j)
            {
                double y = ymin + j*(ymax - ymin)/((double)grid_reso[1] - 1.0);
                for (unsigned int k = 0; k < grid_reso[2]; ++k)
                {
                    double z = zmin + k*(zmax - zmin)/((double)grid_reso[2] - 1.0);
                    dvec3 r = dvec3{x,y,z}; //Current point of the grid.
                    if (poly.encloses_point(r))
                        points.push_back(r);
                }
            }
        }
    }

    void export_points_to_obj(const char *path)
    {
        FILE *fp = fopen(path,"w");
        for (size_t i = 0; i < points.size(); ++i)
            fprintf(fp, "v %.15lf %.15lf %.15lf\n", points[i][0],points[i][1],points[i][2]);
        fclose(fp);
    }
};

#endif
#ifndef MASCONS_H
#define MASCONS_H

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>

#include<Eigen/Dense>

#include"typedef.h"
#include"linalg.h"
#include"polyhedron.h"

class mascons
{
private:
    dmatnx3 points;

public:
    //Load the vertices ('v x y z') from an .obj file (assuming them to be point mascons).
    void load_obj_file(const char *path)
    {
        points.clear();

        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            fprintf(stderr, "Error : '%s' could not be opened. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        double x,y,z; //Point mascon coordinates.

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
    }

    //Export to an .obj file the current state of 'points' member variable, in the form 'v x y z'.
    void export_obj_file(const char *path)
    {
        FILE *fp = fopen(path,"w");
        for (size_t i = 0; i < points.size(); ++i)
            fprintf(fp, "v %.15lf %.15lf %.15lf\n", points[i][0],points[i][1],points[i][2]);
        fclose(fp);
    }

    //Display to the terminal the coordinates of the mascons.
    void print_points()
    {
        for (size_t i = 0; i < points.size(); ++i)
            printf("[ %.15lf   %.15lf   %.15lf ]\n", points[i][0],points[i][1],points[i][2]);
    }

    //Get the total number of the current mascons.
    size_t get_total()
    {
        return points.size();
    }

    //Farthest point mascon distance with respect to the local coordinate system.
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

    //Nearest point mascon distance with respect to the local coordinate system.
    double get_nearest_point_distance()
    {
        double nearest = length(points[0]); //Assume that the nearest point distance corresponds to the first mascon.
        for (size_t i = 1; i < points.size(); ++i)
        {
            double dist = length(points[i]);
            if (dist < nearest)
                nearest = dist;
        }
        return nearest;
    }

    //Calculate the center of mass of the mascons distribution, assuming homogeneous mass density.
    dvec3 get_com()
    {
        dvec3 com = dvec3{0.0,0.0,0.0};
        for (size_t i = 0; i < points.size(); ++i)
            com = com + points[i];
        return com/points.size();
    }

    //Calculate the inertia matrix of the mascons distribution, assuming homogeneous mass density.
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

    //Inertial integral tensor of arbitrary order of the mascons distribution, assuming homogeneous mass density (non normalized).
    dtens get_inertial_integrals(const double M, const int ord)
    {
        double m = (double)M/points.size(); //Mass of each mascon.
        dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //Initialize the tensor with zeros.
        for (int i = 0; i < ord + 1; ++i)
            for (int j = 0; j < ord + 1; ++j)
                for (int k = 0; k < ord + 1; ++k)
                    for (size_t n = 0; n < points.size(); ++n)
                        J[i][j][k] += m*pow(points[n][0], i)*pow(points[n][1], j)*pow(points[n][2], k); //J_ijk = m*(x[n]^i)*(y[n]^j)*(z[n]^k)
        return J;
    }

    //This function translates all the mascon points, such that the resulted center of mass coincides with the local origin (zero). Homogeneous mass density is assumed.
    void set_com_zero()
    {
        dvec3 com = get_com();
        for (size_t i = 0; i < points.size(); ++i)
            points[i] = points[i] - com;
    }

    //This function rotates all the mascon points, such that the resulted inertia matrix becomes diagonal. The rotation happens via left-multiplication of all
    //the points (vectors) with a rotation matrix, which is basically the eigenvectors of the inertia matrix. Again homogeneous mass density is assumed.
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

        //Apply the rotation to all the points of the rigid body.
        for (size_t i = 0; i < points.size(); ++i)
            points[i] = dot(fin_rot_mat, points[i]);

        //Done!
    }

    //This function fills with point mascons the interior of a given polyhedron surface (poly), in accordance with a given grid resolution (grid_reso).
    //The function assumes a (slightly scaled) circumscribed cuboid-grid around the input polyhedron. The grid is traversed along x,y,z axes and at each grid-point,
    //the function checks whether or not that grid-point is inside the polyhedron. If yes, the grid-point is appended as point mascon to the member variable 'points'.
    void generate_from_polyhedron(polyhedron &poly, const uvec3 &grid_reso)
    {
        points.clear();

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
                    if (poly.encloses_point(r)) //Raycast test.
                        points.push_back(r);
                }
            }
        }
    }

    //This function fills with point mascons the interior of a given ellipsoid (semiaxes), in accordance with a given grid resolution (grid_reso).
    //The function assumes a (slightly scaled) circumscribed cuboid-grid around the input ellipsoid. The grid is traversed along x,y,z axes and at each grid-point,
    //the function checks whether or not that grid-point is inside the ellipsoid, via the equation x^2/a^2 + y^2/b^2 + z^2/c^2 = 1. If yes, the grid-point is
    //appended as point mascon to the member variable 'points'.
    void generate_from_ellipsoid(const dvec3 &semiaxes, const uvec3 &grid_reso)
    {
        points.clear();

        double a = 1.1*semiaxes[0];
        double b = 1.1*semiaxes[1];
        double c = 1.1*semiaxes[2];
        for (unsigned int i = 0; i < grid_reso[0]; ++i)
        {
            double x = -a + 2.0*i*a/((double)grid_reso[0] - 1.0);
            for (unsigned int j = 0; j < grid_reso[1]; ++j)
            {
                double y = -b + 2.0*j*b/((double)grid_reso[1] - 1.0);
                for (unsigned int k = 0; k < grid_reso[2]; ++k)
                {
                    double z = -c + 2.0*k*c/((double)grid_reso[2] - 1.0);
                    if ( x*x/(a*a) + y*y/(b*b) + z*z/(c*c) < 1.0 )
                        points.push_back(dvec3{x,y,z});
                }
            }
        }
    }
};

#endif
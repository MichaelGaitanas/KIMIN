#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include<cmath>
#include<algorithm>
#include<limits>

#include<Eigen/Dense>

#include"typedef.h"
#include"linalg.h"
#include"polyhedron.h"

//Quaternion odes rhs for a rigid body. The angular velocity w is in the body frame.
dvec4 quat_rhs(const dvec4 &q, const dvec3 &w)
{
    //This comment is useless, but I wanted you to know that right now, I am listening to this
    //https://www.youtube.com/watch?v=A_tA7G1Y8Ew&list=RDX2xpR5EoAGY&index=2&ab_channel=FrankKlepacki-Topic
    //song while writing the code.
    const double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    const double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    const double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    const double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0, dq1, dq2, dq3};
}

//Euler odes rhs for a rigid body, assuming I[][] is diagonal (principal axes frame).
//Angular velocity w, moment of inertia I and torque tau are in the body frame.
dvec3 euler_rhs(const dvec3 &w, const dmat3 &I, const dvec3 &tau)
{
    const double dw0 = (tau[0] + w[1]*w[2]*(I[1][1] - I[2][2]))/I[0][0];
    const double dw1 = (tau[1] + w[2]*w[0]*(I[2][2] - I[0][0]))/I[1][1];
    const double dw2 = (tau[2] + w[0]*w[1]*(I[0][0] - I[1][1]))/I[2][2];
    return {dw0, dw1, dw2};
}

//Sphere - sphere collision detection criterion.
bool sphere_sphere_collision(const double distance, const double R1, const double R2)
{
    return distance <= (R1 + R2) ? true : false;
}

//Sphere - point collision detection criterion.
bool sphere_point_collision(const double distance, const double R)
{
    return (distance <= R) ? true : false;
}

//Polyhedron - point collision detection criterion.
bool polyhedron_point_collision(const polyhedron &poly, const dmat3 &A, const dvec3 &r, const dvec3 &r_sp)
{
    //Transform spacecraft into the polyhedron's local (body) frame.
    const dvec3 r_body_frame = dot(transpose(A), (r_sp - r));
    return (poly.encloses_point(r_body_frame)) ? true : false;
}

//Triangle - triangle collision detection criterion.
bool triangle_triangle_collision(const dvec3 &vtx0, const dvec3 &vtx1, const dvec3 &vtx2,
                                 const dvec3 &utx0, const dvec3 &utx1, const dvec3 &utx2,
                                 const double eps = 1e-12)
{
    const Eigen::Vector3d V0(vtx0[0], vtx0[1], vtx0[2]);
    const Eigen::Vector3d V1(vtx1[0], vtx1[1], vtx1[2]);
    const Eigen::Vector3d V2(vtx2[0], vtx2[1], vtx2[2]);

    const Eigen::Vector3d U0(utx0[0], utx0[1], utx0[2]);
    const Eigen::Vector3d U1(utx1[0], utx1[1], utx1[2]);
    const Eigen::Vector3d U2(utx2[0], utx2[1], utx2[2]);

    const Eigen::Vector3d E1 = V1 - V0;
    const Eigen::Vector3d E2 = V2 - V0;
    const Eigen::Vector3d F1 = U1 - U0;
    const Eigen::Vector3d F2 = U2 - U0;
    const Eigen::Vector3d Delta = U0 - V0;

    //Build M = [E1, E2, -F1].
    Eigen::Matrix3d M;
    M.col(0) =  E1;
    M.col(1) =  E2;
    M.col(2) = -F1;

    //Check for degenerate M.
    if (std::abs(M.determinant()) < eps) //Triangles are coplanar or degenerate. Treat as no intersection.
        return false;

    //Solve for a = M^(-1)*Delta, b = M^(-1)*F2
    Eigen::ColPivHouseholderQR<Eigen::Matrix3d> solver(M);
    Eigen::Vector3d a = solver.solve(Delta);
    Eigen::Vector3d b = solver.solve(F2);

    double a_u = a.x(), a_v = a.y(), a_s = a.z();
    double b_u = b.x(), b_v = b.y(), b_s = b.z();

    std::vector<double> lowers{0.0};
    std::vector<double> uppers;

    auto add_bound = [&](double ai, double bi) -> bool
    {
        //Enforce ai + bi*t >= 0
        if (std::abs(bi) < eps)
        {
            if (ai < 0.0)
                return false;
        }
        else
        {
            double bound = -ai/bi;
            if (bi > 0) 
                lowers.push_back(bound);
            else
                uppers.push_back(bound);
        }
        return true;
    };

    //u >= 0, v >= 0, s >= 0
    if (!add_bound(a_u, b_u)) return false;
    if (!add_bound(a_v, b_v)) return false;
    if (!add_bound(a_s, b_s)) return false;

    //u + v <= 1  =>  (a_u + a_v) + (b_u + b_v) * t <= 1
    {
        double A = a_u + a_v;
        double B = b_u + b_v;
        if (std::abs(B) < eps)
        {
            if (A > 1.0)
                return false;
        }
        else
        {
            double bound = (1.0 - A)/B;
            if (B > 0)
                uppers.push_back(bound);
            else
                lowers.push_back(bound);
        }
    }

    //s + t <= 1  =>  a_s + (b_s + 1) * t <= 1
    {
        double C = a_s;
        double D = b_s + 1.0;
        if (std::abs(D) < eps)
        {
            if (C > 1.0)
                return false;
        }
        else
        {
            double bound = (1.0 - C)/D;
            if (D > 0)
                uppers.push_back(bound);
            else
                lowers.push_back(bound);
        }
    }

    //Final supremum/infimum check.
    double t_min = *std::max_element(lowers.begin(), lowers.end());
    double t_max = uppers.empty()? std::numeric_limits<double>::infinity() : *std::min_element(uppers.begin(), uppers.end());

    return (t_min <= t_max);
}

//Polyhedron - polyhedron collision detection criterion.
bool polyhedron_polyhedron_collision(const polyhedron &poly1, const dmat3 &A1, const dvec3 &r1,
                                     const polyhedron &poly2, const dmat3 &A2, const dvec3 &r2)
{
    //Extract verts and faces.
    const dmatnx3 &verts1 = poly1.get_verts();
    const umatnx3 &faces1 = poly1.get_faces();

    const dmatnx3 &verts2 = poly2.get_verts();
    const umatnx3 &faces2 = poly2.get_faces();

    //Pre‑allocate memory for the transformed (world spaced) vertex matrices.
    dmatnx3 tverts1(verts1.size());
    dmatnx3 tverts2(verts2.size());

    //Now apply the spatial transformations to the polyhedra.
    for (size_t i = 0; i < verts1.size(); ++i)
        tverts1[i] = r1 + dot(A1, verts1[i]);
    for (size_t i = 0; i < verts2.size(); ++i)
        tverts2[i] = r2 + dot(A2, verts2[i]);

    //Brute force triangle ‐ triangle check.
    for (auto const &f1 : faces1)
    {
        const dvec3 &V0 = tverts1[f1[0]];
        const dvec3 &V1 = tverts1[f1[1]];
        const dvec3 &V2 = tverts1[f1[2]];
        for (auto const &f2 : faces2)
        {
            const dvec3 &U0 = tverts2[f2[0]];
            const dvec3 &U1 = tverts2[f2[1]];
            const dvec3 &U2 = tverts2[f2[2]];
            if (triangle_triangle_collision(V0,V1,V2, U0,U1,U2))
                return true;
        }
    }
    return false;
}

//This function tells if an intersection point exists between a line segment (formed by the vectors r1,r2)
//and a sphere with center at rsphere and radius R.
bool line_sphere_intersection(const dvec3 &r1, const dvec3 &r2, const dvec3 &rsphere, const double R)
{
    dvec3 dr1 = r2 - r1;
    dvec3 dr2 = r1 - rsphere;
    double dr1len = length(dr1);
    double dr2len = length(dr2);

    double a = dr1len*dr1len;
    double b = 2.0*dot(dr1,dr2);
    double c = dr2len*dr2len - R*R;

    double D = b*b - 4.0*a*c;

    if (D < 0.0)
        return false;

    double root1 = (-b - sqrt(D))/(2.0*a);
    double root2 = (-b + sqrt(D))/(2.0*a);
    if ((root1 >= 0.0 && root1 <= 1.0) || (root2 >= 0.0 && root2 <= 1.0))
        return true; //Found intersection.

    return false;
}

#endif

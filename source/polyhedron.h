#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include<GL/glew.h>

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<fstream>
#include<algorithm>
#include<vector>
#include<string>

#include<Eigen/Dense>

#include"typedef.h"
#include"constant.h"
#include"linalg.h"

class polyhedron
{
private:
    dmatnx3 verts;
    umatnx3 faces;
    mutable dmatnx3 norms;
    umatnx2 edges;

    double vol; //Polyhedron's total volume.

    //Boolean flags that track the state of the instance.
    mutable bool norms_exist;
    bool edges_exist, vol_exists;

    //OpenGL related members. They are used after the numerical integration ends, in order to render the 3D scene.
    bool gl_ready; //Whether or not the mesh data are uploaded to the gpu.
    unsigned int gl_vao, gl_vbo; //Vertex array and vertex buffer objects.
    size_t gl_vertex_count; //Triangle vertices in the interleaved buffer.

public:
    polyhedron() : verts(),
                   faces(),
                   norms(),
                   edges(),
                   vol(0.0),
                   norms_exist(false),
                   edges_exist(false),
                   vol_exists(false),
                   gl_ready(false),
                   gl_vao(0),
                   gl_vbo(0),
                   gl_vertex_count(0)
    { }

    const dmatnx3 &get_verts() const
    {
        return verts;
    }

    const umatnx3 &get_faces() const
    {
        return faces;
    }

    const dmatnx3 &get_norms() const
    {
        return norms;
    }

    const umatnx2 &get_edges() const
    {
        return edges;
    }

    //Load the .obj file assuming it contains only vertcies and faces ('v x y z' and 'f i j k').
    void load_obj_file(const char *path)
    {
        //Reset the polyhedron's state first.
        verts.clear(); faces.clear(); norms.clear(); edges.clear();
        vol = 0.0;
        norms_exist = edges_exist = vol_exists = false;
        //*this = polyhedron{}; //Reset all members to their constructor defaults.
        clear_gl_mesh(); //Tear down any gl* state from the previous mesh (if any).

        //Now proceed to loading.

        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            fprintf(stderr, "[Warning] : In polyhedron::load_obj_file(), '%s' could not be opened. Returning...\n", path);
            return;
        }

        double x,y,z; //Vertices.
        unsigned int vi1,vi2,vi3; //Faces.
        std::string line;
        while (getline(objfile, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                sscanf(line.c_str(), "v %lf %lf %lf", &x, &y, &z);
                verts.push_back(dvec3{x,y,z});
            }
            else if (line[0] == 'f' && line[1] == ' ') //Then we have a face line.
            {
                sscanf(line.c_str(), "f %u %u %u", &vi1,&vi2,&vi3);
                faces.push_back(uvec3{vi1-1, vi2-1, vi3-1});
            }
        }
        objfile.close();
    }

    //Export to an .obj file the current vertices and faces of the polyhedron mesh ('v x y z' and 'f i j k').
    void export_obj_file_vf(const char *path)
    {
        FILE *fp = fopen(path,"w");
        for (size_t i = 0; i < verts.size(); ++i)
            fprintf(fp, "v %.15lf %.15lf %.15lf\n", verts[i][0], verts[i][1], verts[i][2]);
        for (size_t i = 0; i < faces.size(); ++i)
            fprintf(fp, "f %u %u %u\n", faces[i][0]+1, faces[i][1]+1, faces[i][2]+1);
        fclose(fp);
    }

    //Generate the polyhderon's (flat) normals.
    void gen_norms() const
    {
        if (norms_exist) return; //Do not repeat the same shit...

        if (faces.empty())
        {
            fprintf(stderr, "[Warning] : In polyhedron::gen_norms(), faces.empty() = true. No normals are generated. Returning...\n");
            return;
        }
        
        norms.clear();
        norms.resize(faces.size());
        for (size_t i = 0; i < faces.size(); ++i)
        {
            const dvec3 &p0 = verts[faces[i][0]];
            const dvec3 &p1 = verts[faces[i][1]];
            const dvec3 &p2 = verts[faces[i][2]];
            dvec3 perp = cross(p1-p0, p2-p1);
            double len = length(perp);
            if (len > 1e-15)
                norms[i] = perp/len; //Unit normal.
            else
                norms[i] = dvec3{0.0,0.0,0.0}; //Degenerate case...
        }
        norms_exist = true;
    }

    //Generate the polyhedron's (unique) edge indices.
    void gen_edges()
    {
        if (edges_exist) return;
        
        if (faces.empty())
        {
            fprintf(stderr, "[Warning] : In polyhedron::gen_edges(), faces.empty() = true. No edges are generated. Returning...\n");
            return;
        }

        edges.clear();
        edges.resize(3*faces.size());
        size_t j = 0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            unsigned int f0 = faces[i][0];
            unsigned int f1 = faces[i][1];
            unsigned int f2 = faces[i][2];
            //Store each edge as a sorted pair (smallest index first).
            edges[j++] = {std::min(f0,f1), std::max(f0,f1)};
            edges[j++] = {std::min(f1,f2), std::max(f1,f2)};
            edges[j++] = {std::min(f2,f0), std::max(f2,f0)};
        }
        edges.resize(j); //Remove any unused capacity if any...
        std::sort(edges.begin(), edges.end()); //Sort the edges to prepare for duplicate removal.
        
        edges.erase(std::unique(edges.begin(), edges.end()), edges.end()); //Now remove duplicates.

        edges_exist = true;
    }

    //This member function decides whether or not the polyhedron is a closed manifold, i.e. closed surface (with no boundaries).
    //E.g. Cuboid, asteroid, torus, etc...
    bool is_closed_manifold()
    {   
        if (faces.empty())
        {
            fprintf(stderr, "[Warning] : In polyhedron::is_closed_manifold(), faces.empty() = true. Returning false.\n");
            return false; //Because an empty mesh isn't a closed manifold.
        }

        umatnx2 local_edges;
        local_edges.resize(3*faces.size());
        size_t j = 0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            unsigned int f0 = faces[i][0];
            unsigned int f1 = faces[i][1];
            unsigned int f2 = faces[i][2];
            //Store each edge as a sorted pair (smallest index first).
            local_edges[j++] = {std::min(f0,f1), std::max(f0,f1)};
            local_edges[j++] = {std::min(f1,f2), std::max(f1,f2)};
            local_edges[j++] = {std::min(f2,f0), std::max(f2,f0)};
        }
        local_edges.resize(j); //Remove any unused capacity if any...
        std::sort(local_edges.begin(), local_edges.end()); //Sort the edges lexicographically.

        //Now, each unique edge should appear exactly twice.
        for (size_t i = 0; i < local_edges.size(); )
        {
            size_t count = 1;
            for (size_t j = i + 1; j < local_edges.size(); ++j)
            {
                if (local_edges[i][0] == local_edges[j][0] && local_edges[i][1] == local_edges[j][1])
                    ++count;
                else
                    break;
            }
            if (count != 2)
                return false;
            i += count;
        }
        
        return true;
    }

    bool is_kimin_valid_obj(const char *path)
    {
        std::ifstream objfile(path);
        if (!objfile.is_open())
        {
            fprintf(stderr, "[Warning] : In polyhedron::is_kimin_valid_obj(), '%s' could not be opened. Returning false.\n", path);
            return false;
        }
    
        bool has_verts = false, has_faces = false;
        std::string line;
        while (getline(objfile, line))
        {
            if (line[0] == 'v' && line[1] == ' ')
                has_verts = true;
            else if (line[0] == 'f' && line[1] == ' ')
                has_faces = true;
            else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') //This means that the .obj has normals, which we reject.
                return false;
            else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') //This means that the .obj has UVs, which we reject.
                return false;
        }
        objfile.close();

        if (has_verts && has_faces)
            return true;

        return false;
    }

    //Build and upload to the GPU an interleaved (position + normal) buffer for flat shading (Lambert).
    void set_as_gl_mesh()
    {
        //If the polyhedron is already ready for rendering, exit the function.
        if (gl_ready) return;

        //Safety check. You never know...
        if (faces.empty() || verts.empty() || norms.empty())
        {
            fprintf(stderr, "[Warning] : In polyhedron::set_as_gl_mesh(), essential mesh data (vertices || faces || normals) are missing. Returning...\n");
            return;
        }

        std::vector<float> interleaved_buffer;
        interleaved_buffer.resize(3*6*faces.size()); //3 vertices per face, 6 floats each.
        size_t j = 0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            const dvec3 &n = norms[i];
            for (int k = 0; k < 3; k++)
            {
                const dvec3 &v = verts[faces[i][k]];

                interleaved_buffer[j++] = static_cast<float>(v[0]);
                interleaved_buffer[j++] = static_cast<float>(v[1]);
                interleaved_buffer[j++] = static_cast<float>(v[2]);

                interleaved_buffer[j++] = static_cast<float>(n[0]);
                interleaved_buffer[j++] = static_cast<float>(n[1]);
                interleaved_buffer[j++] = static_cast<float>(n[2]);
            }
        }
        gl_vertex_count = interleaved_buffer.size()/6; //Because each vertex has 6 float attributes bound.

        glGenVertexArrays(1, &gl_vao);
        glBindVertexArray(gl_vao);

        glGenBuffers(1, &gl_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved_buffer.size()*sizeof(float), interleaved_buffer.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        gl_ready = true;
    }

    //Render the polyhedral mesh.
    void render()
    {
        if (!gl_ready) return; //Guard.
        
        glBindVertexArray(gl_vao);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)gl_vertex_count);
        glBindVertexArray(0);
    }

    //Cleanup GPU resources. This basically resets the OpenGL - related members back to what u see in the beginning of the class.
    void clear_gl_mesh()
    {
        if (gl_vbo != 0)
        {
            glDeleteBuffers(1, &gl_vbo);
            gl_vbo = 0;
        }
        if (gl_vao != 0)
        {
            glDeleteVertexArrays(1, &gl_vao);
            gl_vao = 0;
        }
        gl_vertex_count = 0;
        gl_ready = false;
    }

    //Farthest vertex distance with respect to the local coordinate system.
    double get_farthest_vertex_distance()
    {
        //Apply comparison to the squared length, as it is a monotonic increasing function, in order to avoid calls to sqrt.
        double farthest2 = dot(verts[0], verts[0]); //Assume that the farthest vertex distance is the first one.
        for (size_t i = 1; i < verts.size(); ++i)
        {
            double dist2 = dot(verts[i], verts[i]);
            if (dist2 > farthest2)
                farthest2 = dist2;
        }
        return sqrt(farthest2);
    }

    //Nearest vertex distance with respect to the local coordinate system.
    double get_nearest_vertex_distance()
    {
        //Apply comparison to the squared length, as it is a monotonic increasing function, in order to avoid calls to sqrt.
        double nearest2 = dot(verts[0], verts[0]); //Assume that the nearest vertex distance is the first one.
        for (size_t i = 1; i < verts.size(); ++i)
        {
            double dist2 = dot(verts[i], verts[i]);
            if (dist2 < nearest2)
                nearest2 = dist2;
        }
        return sqrt(nearest2);
    }

    //Get the (xmax, ymax, zmax) of the polyhedron with respect to the local coordinate system.
    dvec3 get_farthest_coord_per_axis()
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

    //Get the (xmin, ymin, zmin) of the polyhedron with respect to the local coordinate system.
    dvec3 get_nearest_coord_per_axis()
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
    bool encloses_point(const dvec3 &r) const
    {
        gen_norms();

        //Ray's destination point. It is assumed to be very far away, aiming to be outside of the polyhedron. The irrational numbers help avoid degeneracies.
        dvec3 pdest = 10000000.0*dvec3{pi, exp(1.0), sqrt(2.0)};
        
        size_t intersections = 0;

        //The polyhderon is basically a collection of triangles. To find intersections between the ray and the polyhderon, we essentially loop through all the faces and check.
        for (size_t j = 0; j < faces.size(); ++j)
        {
            //Define the triangle j from the 3 vertices p0,p1,p2.
            const dvec3 &p0 = verts[faces[j][0]];
            const dvec3 &p1 = verts[faces[j][1]];
            const dvec3 &p2 = verts[faces[j][2]];
            //By solving the equation of a line and a plane, we find intersection point pj.
            double lam = ( (p0[0] - r[0])*norms[j][0] + (p0[1] - r[1])*norms[j][1] + (p0[2] - r[2])*norms[j][2] )/( (pdest[0] - r[0])*norms[j][0] + (pdest[1] - r[1])*norms[j][1] + (pdest[2] - r[2])*norms[j][2] );
            dvec3 pj = dvec3{ r[0] + lam*(pdest[0] - r[0]),
                              r[1] + lam*(pdest[1] - r[1]),
                              r[2] + lam*(pdest[2] - r[2]) };

            //We must check however if the ray intersects the triangle j and not the whole extended mathematical plane.

            //Form the following 3 triangles and calculate their area.
            double Aj01 = 0.5*length(cross(p0-pj, p1-p0));
            double Aj12 = 0.5*length(cross(p1-pj, p2-p1));
            double Aj20 = 0.5*length(cross(p2-pj, p0-p2));
            double A012 = 0.5*length(cross(p1-p0, p2-p1));

            //If the sum of the 3 areas is equal to the area of the surface triangle, then pj sits upon the surface of the triangle.
            if ( fabs(Aj01 + Aj12 + Aj20 - A012) <= 1e-12 && pj[0] > r[0] && pj[1] > r[1] && pj[2] > r[2] ) //I must to fix this...
                ++intersections;
        }
        
        if (intersections%2 == 1)
            return true;

        return false;
    }

    //Calculate the center of mass of the surface vertices of the polyhedron, assuming homogeneous mass density.
    dvec3 get_com_vertices()
    {
        dvec3 com = dvec3{0.0,0.0,0.0};
        for (size_t i = 0; i < verts.size(); ++i)
            com = com + verts[i];
        return com/verts.size();
    }

    //Calculate the polyhderon's total volume.
    double get_vol()
    {
        if (vol_exists) return vol;

        gen_norms();

        vol = 0.0;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            const dvec3 &p0 = verts[faces[i][0]];
            const dvec3 &p1 = verts[faces[i][1]];
            const dvec3 &p2 = verts[faces[i][2]];
            double d = p0[0]*norms[i][0] + p0[1]*norms[i][1] + p0[2]*norms[i][2]; //x*nx + y*ny + z*nz - d = 0 (plane equation).
            double A = 0.5*length(cross(p1-p0, p2-p1));
            vol += d*A/3.0;
        }
        vol_exists = true;
        return vol;
    }

    //Calculate the center of mass of the polyhedron, assuming homogeneous mass density.
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

            //Pick the index of the greatest absolute normal component, as this will be the most stable in the computation.
            double absnx = fabs(nx);
            double absny = fabs(ny);
            double absnz = fabs(nz);

            int best_axis = 2; //Assume 0 -> x, 1 -> y, 2 -> z
            double best_val = absnz; 
            if (absny > best_val)
            {
                best_axis = 1;
                best_val = absny;
            }
            if (absnx > best_val)
            {
                best_axis = 0;
                best_val = absnx;
            }

            if (best_axis == 2) //Division by nz.
            {
                double Jacxy = fabs(x21*y31 - x31*y21);
                double coeffnz = d*Jacxy/fabs(nz);
                com[0] += coeffnz*(3.0*x1 + x21 + x31)/6.0;
                com[1] += coeffnz*(3.0*y1 + y21 + y31)/6.0;
                com[2] += coeffnz*(3.0*d - nx*(3.0*x1 + x21 + x31) - ny*(3.0*y1 + y21 + y31))/(6.0*nz);
            }
            else if (best_axis == 1) //Division by ny.
            {
                double Jacxz = fabs(x21*z31 - x31*z21);
                double coeffny = d*Jacxz/fabs(ny);
                com[0] += coeffny*(3.0*x1 + x21 + x31)/6.0;
                com[1] += coeffny*(3.0*d - nx*(3.0*x1 + x21 + x31) - nz*(3.0*z1 + z21 + z31))/(6.0*ny);
                com[2] += coeffny*(3.0*z1 + z21 + z31)/6.0;
            }
            else //Division by nx.
            {
                    double Jacyz = fabs(y21*z31 - y31*z21);
                    double coeffnx = d*Jacyz/fabs(nx);
                    com[0] += coeffnx*(3.0*d - ny*(3.0*y1 + y21 + y31) - nz*(3.0*z1 + z21 + z31))/(6.0*nx);
                    com[1] += coeffnx*(3.0*y1 + y21 + y31)/6.0;
                    com[2] += coeffnx*(3.0*z1 + z21 + z31)/6.0;
            }
        }
        return com/(4.0*get_vol());
    }

    //This member function calculates all the inertial integrals of order 2 of the polyhedron, assuming homogeneous mass density.
    //If only_inertial_integrals = false, then the function utilizes the inertial integrals to compute the inertia matrix. Else, only the
    //DIAGONAL inertial integrals are returned.
    dmat3 get_inertia(const double M, bool only_inertial_integrals = false)
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
            
            //Pick the index of the greatest absolute normal component, as this will be the most stable in the computation.
            double absnx = fabs(nx);
            double absny = fabs(ny);
            double absnz = fabs(nz);

            int best_axis = 2; //Assume 0 -> x, 1 -> y, 2 -> z
            double best_val = absnz; 
            if (absny > best_val)
            {
                best_axis = 1;
                best_val = absny;
            }
            if (absnx > best_val)
            {
                best_axis = 0;
                best_val = absnx;
            }

            if (best_axis == 2) //Division by nz.
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
            else if (best_axis == 1) //Division by ny.
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
            else //Division by nx.
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

        double ord2_coeff = M/(5.0*get_vol());
        Jxx *= ord2_coeff;
        Jyy *= ord2_coeff;
        Jzz *= ord2_coeff;
        Jxy *= ord2_coeff;
        Jxz *= ord2_coeff;
        Jyz *= ord2_coeff;

        if (only_inertial_integrals)
            return {{{Jxx,  0.0,  0.0},
                     {0.0,  Jyy,  0.0},
                     {0.0,  0.0,  Jzz}}};

        return {{{Jyy + Jzz,    -Jxy,       -Jxz   },
                 {  -Jxy,     Jxx + Jzz,    -Jyz   },
                 {  -Jxz,       -Jyz,     Jxx + Jyy}}};
    }

    //This function returns the polyhedron's diagonal inertial integrals of order 2, assuming homogeneous mass density. Though this can
    //be accomplished via get_inertia(M, true), we just wrap it in an alias name function.
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

    dtens get_inertial_integrals_ord3(const double M)
    {
        gen_norms();

        double Jxx = 0.0, Jyy = 0.0, Jzz = 0.0;
        double Jxxx = 0.0, Jyyy = 0.0, Jzzz = 0.0, Jxxy = 0.0, Jxyy = 0.0, Jxxz = 0.0, Jxzz = 0.0, Jyyz = 0.0, Jyzz = 0.0, Jxyz = 0.0;
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
            
            //Pick the index of the greatest absolute normal component, as this will be the most stable in the computation.
            double absnx = fabs(nx);
            double absny = fabs(ny);
            double absnz = fabs(nz);

            int best_axis = 2; //Assume 0 -> x, 1 -> y, 2 -> z
            double best_val = absnz; 
            if (absny > best_val)
            {
                best_axis = 1;
                best_val = absny;
            }
            if (absnx > best_val)
            {
                best_axis = 0;
                best_val = absnx;
            }

            if (best_axis == 2) //Division by nz.
            {
                double Jacxy = fabs(x21*y31 - x31*y21);
                double coeffnz = d*Jacxy/fabs(nz);
                Jxx +=  coeffnz*(6.0*x1*x1 + x21*x21 + x21*x31 + x31*x31 + 4.0*x1*(x21 + x31))/12.0;
                Jyy +=  coeffnz*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                Jzz +=  coeffnz*(6*pow(d,2) + pow(nx,2)*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) - 4*d*(nx*(3*x1 + x21 + x31) + ny*(3*y1 + y21 + y31)) + nx*ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(12.*pow(nz,2));
                Jxxx += coeffnz*((10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))/20.);
                Jyyy += coeffnz*((10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))/20.);
                Jzzz += coeffnz*(-(pow(nz,-3)*(2*(nx*(3*x1 + x21 + x31) + ny*(3*y1 + y21 + y31))*pow(d,2) - 2*pow(d,3) - d*(nx*ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + pow(nx,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + pow(ny,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + (ny*pow(nx,2)*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + pow(nx,3)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) + pow(ny,3)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) + nx*pow(ny,2)*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))/5.))/4.);
                Jxxy += coeffnz*((x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2))/60.);
                Jxyy += coeffnz*((5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))/60.);
                Jxxz += coeffnz*(-(pow(nz,-1)*(-5*d*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + ny*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*nx*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))))/60.);
                Jxzz += coeffnz*((pow(nz,-2)*(10*(3*x1 + x21 + x31)*pow(d,2) + 2*nx*ny*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*pow(nx,2)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) - 5*d*(ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 2*nx*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2))) + pow(ny,2)*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))))/60.);
                Jyyz += coeffnz*((pow(nz,-1)*(5*d*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + nx*(-5*(4*x1 + 2*x21 + x31)*y1*y21 - (5*(4*x1 + x21 + 2*x31)*y1 + 5*x1*y21 + 2*(x21 + x31)*y21)*y31 - 10*(3*x1 + x21 + x31)*pow(y1,2) - (5*x1 + 3*x21 + x31)*pow(y21,2) - (5*x1 + x21 + 3*x31)*pow(y31,2)) - 3*ny*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))))/60.);
                Jyzz += coeffnz*((pow(nz,-2)*(10*(3*y1 + y21 + y31)*pow(d,2) + pow(nx,2)*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*pow(ny,2)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) - 5*d*(nx*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 2*ny*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + 2*nx*ny*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))))/60.);
                Jxyz += coeffnz*((pow(nz,-1)*(5*d*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) - 2*(nx*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + ny*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))))/120.);
            }
            else if (best_axis == 1) //Division by ny.
            {
                double Jacxz = fabs(x21*z31 - x31*z21);
                double coeffny = d*Jacxz/fabs(ny);
                Jxx +=  coeffny*(6.0*x1*x1 + x21*x21 + x21*x31 + x31*x31 + 4.0*x1*(x21 + x31))/12.0;
                Jyy +=  coeffny*(6*pow(d,2) + pow(nx,2)*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(nx*(3*x1 + x21 + x31) + nz*(3*z1 + z21 + z31)) + nx*nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(12.*pow(ny,2));
                Jzz +=  coeffny*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                Jxxx += coeffny*((10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))/20.);
                Jyyy += coeffny*(-(pow(ny,-3)*(2*(nx*(3*x1 + x21 + x31) + nz*(3*z1 + z21 + z31))*pow(d,2) - 2*pow(d,3) - d*(nx*nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + pow(nx,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + pow(nz,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + (nz*pow(nx,2)*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + pow(nx,3)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3)) + nx*pow(nz,2)*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + pow(nz,3)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)))/5.))/4.);
                Jzzz += coeffny*((10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))/20.);
                Jxxy += coeffny*(-(pow(ny,-1)*(-5*d*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + nz*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 3*nx*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))))/60.);
                Jxyy += coeffny*((pow(ny,-2)*(10*(3*x1 + x21 + x31)*pow(d,2) + 2*nx*nz*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 3*pow(nx,2)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) - 5*d*(nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 2*nx*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2))) + pow(nz,2)*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))))/60.);
                Jxxz += coeffny*((x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2))/60.);
                Jxzz += coeffny*((5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))/60.);
                Jyyz += coeffny*((pow(ny,-2)*(10*(3*z1 + z21 + z31)*pow(d,2) + pow(nx,2)*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) - 5*d*(nx*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 2*nz*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + 2*nx*nz*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + 3*pow(nz,2)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))))/60.);
                Jyzz += coeffny*((pow(ny,-1)*(5*d*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + nx*(-5*(4*x1 + 2*x21 + x31)*z1*z21 - (5*(4*x1 + x21 + 2*x31)*z1 + 5*x1*z21 + 2*(x21 + x31)*z21)*z31 - 10*(3*x1 + x21 + x31)*pow(z1,2) - (5*x1 + 3*x21 + x31)*pow(z21,2) - (5*x1 + x21 + 3*x31)*pow(z31,2)) - 3*nz*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))))/60.);
                Jxyz += coeffny*((pow(ny,-1)*(5*d*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) - 2*(nx*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + nz*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/120.);
            }
            else //Division by nx.
            {
                double Jacyz = fabs(y21*z31 - y31*z21);
                double coeffnx = d*Jacyz/fabs(nx);
                Jxx +=  coeffnx*(6*pow(d,2) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31)) + ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(12.*pow(nx,2));
                Jyy +=  coeffnx*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                Jzz +=  coeffnx*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                Jxxx += coeffnx*(-(pow(nx,-3)*(2*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31))*pow(d,2) - 2*pow(d,3) - d*(ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + pow(ny,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(nz,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + (nz*pow(ny,2)*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + pow(ny,3)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + ny*pow(nz,2)*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + pow(nz,3)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)))/5.))/4.);
                Jyyy += coeffnx*((10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))/20.);
                Jzzz += coeffnx*((10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))/20.);
                Jxxy += coeffnx*((pow(nx,-2)*(10*(3*y1 + y21 + y31)*pow(d,2) + 2*ny*nz*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*pow(ny,2)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) - 5*d*(nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 2*ny*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + pow(nz,2)*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))))/60.);
                Jxyy += coeffnx*(-(pow(nx,-1)*(-5*d*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + nz*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*ny*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))))/60.);
                Jxxz += coeffnx*((pow(nx,-2)*(10*(3*z1 + z21 + z31)*pow(d,2) + pow(ny,2)*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*pow(nz,2)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2))) - 5*d*(ny*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 2*nz*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + 2*ny*nz*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))))/60.);
                Jxzz += coeffnx*((pow(nx,-1)*(5*d*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + ny*(-5*(4*y1 + 2*y21 + y31)*z1*z21 - (5*(4*y1 + y21 + 2*y31)*z1 + 5*y1*z21 + 2*(y21 + y31)*z21)*z31 - 10*(3*y1 + y21 + y31)*pow(z1,2) - (5*y1 + 3*y21 + y31)*pow(z21,2) - (5*y1 + y21 + 3*y31)*pow(z31,2)) - 3*nz*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))))/60.);
                Jyyz += coeffnx*((y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2))/60.);
                Jyzz += coeffnx*((5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))/60.);
                Jxyz += coeffnx*((pow(nx,-1)*(5*d*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) - 2*(ny*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + nz*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/120.);
            }
        }

        const int ord = 3;
        dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //Initialize the tensor with zeros.
        
        double ord2_coeff = M/(5.0*get_vol());
        J[2][0][0] = Jxx*ord2_coeff;
        J[0][2][0] = Jyy*ord2_coeff;
        J[0][0][2] = Jzz*ord2_coeff;

        double ord3_coeff = M/(6.0*get_vol());
        J[3][0][0] = Jxxx*ord3_coeff;
        J[0][3][0] = Jyyy*ord3_coeff;
        J[0][0][3] = Jzzz*ord3_coeff;
        J[2][1][0] = Jxxy*ord3_coeff;
        J[1][2][0] = Jxyy*ord3_coeff;
        J[2][0][1] = Jxxz*ord3_coeff;
        J[1][0][2] = Jxzz*ord3_coeff;
        J[0][2][1] = Jyyz*ord3_coeff;
        J[0][1][2] = Jyzz*ord3_coeff;
        J[1][1][1] = Jxyz*ord3_coeff;

        return J;
    }

    dtens get_inertial_integrals_ord4(const double M)
    {
        gen_norms();

        double Jxx = 0.0, Jyy = 0.0, Jzz = 0.0;
        double Jxxx = 0.0, Jyyy = 0.0, Jzzz = 0.0, Jxxy = 0.0, Jxyy = 0.0, Jxxz = 0.0, Jxzz = 0.0, Jyyz = 0.0, Jyzz = 0.0, Jxyz = 0.0;
        double Jxxxx = 0.0, Jyyyy = 0.0, Jzzzz = 0.0, Jxxxy = 0.0, Jxyyy = 0.0, Jxxxz = 0.0, Jxzzz = 0.0, Jyyyz = 0.0, Jyzzz = 0.0, Jxxyy = 0.0, Jxxzz = 0.0, Jyyzz = 0.0, Jxxyz = 0.0, Jxyyz = 0.0, Jxyzz = 0.0;
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
            
            //Pick the index of the greatest absolute normal component, as this will be the most stable in the computation.
            double absnx = fabs(nx);
            double absny = fabs(ny);
            double absnz = fabs(nz);

            int best_axis = 2; //Assume 0 -> x, 1 -> y, 2 -> z
            double best_val = absnz; 
            if (absny > best_val)
            {
                best_axis = 1;
                best_val = absny;
            }
            if (absnx > best_val)
            {
                best_axis = 0;
                best_val = absnx;
            }

            if (best_axis == 2) //Division by nz.
            {
                double Jacxy = fabs(x21*y31 - x31*y21);
                double coeffnz = d*Jacxy/fabs(nz);
                Jxx +=  coeffnz*(6.0*x1*x1 + x21*x21 + x21*x31 + x31*x31 + 4.0*x1*(x21 + x31))/12.0;
                Jyy +=  coeffnz*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                Jzz +=  coeffnz*(6*pow(d,2) + pow(nx,2)*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) - 4*d*(nx*(3*x1 + x21 + x31) + ny*(3*y1 + y21 + y31)) + nx*ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)))/(12.*pow(nz,2));
                Jxxx += coeffnz*((10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))/20.);
                Jyyy += coeffnz*((10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))/20.);
                Jzzz += coeffnz*(-(pow(nz,-3)*(2*(nx*(3*x1 + x21 + x31) + ny*(3*y1 + y21 + y31))*pow(d,2) - 2*pow(d,3) - d*(nx*ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + pow(nx,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + pow(ny,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + (ny*pow(nx,2)*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + pow(nx,3)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) + pow(ny,3)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) + nx*pow(ny,2)*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))/5.))/4.);
                Jxxy += coeffnz*((x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2))/60.);
                Jxyy += coeffnz*((5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))/60.);
                Jxxz += coeffnz*(-(pow(nz,-1)*(-5*d*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + ny*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*nx*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))))/60.);
                Jxzz += coeffnz*((pow(nz,-2)*(10*(3*x1 + x21 + x31)*pow(d,2) + 2*nx*ny*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*pow(nx,2)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) - 5*d*(ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 2*nx*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2))) + pow(ny,2)*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))))/60.);
                Jyyz += coeffnz*((pow(nz,-1)*(5*d*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + nx*(-5*(4*x1 + 2*x21 + x31)*y1*y21 - (5*(4*x1 + x21 + 2*x31)*y1 + 5*x1*y21 + 2*(x21 + x31)*y21)*y31 - 10*(3*x1 + x21 + x31)*pow(y1,2) - (5*x1 + 3*x21 + x31)*pow(y21,2) - (5*x1 + x21 + 3*x31)*pow(y31,2)) - 3*ny*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))))/60.);
                Jyzz += coeffnz*((pow(nz,-2)*(10*(3*y1 + y21 + y31)*pow(d,2) + pow(nx,2)*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*pow(ny,2)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) - 5*d*(nx*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 2*ny*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + 2*nx*ny*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))))/60.);
                Jxyz += coeffnz*((pow(nz,-1)*(5*d*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) - 2*(nx*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + ny*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))))/120.);
                Jxxxx += coeffnz*((20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 6*x1*(x21 + x31)*(pow(x21,2) + pow(x31,2)) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + pow(x31,4))/30.);
                Jyyyy += coeffnz*((20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 6*y1*(y21 + y31)*(pow(y21,2) + pow(y31,2)) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + pow(y31,4))/30.);
                Jzzzz += coeffnz*(-(pow(nz,-4)*(20*(nx*(3*x1 + x21 + x31) + ny*(3*y1 + y21 + y31))*pow(d,3) - 15*pow(d,4) - ny*pow(nx,3)*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) - pow(nx,4)*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + 6*x1*(x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + pow(x31,3)) + pow(x31,4)) - 15*pow(d,2)*(nx*ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + pow(nx,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + pow(ny,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) - pow(nx,2)*pow(ny,2)*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) + 6*d*(ny*pow(nx,2)*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + pow(nx,3)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3)) + nx*pow(ny,2)*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))) + pow(ny,3)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3))) - nx*pow(ny,3)*(6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x21*(15*(2*y21 + y31)*pow(y1,2) + 20*pow(y1,3) + 3*y31*pow(y21,2) + 4*pow(y21,3) + 2*y21*pow(y31,2) + 6*y1*(2*y21*y31 + 3*pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3))) - pow(ny,4)*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + 6*y1*(y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + pow(y31,3)) + pow(y31,4))))/30.);
                Jxxxy += coeffnz*((15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3))/120.);
                Jxyyy += coeffnz*((6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) + x21*(3*y31*(4*y1*y21 + 5*pow(y1,2) + pow(y21,2)) + 2*(2*y1 + y21)*(5*y1*y21 + 5*pow(y1,2) + 2*pow(y21,2)) + 2*(3*y1 + y21)*pow(y31,2) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3)))/120.);
                Jxxxz += coeffnz*(-(pow(nz,-1)*(-6*d*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) + ny*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) + 4*nx*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 6*x1*(x21 + x31)*(pow(x21,2) + pow(x31,2)) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + pow(x31,4))))/120.);
                Jxzzz += coeffnz*(-(pow(nz,-3)*(-20*(3*x1 + x21 + x31)*pow(d,3) + 15*pow(d,2)*(ny*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 2*nx*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2))) + 3*ny*pow(nx,2)*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) + 4*pow(nx,3)*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + 6*x1*(x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + pow(x31,3)) + pow(x31,4)) + 2*nx*pow(ny,2)*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) - 6*d*(2*nx*ny*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*pow(nx,2)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3)) + pow(ny,2)*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) + pow(ny,3)*(6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x21*(15*(2*y21 + y31)*pow(y1,2) + 20*pow(y1,3) + 3*y31*pow(y21,2) + 4*pow(y21,3) + 2*y21*pow(y31,2) + 6*y1*(2*y21*y31 + 3*pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3)))))/120.);
                Jyyyz += coeffnz*((pow(nz,-1)*(6*d*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) + nx*(-15*(4*x1 + 2*x21 + x31)*y21*pow(y1,2) - 20*(3*x1 + x21 + x31)*pow(y1,3) - 6*(5*x1 + 3*x21 + x31)*y1*pow(y21,2) - y31*(6*(5*x1 + 2*(x21 + x31))*y1*y21 + 15*(4*x1 + x21 + 2*x31)*pow(y1,2) + (6*x1 + 3*x21 + 2*x31)*pow(y21,2)) - (6*x1 + 4*x21 + x31)*pow(y21,3) - (6*(5*x1 + x21 + 3*x31)*y1 + (6*x1 + 2*x21 + 3*x31)*y21)*pow(y31,2) - (6*x1 + x21 + 4*x31)*pow(y31,3)) - 4*ny*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 6*y1*(y21 + y31)*(pow(y21,2) + pow(y31,2)) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + pow(y31,4))))/120.);
                Jyzzz += coeffnz*(-(pow(nz,-3)*(-20*(3*y1 + y21 + y31)*pow(d,3) + pow(nx,3)*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) + 15*pow(d,2)*(nx*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 2*ny*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + 2*ny*pow(nx,2)*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) - 6*d*(pow(nx,2)*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 2*nx*ny*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))) + 3*pow(ny,2)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3))) + 3*nx*pow(ny,2)*(6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x21*(15*(2*y21 + y31)*pow(y1,2) + 20*pow(y1,3) + 3*y31*pow(y21,2) + 4*pow(y21,3) + 2*y21*pow(y31,2) + 6*y1*(2*y21*y31 + 3*pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3))) + 4*pow(ny,3)*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + 6*y1*(y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + pow(y31,3)) + pow(y31,4))))/120.);
                Jxxyy += coeffnz*((15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))/180.);
                Jxxzz += coeffnz*((pow(nz,-2)*(15*pow(d,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + 3*nx*ny*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) - 6*d*(ny*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + 3*nx*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3))) + 6*pow(nx,2)*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + 6*x1*(x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + pow(x31,3)) + pow(x31,4)) + pow(ny,2)*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))))/180.);
                Jyyzz += coeffnz*((pow(nz,-2)*(15*pow(d,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(nx,2)*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) - 6*d*(nx*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))) + 3*ny*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3))) + 3*nx*ny*(6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x21*(15*(2*y21 + y31)*pow(y1,2) + 20*pow(y1,3) + 3*y31*pow(y21,2) + 4*pow(y21,3) + 2*y21*pow(y31,2) + 6*y1*(2*y21*y31 + 3*pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3))) + 6*pow(ny,2)*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + 6*y1*(y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + pow(y31,3)) + pow(y31,4))))/180.);
                Jxxyz += coeffnz*((pow(nz,-1)*(6*d*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) - 3*nx*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) - 2*ny*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))))))/360.);
                Jxyyz += coeffnz*((pow(nz,-1)*(6*d*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2))) - 2*nx*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) - 3*ny*(6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x21*(15*(2*y21 + y31)*pow(y1,2) + 20*pow(y1,3) + 3*y31*pow(y21,2) + 4*pow(y21,3) + 2*y21*pow(y31,2) + 6*y1*(2*y21*y31 + 3*pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3)))))/360.);
                Jxyzz += coeffnz*(-(pow(nz,-2)*(-15*(4*x1*(3*y1 + y21 + y31) + x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(d,2) - 3*pow(nx,2)*(15*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31))*pow(x1,2) + 20*(3*y1 + y21 + y31)*pow(x1,3) + x31*(6*y1 + 3*y21 + 2*y31)*pow(x21,2) + (6*y1 + 4*y21 + y31)*pow(x21,3) + x21*(6*y1 + 2*y21 + 3*y31)*pow(x31,2) + 6*x1*(x21*x31*(5*y1 + 2*(y21 + y31)) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + (6*y1 + y21 + 4*y31)*pow(x31,3)) - 4*nx*ny*(15*pow(x1,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(x21,2)*(3*y21*y31 + 6*y1*(3*y21 + y31) + 15*pow(y1,2) + 6*pow(y21,2) + pow(y31,2)) + x21*x31*(4*y21*y31 + 12*y1*(y21 + y31) + 15*pow(y1,2) + 3*pow(y21,2) + 3*pow(y31,2)) + pow(x31,2)*(3*y21*y31 + 6*y1*(y21 + 3*y31) + 15*pow(y1,2) + pow(y21,2) + 6*pow(y31,2)) + 6*x1*(x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) + 12*d*(nx*(x21*x31*(5*y1 + 2*(y21 + y31)) + 5*x1*(x21*(4*y1 + 2*y21 + y31) + x31*(4*y1 + y21 + 2*y31)) + 10*(3*y1 + y21 + y31)*pow(x1,2) + (5*y1 + 3*y21 + y31)*pow(x21,2) + (5*y1 + y21 + 3*y31)*pow(x31,2)) + ny*(5*x1*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + x21*(2*y21*y31 + 5*y1*(2*y21 + y31) + 10*pow(y1,2) + 3*pow(y21,2) + pow(y31,2)) + x31*(2*y21*y31 + 5*y1*(y21 + 2*y31) + 10*pow(y1,2) + pow(y21,2) + 3*pow(y31,2)))) - 3*pow(ny,2)*(6*x1*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x21*(15*(2*y21 + y31)*pow(y1,2) + 20*pow(y1,3) + 3*y31*pow(y21,2) + 4*pow(y21,3) + 2*y21*pow(y31,2) + 6*y1*(2*y21*y31 + 3*pow(y21,2) + pow(y31,2)) + pow(y31,3)) + x31*(15*(y21 + 2*y31)*pow(y1,2) + 20*pow(y1,3) + 2*y31*pow(y21,2) + pow(y21,3) + 3*y21*pow(y31,2) + 6*y1*(2*y21*y31 + pow(y21,2) + 3*pow(y31,2)) + 4*pow(y31,3)))))/360.);
            }
            else if (best_axis == 1) //Division by ny.
            {
                double Jacxz = fabs(x21*z31 - x31*z21);
                double coeffny = d*Jacxz/fabs(ny);
                Jxx +=  coeffny*(6.0*x1*x1 + x21*x21 + x21*x31 + x31*x31 + 4.0*x1*(x21 + x31))/12.0;
                Jyy +=  coeffny*(6*pow(d,2) + pow(nx,2)*(6*pow(x1,2) + pow(x21,2) + x21*x31 + pow(x31,2) + 4*x1*(x21 + x31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(nx*(3*x1 + x21 + x31) + nz*(3*z1 + z21 + z31)) + nx*nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)))/(12.*pow(ny,2));
                Jzz +=  coeffny*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                Jxxx += coeffny*((10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))/20.);
                Jyyy += coeffny*(-(pow(ny,-3)*(2*(nx*(3*x1 + x21 + x31) + nz*(3*z1 + z21 + z31))*pow(d,2) - 2*pow(d,3) - d*(nx*nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + pow(nx,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + pow(nz,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + (nz*pow(nx,2)*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + pow(nx,3)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3)) + nx*pow(nz,2)*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + pow(nz,3)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)))/5.))/4.);
                Jzzz += coeffny*((10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))/20.);
                Jxxy += coeffny*(-(pow(ny,-1)*(-5*d*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + nz*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 3*nx*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)))))/60.);
                Jxyy += coeffny*((pow(ny,-2)*(10*(3*x1 + x21 + x31)*pow(d,2) + 2*nx*nz*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 3*pow(nx,2)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) - 5*d*(nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 2*nx*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2))) + pow(nz,2)*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))))/60.);
                Jxxz += coeffny*((x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2))/60.);
                Jxzz += coeffny*((5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))/60.);
                Jyyz += coeffny*((pow(ny,-2)*(10*(3*z1 + z21 + z31)*pow(d,2) + pow(nx,2)*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) - 5*d*(nx*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 2*nz*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + 2*nx*nz*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + 3*pow(nz,2)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))))/60.);
                Jyzz += coeffny*((pow(ny,-1)*(5*d*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + nx*(-5*(4*x1 + 2*x21 + x31)*z1*z21 - (5*(4*x1 + x21 + 2*x31)*z1 + 5*x1*z21 + 2*(x21 + x31)*z21)*z31 - 10*(3*x1 + x21 + x31)*pow(z1,2) - (5*x1 + 3*x21 + x31)*pow(z21,2) - (5*x1 + x21 + 3*x31)*pow(z31,2)) - 3*nz*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))))/60.);
                Jxyz += coeffny*((pow(ny,-1)*(5*d*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) - 2*(nx*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + nz*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/120.);
                Jxxxx += coeffny*((20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 6*x1*(x21 + x31)*(pow(x21,2) + pow(x31,2)) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + pow(x31,4))/30.);
                Jyyyy += coeffny*(-(pow(ny,-4)*(20*(nx*(3*x1 + x21 + x31) + nz*(3*z1 + z21 + z31))*pow(d,3) - 15*pow(d,4) - nz*pow(nx,3)*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) - pow(nx,4)*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + 6*x1*(x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + pow(x31,3)) + pow(x31,4)) - 15*pow(d,2)*(nx*nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + pow(nx,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + pow(nz,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) - pow(nx,2)*pow(nz,2)*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) + 6*d*(nz*pow(nx,2)*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + pow(nx,3)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3)) + nx*pow(nz,2)*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + pow(nz,3)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))) - nx*pow(nz,3)*(6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3))) - pow(nz,4)*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + 6*z1*(z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + pow(z31,3)) + pow(z31,4))))/30.);
                Jzzzz += coeffny*((20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 6*z1*(z21 + z31)*(pow(z21,2) + pow(z31,2)) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + pow(z31,4))/30.);
                Jxxxy += coeffny*(-(pow(ny,-1)*(-6*d*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + (x21 + x31)*(pow(x21,2) + pow(x31,2)) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2))) + nz*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) + 4*nx*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 6*x1*(x21 + x31)*(pow(x21,2) + pow(x31,2)) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + pow(x31,4))))/120.);
                Jxyyy += coeffny*(-(pow(ny,-3)*(-20*(3*x1 + x21 + x31)*pow(d,3) + 15*pow(d,2)*(nz*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 2*nx*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2))) + 3*nz*pow(nx,2)*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) + 4*pow(nx,3)*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + 6*x1*(x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + pow(x31,3)) + pow(x31,4)) + 2*nx*pow(nz,2)*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 6*d*(2*nx*nz*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 3*pow(nx,2)*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3)) + pow(nz,2)*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) + pow(nz,3)*(6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))))/120.);
                Jxxxz += coeffny*((15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3))/120.);
                Jxzzz += coeffny*((6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2))) + x21*(3*z31*(4*z1*z21 + 5*pow(z1,2) + pow(z21,2)) + 2*(2*z1 + z21)*(5*z1*z21 + 5*pow(z1,2) + 2*pow(z21,2)) + 2*(3*z1 + z21)*pow(z31,2) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))/120.);
                Jyyyz += coeffny*(-(pow(ny,-3)*(-20*(3*z1 + z21 + z31)*pow(d,3) + pow(nx,3)*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) + 15*pow(d,2)*(nx*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 2*nz*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + 2*nz*pow(nx,2)*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 6*d*(pow(nx,2)*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 2*nx*nz*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + 3*pow(nz,2)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))) + 3*nx*pow(nz,2)*(6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3))) + 4*pow(nz,3)*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + 6*z1*(z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + pow(z31,3)) + pow(z31,4))))/120.);
                Jyzzz += coeffny*((pow(ny,-1)*(6*d*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2))) + nx*(-15*(4*x1 + 2*x21 + x31)*z21*pow(z1,2) - 20*(3*x1 + x21 + x31)*pow(z1,3) - 6*(5*x1 + 3*x21 + x31)*z1*pow(z21,2) - z31*(6*(5*x1 + 2*(x21 + x31))*z1*z21 + 15*(4*x1 + x21 + 2*x31)*pow(z1,2) + (6*x1 + 3*x21 + 2*x31)*pow(z21,2)) - (6*x1 + 4*x21 + x31)*pow(z21,3) - (6*(5*x1 + x21 + 3*x31)*z1 + (6*x1 + 2*x21 + 3*x31)*z21)*pow(z31,2) - (6*x1 + x21 + 4*x31)*pow(z31,3)) - 4*nz*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 6*z1*(z21 + z31)*(pow(z21,2) + pow(z31,2)) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + pow(z31,4))))/120.);
                Jxxyy += coeffny*((pow(ny,-2)*(15*pow(d,2)*(x21*x31 + 4*x1*(x21 + x31) + 6*pow(x1,2) + pow(x21,2) + pow(x31,2)) + 3*nx*nz*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) - 6*d*(nz*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + 3*nx*(10*(x21 + x31)*pow(x1,2) + 10*pow(x1,3) + x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + 5*x1*(x21*x31 + pow(x21,2) + pow(x31,2)) + pow(x31,3))) + 6*pow(nx,2)*(20*(x21 + x31)*pow(x1,3) + 15*pow(x1,4) + x31*pow(x21,3) + pow(x21,4) + pow(x21,2)*pow(x31,2) + 15*pow(x1,2)*(x21*x31 + pow(x21,2) + pow(x31,2)) + x21*pow(x31,3) + 6*x1*(x31*pow(x21,2) + pow(x21,3) + x21*pow(x31,2) + pow(x31,3)) + pow(x31,4)) + pow(nz,2)*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/180.);
                Jxxzz += coeffny*((15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))/180.);
                Jyyzz += coeffny*((pow(ny,-2)*(15*pow(d,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(nx,2)*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 6*d*(nx*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + 3*nz*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))) + 3*nx*nz*(6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3))) + 6*pow(nz,2)*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + 6*z1*(z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + pow(z31,3)) + pow(z31,4))))/180.);
                Jxxyz += coeffny*((pow(ny,-1)*(6*d*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) - 3*nx*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) - 2*nz*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/360.);
                Jxyyz += coeffny*(-(pow(ny,-2)*(-15*(4*x1*(3*z1 + z21 + z31) + x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(d,2) - 3*pow(nx,2)*(15*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31))*pow(x1,2) + 20*(3*z1 + z21 + z31)*pow(x1,3) + x31*(6*z1 + 3*z21 + 2*z31)*pow(x21,2) + (6*z1 + 4*z21 + z31)*pow(x21,3) + x21*(6*z1 + 2*z21 + 3*z31)*pow(x31,2) + 6*x1*(x21*x31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + (6*z1 + z21 + 4*z31)*pow(x31,3)) - 4*nx*nz*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) + 12*d*(nx*(x21*x31*(5*z1 + 2*(z21 + z31)) + 5*x1*(x21*(4*z1 + 2*z21 + z31) + x31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(x1,2) + (5*z1 + 3*z21 + z31)*pow(x21,2) + (5*z1 + z21 + 3*z31)*pow(x31,2)) + nz*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 3*pow(nz,2)*(6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))))/360.);
                Jxyzz += coeffny*((pow(ny,-1)*(6*d*(5*x1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) - 2*nx*(15*pow(x1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(x21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + x21*x31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(x31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*x1*(x21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + x31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 3*nz*(6*x1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + x31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))))/360.);
            }
            else //Division by nx.
            {
                double Jacyz = fabs(y21*z31 - y31*z21);
                double coeffnx = d*Jacyz/fabs(nx);
                Jxx +=  coeffnx*(6*pow(d,2) + pow(ny,2)*(6*pow(y1,2) + pow(y21,2) + y21*y31 + pow(y31,2) + 4*y1*(y21 + y31)) + pow(nz,2)*(6*pow(z1,2) + pow(z21,2) + z21*z31 + pow(z31,2) + 4*z1*(z21 + z31)) - 4*d*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31)) + ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)))/(12.*pow(nx,2));
                Jyy +=  coeffnx*(6.0*y1*y1 + y21*y21 + y21*y31 + y31*y31 + 4.0*y1*(y21 + y31))/12.0;
                Jzz +=  coeffnx*(6.0*z1*z1 + z21*z21 + z21*z31 + z31*z31 + 4.0*z1*(z21 + z31))/12.0;
                Jxxx += coeffnx*(-(pow(nx,-3)*(2*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31))*pow(d,2) - 2*pow(d,3) - d*(ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + pow(ny,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(nz,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + (nz*pow(ny,2)*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + pow(ny,3)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + ny*pow(nz,2)*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + pow(nz,3)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)))/5.))/4.);
                Jyyy += coeffnx*((10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))/20.);
                Jzzz += coeffnx*((10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))/20.);
                Jxxy += coeffnx*((pow(nx,-2)*(10*(3*y1 + y21 + y31)*pow(d,2) + 2*ny*nz*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*pow(ny,2)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) - 5*d*(nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 2*ny*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + pow(nz,2)*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))))/60.);
                Jxyy += coeffnx*(-(pow(nx,-1)*(-5*d*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + nz*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*ny*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)))))/60.);
                Jxxz += coeffnx*((pow(nx,-2)*(10*(3*z1 + z21 + z31)*pow(d,2) + pow(ny,2)*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*pow(nz,2)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2))) - 5*d*(ny*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 2*nz*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + 2*ny*nz*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))))/60.);
                Jxzz += coeffnx*((pow(nx,-1)*(5*d*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + ny*(-5*(4*y1 + 2*y21 + y31)*z1*z21 - (5*(4*y1 + y21 + 2*y31)*z1 + 5*y1*z21 + 2*(y21 + y31)*z21)*z31 - 10*(3*y1 + y21 + y31)*pow(z1,2) - (5*y1 + 3*y21 + y31)*pow(z21,2) - (5*y1 + y21 + 3*y31)*pow(z31,2)) - 3*nz*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)))))/60.);
                Jyyz += coeffnx*((y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2))/60.);
                Jyzz += coeffnx*((5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))/60.);
                Jxyz += coeffnx*((pow(nx,-1)*(5*d*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) - 2*(ny*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + nz*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/120.);
                Jxxxx += coeffnx*(-(pow(nx,-4)*(20*(ny*(3*y1 + y21 + y31) + nz*(3*z1 + z21 + z31))*pow(d,3) - 15*pow(d,4) - nz*pow(ny,3)*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) - pow(ny,4)*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + 6*y1*(y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + pow(y31,3)) + pow(y31,4)) - 15*pow(d,2)*(ny*nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + pow(ny,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + pow(nz,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) - pow(ny,2)*pow(nz,2)*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) + 6*d*(nz*pow(ny,2)*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + pow(ny,3)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + ny*pow(nz,2)*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + pow(nz,3)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))) - ny*pow(nz,3)*(6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3))) - pow(nz,4)*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + 6*z1*(z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + pow(z31,3)) + pow(z31,4))))/30.);
                Jyyyy += coeffnx*((20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 6*y1*(y21 + y31)*(pow(y21,2) + pow(y31,2)) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + pow(y31,4))/30.);
                Jzzzz += coeffnx*((20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 6*z1*(z21 + z31)*(pow(z21,2) + pow(z31,2)) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + pow(z31,4))/30.);
                Jxxxy += coeffnx*(-(pow(nx,-3)*(-20*(3*y1 + y21 + y31)*pow(d,3) + 15*pow(d,2)*(nz*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 2*ny*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2))) + 3*nz*pow(ny,2)*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) + 4*pow(ny,3)*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + 6*y1*(y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + pow(y31,3)) + pow(y31,4)) + 2*ny*pow(nz,2)*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 6*d*(2*ny*nz*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*pow(ny,2)*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3)) + pow(nz,2)*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) + pow(nz,3)*(6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))))/120.);
                Jxyyy += coeffnx*(-(pow(nx,-1)*(-6*d*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + (y21 + y31)*(pow(y21,2) + pow(y31,2)) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2))) + nz*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) + 4*ny*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 6*y1*(y21 + y31)*(pow(y21,2) + pow(y31,2)) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + pow(y31,4))))/120.);
                Jxxxz += coeffnx*(-(pow(nx,-3)*(-20*(3*z1 + z21 + z31)*pow(d,3) + pow(ny,3)*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) + 15*pow(d,2)*(ny*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 2*nz*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2))) + 2*nz*pow(ny,2)*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 6*d*(pow(ny,2)*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 2*ny*nz*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + 3*pow(nz,2)*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))) + 3*ny*pow(nz,2)*(6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3))) + 4*pow(nz,3)*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + 6*z1*(z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + pow(z31,3)) + pow(z31,4))))/120.);
                Jxzzz += coeffnx*((pow(nx,-1)*(6*d*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2))) + ny*(-15*(4*y1 + 2*y21 + y31)*z21*pow(z1,2) - 20*(3*y1 + y21 + y31)*pow(z1,3) - 6*(5*y1 + 3*y21 + y31)*z1*pow(z21,2) - z31*(6*(5*y1 + 2*(y21 + y31))*z1*z21 + 15*(4*y1 + y21 + 2*y31)*pow(z1,2) + (6*y1 + 3*y21 + 2*y31)*pow(z21,2)) - (6*y1 + 4*y21 + y31)*pow(z21,3) - (6*(5*y1 + y21 + 3*y31)*z1 + (6*y1 + 2*y21 + 3*y31)*z21)*pow(z31,2) - (6*y1 + y21 + 4*y31)*pow(z31,3)) - 4*nz*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 6*z1*(z21 + z31)*(pow(z21,2) + pow(z31,2)) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + pow(z31,4))))/120.);
                Jyyyz += coeffnx*((15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3))/120.);
                Jyzzz += coeffnx*((6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + (z21 + z31)*(pow(z21,2) + pow(z31,2)) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2))) + y21*(3*z31*(4*z1*z21 + 5*pow(z1,2) + pow(z21,2)) + 2*(2*z1 + z21)*(5*z1*z21 + 5*pow(z1,2) + 2*pow(z21,2)) + 2*(3*z1 + z21)*pow(z31,2) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))/120.);
                Jxxyy += coeffnx*((pow(nx,-2)*(15*pow(d,2)*(y21*y31 + 4*y1*(y21 + y31) + 6*pow(y1,2) + pow(y21,2) + pow(y31,2)) + 3*ny*nz*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) - 6*d*(nz*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + 3*ny*(10*(y21 + y31)*pow(y1,2) + 10*pow(y1,3) + y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + 5*y1*(y21*y31 + pow(y21,2) + pow(y31,2)) + pow(y31,3))) + 6*pow(ny,2)*(20*(y21 + y31)*pow(y1,3) + 15*pow(y1,4) + y31*pow(y21,3) + pow(y21,4) + pow(y21,2)*pow(y31,2) + 15*pow(y1,2)*(y21*y31 + pow(y21,2) + pow(y31,2)) + y21*pow(y31,3) + 6*y1*(y31*pow(y21,2) + pow(y21,3) + y21*pow(y31,2) + pow(y31,3)) + pow(y31,4)) + pow(nz,2)*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/180.);
                Jxxzz += coeffnx*((pow(nx,-2)*(15*pow(d,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(ny,2)*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 6*d*(ny*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) + 3*nz*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3))) + 3*ny*nz*(6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3))) + 6*pow(nz,2)*(20*(z21 + z31)*pow(z1,3) + 15*pow(z1,4) + z31*pow(z21,3) + pow(z21,4) + pow(z21,2)*pow(z31,2) + 15*pow(z1,2)*(z21*z31 + pow(z21,2) + pow(z31,2)) + z21*pow(z31,3) + 6*z1*(z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + pow(z31,3)) + pow(z31,4))))/180.);
                Jyyzz += coeffnx*((15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))/180.);
                Jxxyz += coeffnx*(-(pow(nx,-2)*(-15*(4*y1*(3*z1 + z21 + z31) + y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(d,2) - 3*pow(ny,2)*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) - 4*ny*nz*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) + 12*d*(ny*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + nz*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 3*pow(nz,2)*(6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))))/360.);
                Jxyyz += coeffnx*((pow(nx,-1)*(6*d*(y21*y31*(5*z1 + 2*(z21 + z31)) + 5*y1*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31)) + 10*(3*z1 + z21 + z31)*pow(y1,2) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) - 3*ny*(15*(y21*(4*z1 + 2*z21 + z31) + y31*(4*z1 + z21 + 2*z31))*pow(y1,2) + 20*(3*z1 + z21 + z31)*pow(y1,3) + y31*(6*z1 + 3*z21 + 2*z31)*pow(y21,2) + (6*z1 + 4*z21 + z31)*pow(y21,3) + y21*(6*z1 + 2*z21 + 3*z31)*pow(y31,2) + 6*y1*(y21*y31*(5*z1 + 2*(z21 + z31)) + (5*z1 + 3*z21 + z31)*pow(y21,2) + (5*z1 + z21 + 3*z31)*pow(y31,2)) + (6*z1 + z21 + 4*z31)*pow(y31,3)) - 2*nz*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))))))/360.);
                Jxyzz += coeffnx*((pow(nx,-1)*(6*d*(5*y1*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2))) - 2*ny*(15*pow(y1,2)*(z21*z31 + 4*z1*(z21 + z31) + 6*pow(z1,2) + pow(z21,2) + pow(z31,2)) + pow(y21,2)*(3*z21*z31 + 6*z1*(3*z21 + z31) + 15*pow(z1,2) + 6*pow(z21,2) + pow(z31,2)) + y21*y31*(4*z21*z31 + 12*z1*(z21 + z31) + 15*pow(z1,2) + 3*pow(z21,2) + 3*pow(z31,2)) + pow(y31,2)*(3*z21*z31 + 6*z1*(z21 + 3*z31) + 15*pow(z1,2) + pow(z21,2) + 6*pow(z31,2)) + 6*y1*(y21*(2*z21*z31 + 5*z1*(2*z21 + z31) + 10*pow(z1,2) + 3*pow(z21,2) + pow(z31,2)) + y31*(2*z21*z31 + 5*z1*(z21 + 2*z31) + 10*pow(z1,2) + pow(z21,2) + 3*pow(z31,2)))) - 3*nz*(6*y1*(10*(z21 + z31)*pow(z1,2) + 10*pow(z1,3) + z31*pow(z21,2) + pow(z21,3) + z21*pow(z31,2) + 5*z1*(z21*z31 + pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y21*(15*(2*z21 + z31)*pow(z1,2) + 20*pow(z1,3) + 3*z31*pow(z21,2) + 4*pow(z21,3) + 2*z21*pow(z31,2) + 6*z1*(2*z21*z31 + 3*pow(z21,2) + pow(z31,2)) + pow(z31,3)) + y31*(15*(z21 + 2*z31)*pow(z1,2) + 20*pow(z1,3) + 2*z31*pow(z21,2) + pow(z21,3) + 3*z21*pow(z31,2) + 6*z1*(2*z21*z31 + pow(z21,2) + 3*pow(z31,2)) + 4*pow(z31,3)))))/360.);
            }
        }

        const int ord = 4;
        dtens J(ord+1, dmat(ord+1, dvec(ord+1, 0.0) ) ); //Initialize the tensor with zeros.
        
        double ord2_coeff = M/(5.0*get_vol());
        J[2][0][0] = Jxx*ord2_coeff;
        J[0][2][0] = Jyy*ord2_coeff;
        J[0][0][2] = Jzz*ord2_coeff;

        double ord3_coeff = M/(6.0*get_vol());
        J[3][0][0] = Jxxx*ord3_coeff;
        J[0][3][0] = Jyyy*ord3_coeff;
        J[0][0][3] = Jzzz*ord3_coeff;
        J[2][1][0] = Jxxy*ord3_coeff;
        J[1][2][0] = Jxyy*ord3_coeff;
        J[2][0][1] = Jxxz*ord3_coeff;
        J[1][0][2] = Jxzz*ord3_coeff;
        J[0][2][1] = Jyyz*ord3_coeff;
        J[0][1][2] = Jyzz*ord3_coeff;
        J[1][1][1] = Jxyz*ord3_coeff;

        double ord4_coeff = M/(7.0*get_vol());
        J[4][0][0] = Jxxxx*ord4_coeff;
        J[0][4][0] = Jyyyy*ord4_coeff;
        J[0][0][4] = Jzzzz*ord4_coeff;
        J[3][1][0] = Jxxxy*ord4_coeff;
        J[1][3][0] = Jxyyy*ord4_coeff;
        J[3][0][1] = Jxxxz*ord4_coeff;
        J[1][0][3] = Jxzzz*ord4_coeff;
        J[0][3][1] = Jyyyz*ord4_coeff;
        J[0][1][3] = Jyzzz*ord4_coeff;
        J[2][2][0] = Jxxyy*ord4_coeff;
        J[2][0][2] = Jxxzz*ord4_coeff;
        J[0][2][2] = Jyyzz*ord4_coeff;
        J[2][1][1] = Jxxyz*ord4_coeff;
        J[1][2][1] = Jxyyz*ord4_coeff;
        J[1][1][2] = Jxyzz*ord4_coeff;

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
    void set_inertia_diagonal()
    {
        dmat3 I = get_inertia(1.0); //Since homogeneous mass density is assumed, the true total mass of the polyhedron, plays no role in the following diagonalization protocol. Hence pass whatever u want.

        //Convert our dmat3 datatype to Eigen's.
        Eigen::Matrix3d eigen_I;
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                eigen_I(row, col) = I[row][col];

        //Solve the eigensystem (3x3, real and symmetric matrix).
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(eigen_I);
        if (solver.info() != Eigen::Success) //This must never happen, due to the algebraic nature of the inertia matrix, but let's add a check...
        {
            fprintf(stderr, "[Warning] : In polyhedron::set_inertia_diagonal(), the inertia eigenvalue decomposition failed. Returning...\n");
            return;
        }
        Eigen::Vector3d eigenvalues = solver.eigenvalues(); //By default, Eigen sorts them in ascending order : eigenvalues[0] <= eigenvalues[1] <= eigenvalues[2]
        Eigen::Matrix3d eigenvectors = solver.eigenvectors();

        double Ixx = I[0][0], Iyy = I[1][1], Izz = I[2][2];
        //Reorder the eigenvalues appropriately.
        uvec3 indices; //Order by which the eigenvalues will be sorted (and thus order of the eigenvectors in the final rotation matrix).
        if (Ixx < Iyy && Iyy < Izz) //Case : Ixx < Iyy < Izz
            indices = uvec3{0,1,2}; //Default by Eigen.
        else if (Ixx < Izz && Izz < Iyy) //Case : Ixx < Izz < Iyy
            indices = uvec3{0,2,1};
        else if (Iyy < Ixx && Ixx < Izz) //Case : Iyy < Ixx < Izz
            indices = uvec3{1,0,2};
        else if (Iyy < Izz && Izz < Ixx) //Case : Iyy < Izz < Ixx
            indices = uvec3{2,0,1};
        else if (Izz < Iyy && Iyy < Ixx) //Case : Izz < Iyy < Ixx
            indices = uvec3{2,1,0};
        else if (Izz < Ixx && Ixx < Iyy) //Case : Izz < Ixx < Iyy
            indices = uvec3{1,2,0};
        else //This implies that some sort of equality was found between Ixx,Iyy,Izz, but this is not expected, as we are dealing with double precision.
            indices = uvec3{0,1,2}; //Stick to the default by Eigen. I will fix it later...

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

        //Now convert the Eigen variable 'eigen_rot_mat' back to our dmat3.
        dmat3 fin_rot_mat;
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                fin_rot_mat[row][col] = eigen_rot_mat(row, col);

        //Apply the rotation to all the vertices of the rigid body.
        for (size_t i = 0; i < verts.size(); ++i)
            verts[i] = dot(fin_rot_mat, verts[i]);

        //Since the vertices rotated, either we have to rotate the normals as well (with the same matrix), or just recompute them...
        norms_exist = false;
        gen_norms();
    }

    //Uniform scaling (overloaded).
    void set_scale(const double scale_factor)
    {
        for (dvec3 &v : verts)
        {
            v[0] *= scale_factor;
            v[1] *= scale_factor;
            v[2] *= scale_factor;
        }
    }

    //Non-uniform scaling (overloaded).
    void set_scale(const dvec3 &scale_factor)
    {
        const double sx = scale_factor[0];
        const double sy = scale_factor[1];
        const double sz = scale_factor[2];
        for (dvec3 &v : verts)
        {
            v[0] *= sx;
            v[1] *= sy;
            v[2] *= sz;
        }
    }
};

#endif
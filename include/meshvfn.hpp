#ifndef MESHVFN_HPP
#define MESHVFN_HPP

#include<GL/glew.h>

#include<cstdio>
#include<vector>

#include"typedef.hpp"
#include"obj.hpp"

class Meshvfn
{

private:

    unsigned int vao, vbo; //vertex array and buffer objects
    std::vector<float> buffer; //final form of the data to draw
    bool is_init;

public:

    Meshvfn() {
        is_init = false;
    }

    void update_mesh(const char *path)
    { 

        if(is_init){
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            buffer.clear();
        }
            
        std::ifstream file(path);
        if (!file.is_open())
        {
            printf("'%s' not found. Exiting...\n", path);
            exit(EXIT_FAILURE);
        }

        dmatnx3 verts;
        imatnx3 faces;

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
        dmatnx3 norms(faces.size());
        dvec3 perp;
        for (int i = 0; i < norms.size(); ++i)
        {
            dvec3 p0 = verts[faces[i][0]];
            dvec3 p1 = verts[faces[i][1]];
            dvec3 p2 = verts[faces[i][2]];
            perp = cross(p1-p0, p2-p1);
            norms[i] = perp/length(perp);
        }

        //Combine verts[][], norms[][] and faces[][] to construct the buffer[]
        //which will have all the buffer needed for drawing.
        for (int i = 0; i < faces.size(); ++i)
        {
            buffer.push_back( (float)verts[ faces[i][0] ][0] );
            buffer.push_back( (float)verts[ faces[i][0] ][1] );
            buffer.push_back( (float)verts[ faces[i][0] ][2] );
            buffer.push_back( (float)norms[i][0] );
            buffer.push_back( (float)norms[i][1] );
            buffer.push_back( (float)norms[i][2] );

            buffer.push_back( (float)verts[ faces[i][1] ][0] );
            buffer.push_back( (float)verts[ faces[i][1] ][1] );
            buffer.push_back( (float)verts[ faces[i][1] ][2] );
            buffer.push_back( (float)norms[i][0] );
            buffer.push_back( (float)norms[i][1] );
            buffer.push_back( (float)norms[i][2] );

            buffer.push_back( (float)verts[ faces[i][2] ][0] );
            buffer.push_back( (float)verts[ faces[i][2] ][1] );
            buffer.push_back( (float)verts[ faces[i][2] ][2] );
            buffer.push_back( (float)norms[i][0] );
            buffer.push_back( (float)norms[i][1] );
            buffer.push_back( (float)norms[i][2] );
        }

        //buffer[] has now the form : {x1,y1,z1, nx1,ny1,nz1, x2,y2,z2, nx2,ny2,nz2 ... }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, buffer.size()*sizeof(float), &buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        is_init = true;
    }

    //delete the mesh
    ~Meshvfn()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //draw the mesh (triangles)
    void draw()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (unsigned int)(buffer.size()/6));
        glBindVertexArray(0);
    }
};

#endif

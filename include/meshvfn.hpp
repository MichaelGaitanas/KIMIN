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

public:

    Meshvfn(Obj &obj)
    {
        //Combine verts[][], norms[][] and faces[][] to construct the buffer[]
        //which will have all the buffer needed for drawing.
        for (int i = 0; i < obj.faces.size(); ++i)
        {
            buffer.push_back( (float)obj.verts[ obj.faces[i][0] ][0] );
            buffer.push_back( (float)obj.verts[ obj.faces[i][0] ][1] );
            buffer.push_back( (float)obj.verts[ obj.faces[i][0] ][2] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][0] ][0] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][0] ][1] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][0] ][2] );

            buffer.push_back( (float)obj.verts[ obj.faces[i][1] ][0] );
            buffer.push_back( (float)obj.verts[ obj.faces[i][1] ][1] );
            buffer.push_back( (float)obj.verts[ obj.faces[i][1] ][2] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][1] ][0] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][1] ][1] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][1] ][2] );

            buffer.push_back( (float)obj.verts[ obj.faces[i][2] ][0] );
            buffer.push_back( (float)obj.verts[ obj.faces[i][2] ][1] );
            buffer.push_back( (float)obj.verts[ obj.faces[i][2] ][2] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][2] ][0] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][2] ][1] );
            buffer.push_back( (float)obj.norms[ obj.norms[i][2] ][2] );
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

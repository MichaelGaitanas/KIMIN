#ifndef ORBMESH_H
#define ORBMESH_H

#include<GL/glew.h>

#include"typedef.h"


class orbmesh
{
private:
    dmatnx3 verts;

    unsigned int vao, vbo

public:
    orbmesh() : verts(),
                vao(0),
                vbo(0)
    {

    }

    //Draw the orbital mesh.
    void draw()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, (GLsizei)gl_vertex_count);
        glBindVertexArray(0);
    }

};


#endif
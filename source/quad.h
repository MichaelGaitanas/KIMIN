/* Here lies the CPU logic of a simple 2D quad mesh prototype. But with appropriate shader manipulation, a 2D quad can become
   an 'infinite' reference grid or a virtual 3D Sun. See (grid.vert, grid.frag), (sun.vert, sun.frag) for the core operations. */

#ifndef QUAD_H
#define QUAD_H

#include<GL/glew.h>

class quad
{
private:
    unsigned vao, vbo;

public:
    quad() : vao(0),
             vbo(0)
    {
        float verts[] = { -1.0f, -1.0f, 
                           1.0f, -1.0f,
                           1.0f,  1.0f,

                          -1.0f, -1.0f,
                           1.0f,  1.0f,
                          -1.0f,  1.0f };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        //layout(location = 0) in vec2 pos;
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    ~quad()
    {
        if (vbo)
            glDeleteBuffers(1, &vbo);
        if (vao)
            glDeleteVertexArrays(1, &vao);
    }

    void render()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
};

#endif
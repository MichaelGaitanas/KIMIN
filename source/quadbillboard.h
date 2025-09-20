#ifndef QUAD_BILLBOARD_H
#define QUAD_BILLBOARD_H

#include <GL/glew.h>

class quadbillboard
{
private:
    unsigned int vao, vbo;

public:
    quadbillboard()
    {
        // Two CCW triangles covering [-1, +1]^2 in local space
        float verts[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,

            -1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // layout(location = 0) in vec2 in_pos;
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ~quadbillboard()
    {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
    }

    void draw() const
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
};

#endif // QUAD_BILLBOARD_H
/* This class is responsible for the Sun rendering logic. Our Sun is gonna be a 2D quad and we will manipulate it via the shader
   to make it look like a Sun sphere. See sun.vert and sun.frag shaders. */

#ifndef SUN_H
#define SUN_H

#include<GL/glew.h>

class sun
{
private:
    unsigned vao, vbo;

public:
    sun() : vao(0),
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

    ~sun()
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
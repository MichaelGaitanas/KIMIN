/* This class handles the CPU logic (memory assignment) of the 'infinite' grid. The actual grid operations happen in the shader grid.vert and grid.frag. */

#ifndef GRID_H
#define GRID_H

#include<GL/glew.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class grid
{
private:
    unsigned int vao, vbo;

public:
    float cell, px, fade_start, fade_end;
    glm::vec3 color;

    grid() : vao(0),
             vbo(0),
             cell(1.0f),
             px(1.0f),
             fade_start(0.0f),
             fade_end(0.0f),
             color(glm::vec3(0.3f))
    {
        //Fullscreen quad in clip space (xy in [-1,1])
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

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    ~grid()
    {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
    }

    void render()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); //Just a quad in reality.
        glBindVertexArray(0);
    }
};

#endif
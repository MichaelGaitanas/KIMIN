#ifndef ORBMESH_H
#define ORBMESH_H

#include<GL/glew.h>

#include<vector>

#include"typedef.h"

class orbmesh
{
private:
    bool gl_ready;
    unsigned int vao, vbo;

public:
    size_t draw_count;
    float thickness; //This is basically the rasterized line width of the trajectory.

    orbmesh() : gl_ready(false),
                vao(0),
                vbo(0),
                draw_count(0),
                thickness(0.2f)
    { }

    void clear()
    {
        if (vbo)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (vao)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        draw_count = 0;
        gl_ready = false;
    }

    void set_as_gl_mesh(const dvec &x, const dvec &y, const dvec &z, const float coeff)
    {
        if (gl_ready) return; //If the orbital data [x(t),y(t),z(t)] have already been uploaded to the GPU, just exit the function.

        //Construct the orbital mesh.
        std::vector<float> interleaved_buffer; //Meant to contain {(x1,y1,z1), (z2,y2,z2), ..., (xn,yn,zn)}, meant to be connected via GL_LINE_STRIP.
        const size_t N = x.size();
        interleaved_buffer.resize(3*N); //3 vertices per face, 6 floats each.

        size_t j = 0;
        for (size_t i = 0; i < N; ++i)
        {
            interleaved_buffer[j++] = coeff*static_cast<float>(x[i]);
            interleaved_buffer[j++] = coeff*static_cast<float>(y[i]);
            interleaved_buffer[j++] = coeff*static_cast<float>(z[i]);
        }
        draw_count = N;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved_buffer.size()*sizeof(float), interleaved_buffer.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        gl_ready = true;
    }

    //Draw the orbital mesh in the form of line strip.
    void render()
    {
        if (!gl_ready) return; //Guard.

        glBindVertexArray(vao);
        glLineWidth(thickness);
        glDrawArrays(GL_LINE_STRIP, 0, draw_count);
        glBindVertexArray(0);
    }
};


#endif
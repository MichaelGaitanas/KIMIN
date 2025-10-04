/* This class handles the logic of the orbital meshes (3D curves) that are displayed. */

#ifndef ORBIT_H
#define ORBIT_H

#include<GL/glew.h>

#include<vector>

#include"typedef.h"

class orbit
{
private:
    bool gl_ready;
    unsigned int vao, vbo;
    float thickness; //This is basically the rasterized line width of the trajectory.

public:
    size_t draw_count;

    orbit() : gl_ready(false),
              vao(0),
              vbo(0),
              thickness(0.5f),
              draw_count(0)
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

    void set_as_gl_mesh(const dvec &x, const dvec &y, const dvec &z, const float coeff = 1.0f)
    {
        if (gl_ready) return; //If the orbital data [x(t),y(t),z(t)] have already been uploaded to the GPU, just exit the function.

        //Construct the orbital mesh in the CPU.
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

        //Now send it to the GPU :

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved_buffer.size()*sizeof(float), interleaved_buffer.data(), GL_STATIC_DRAW);

        //layout(location = 0) in vec3 pos;
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
/* This class handles the CPU logic of the orbital meshes (3D curves) that are displayed. */

#ifndef ORBIT_H
#define ORBIT_H

#include<vector>

#include<GL/glew.h>

#include"typedef.h"

class orbit
{
private:
    bool gl_ready;
    unsigned vao, vbo;
    float thickness; //This is basically the rasterized line width of the trajectory.
    size_t gl_vertex_count; //Total vertices uploaded in the gpu.

public:
    size_t gl_draw_count; //GUI-controlled slider regarding how many vertices to draw (see scene.h).

    orbit() : gl_ready(false),
              vao(0),
              vbo(0),
              thickness(0.5f),
              gl_vertex_count(0),
              gl_draw_count(0)
    { }

    void clear_gl_mesh()
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
        gl_vertex_count = 0;
        gl_ready = false;
    }

    void set_gl_mesh(const dvec &x, const dvec &y, const dvec &z, const float coeff = 1.0f)
    {
        if (gl_ready) return; //If the orbital data [x(t),y(t),z(t)] have already been uploaded to the GPU, just exit the function.

        const size_t N = x.size();
        gl_vertex_count = N;
        if (gl_draw_count > N)
            gl_draw_count = N;

        std::vector<float> interleaved_buffer; //Meant to contain {(x1,y1,z1), (z2,y2,z2), ..., (xn,yn,zn)}, meant to be connected via GL_LINE_STRIP.
        interleaved_buffer.resize(3*N); //3 vertices per face, 6 floats each.

        size_t j = 0;
        for (size_t i = 0; i < N; ++i)
        {
            interleaved_buffer[j++] = coeff*(float)x[i];
            interleaved_buffer[j++] = coeff*(float)y[i];
            interleaved_buffer[j++] = coeff*(float)z[i];
        }

        //Now send it to the GPU :

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved_buffer.size()*sizeof(float), interleaved_buffer.data(), GL_STATIC_DRAW);

        //layout(location = 0) in vec3 pos;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        gl_ready = true;
    }

    //Draw the orbital mesh in the form of line strip.
    void render()
    {
        if (!gl_ready) return; //Guard.

        //Extra safety: clamp at render time too
        const size_t count = (gl_draw_count > gl_vertex_count) ? gl_vertex_count : gl_draw_count;

        glBindVertexArray(vao);
        glLineWidth(thickness);
        glDrawArrays(GL_LINE_STRIP, 0, count);
        glBindVertexArray(0);
    }
};


#endif
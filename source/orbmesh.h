#ifndef ORBMESH_H
#define ORBMESH_H

#include<GL/glew.h>

#include<vector>

#include"solution.h"

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

    void set_as_gl_mesh(const solution &sol, const float com_coeff)
    {
        //If the orbital data [x(t),y(t),z(t)] have already been uploaded to the GPU, exit the function.
        if (gl_ready) return;

        //Construct the 2 orbital meshes.
        std::vector<float> interleaved_buffer; //Meant to contain {(x1,y1,z1), (z2,y2,z2), ..., (xn,yn,zn)}, meant to be connected via GL_LINE_STRIP.
        interleaved_buffer.resize(3*sol.t.size()); //3 vertices per face, 6 floats each.

        size_t j = 0;
        for (size_t i = 0; i < sol.t.size(); ++i)
        {
            interleaved_buffer[j++] = com_coeff*(float)sol.x[i];
            interleaved_buffer[j++] = com_coeff*(float)sol.y[i];
            interleaved_buffer[j++] = com_coeff*(float)sol.z[i];
        }
        draw_count = interleaved_buffer.size()/3; //Because each vertex has 3 float attributes bound.

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
        glBindVertexArray(vao);
        glLineWidth(thickness);
        glDrawArrays(GL_LINE_STRIP, 0, draw_count);
        glBindVertexArray(0);
    }
};


#endif
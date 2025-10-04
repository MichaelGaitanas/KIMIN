#ifndef SUN_H
#define SUN_H

#include<GL/glew.h>

class sun
{
private:
    unsigned int vao, vbo;

public:
    float ang_deg, disc_intensity, disc_edge_soft, limb_strength, limb_power;

    sun() : vao(0),
            vbo(0),
            ang_deg(6.0f),
            disc_intensity(10.0f),
            disc_edge_soft(3.0f),
            limb_strength(0.0f),
            limb_power(0.0f)
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

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ~sun()
    {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
    }

    void render()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); //Just a quad. But with appropriate fragment shader manipulation, it will look like an emissive disk.
        glBindVertexArray(0);
    }
};

#endif
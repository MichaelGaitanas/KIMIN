#ifndef LOGO_HPP
#define LOGO_HPP

#include<GL/glew.h>

#include<cstdio>

#include"shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

class Logo
{
private:
    unsigned int vao, vbo, tex;
    shader shad_logo;

public:
    Logo(const char *img_path) :
        shad_logo("../shaders/vertex/texture.vert", "../shaders/fragment/texture.frag")
    {
        //Procedural quad.
        float interleaved_buffer[] = {  //Positions.         //UVs.
                                        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
                                        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
                                         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
                                         
                                        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
                                         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
                                         0.5f,  0.5f, 0.0f,  1.0f, 1.0f };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(interleaved_buffer), &interleaved_buffer, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0); //Positions.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float))); //UVs.
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        //Tell OpenGL how to apply the texture on the mesh.
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures.

        //Load the image texture.
        int img_width, img_height, img_channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *img_data = stbi_load(img_path, &img_width, &img_height, &img_channels, 0);
        if (!img_data)
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", img_path);
            exit(EXIT_FAILURE);
        }

        //Determine the correct format based on the number of channels (img_channels).
        GLenum format;
        if (img_channels == 1)
            format = GL_RED; //Single-channel (grayscale image).
        else if (img_channels == 3)
            format = GL_RGB; //Classical 3-channel image (e.g. jpg).
        else if (img_channels == 4)
            format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

        glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, img_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(img_data); //Free image resources.

        //shad_logo = shader("../shaders/vertex/texture.vert", "../shaders/fragment/texture.frag");
        shad_logo.use();
    }

    //Delete the logo resources.
    ~Logo()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tex);
    }

    //Draw the logo mesh.
    void draw_triangles()
    {
        shad_logo.use(); //Ensure logo shader is active.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif
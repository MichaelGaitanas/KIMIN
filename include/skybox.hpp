#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include<GL/glew.h>

#include<cstdio>
#include<string>

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

class Skybox
{

private:

    unsigned int vao; //vertex array object
    unsigned int vbo; //vertex buffer object
    unsigned int ebo; //element buffer object
    unsigned int texo; //texture object

public:

    //Default constructor.
    Skybox()
    {
        //unit cube
        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };

        //triangles
        unsigned int inds[] = { //right
                                1, 2, 6,
                                6, 5, 1,
                                //left
                                0, 4, 7,
                                7, 3, 0,
                                //top
                                4, 5, 6,
                                6, 7, 4,
                                //bottom
                                0, 3, 2,
                                2, 1, 0,
                                //back
                                0, 1, 5,
                                5, 4, 0,
                                //front
                                3, 7, 6,
                                6, 2, 3  };

        //Gpu info
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), &inds, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        //Skybox images (make sure they are in this exact order)
        std::string path[6] = { "../skybox/starfield_4k/right.jpg",
                                "../skybox/starfield_4k/left.jpg",
                                "../skybox/starfield_4k/top.jpg",
                                "../skybox/starfield_4k/bottom.jpg",
                                "../skybox/starfield_4k/front.jpg",
                                "../skybox/starfield_4k/back.jpg" };
        //Skybox images (make sure they are in this exact order)
        //std::string path[6] = { "../skybox/galaxy/right.png",
        //                        "../skybox/galaxy/left.png",
        //                        "../skybox/galaxy/top.png",
        //                        "../skybox/galaxy/bottom.png",
        //                        "../skybox/galaxy/front.png",
        //                        "../skybox/galaxy/back.png" };
        //Why not const char *path[6] = {"...", "...", ... }; ? And then remove the .c_str() from stbi_load()

        //Create the skybox texture object
        unsigned int texo;
        glGenTextures(1, &texo);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texo);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); //Don't use it for now...

        //Loop through all the images and attach them to the texture object (texo).
        for (int i = 0; i < 6; i++)
        {
            int width, height, nchannels;
            unsigned char *data = stbi_load(path[i].c_str(), &width, &height, &nchannels, 0);
            if (data)
            {
                stbi_set_flip_vertically_on_load(false);
                glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               0,
                               GL_RGB,
                               width,
                               height,
                               0,
                               GL_RGB,
                               GL_UNSIGNED_BYTE,
                               data );
                stbi_image_free(data);
            }
            else
            {
                printf("Failed to load texture '%s'\n", path[i].c_str());
                stbi_image_free(data);
            }
        }
    }

    //Delete the skybox.
    ~Skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the skybox (elements).
    void draw()
    {
        glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texo);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
    }
};

#endif

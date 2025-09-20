#ifndef SKYBOX_H
#define SKYBOX_H

#include<GL/glew.h>

#include<string>

class skybox
{
private:
    unsigned int vao, vbo, ebo, tex; //Vertex array object, vertex buffer object, element (index) buffer object and texture ID.

public:
    //Construct the mesh procedurally (i.e. no geometry data like vertices or uvs are read from a file), setup the mesh in the gpu memory, load the 6 images and tell how to wrap them.
    //Note : Make sure that all 6 images have the same size in pixels (e.g. 2048x2048, 500x500, etc...) AND the same type of extensions (e.g. jpg, png, bmp, ...).
    skybox(const char *right_img_path, const char *left_img_path, const char *top_img_path, const char *bottom_img_path, const char *front_img_path, const char *back_img_path)
    {   
        //Cube vertices.
        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };

        //Cube indices.
        unsigned int inds[] = { //Right.
                                1, 2, 6,
                                6, 5, 1,
                                //Left.
                                0, 4, 7,
                                7, 3, 0,
                                //Top.
                                4, 5, 6,
                                6, 7, 4,
                                //Bottom.
                                0, 3, 2,
                                2, 1, 0,
                                //Front.
                                3, 7, 6,
                                6, 2, 3, 
                                //Back.
                                0, 1, 5,
                                5, 4, 0 };

        //Setup skybox's data in the memory.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), &inds, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        //Create the skybox's texture.
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures (e.g. grayscale).
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        //Skybox's expected image names. Do not change their order!
        std::string paths[6] = { right_img_path, left_img_path, top_img_path, bottom_img_path, front_img_path, back_img_path };
        int img_widths[6], img_heights[6], img_channels[6];

        stbi_set_flip_vertically_on_load(false);
        for (int i = 0; i < 6; i++)
        {
            unsigned char *data = stbi_load(paths[i].c_str(), &img_widths[i], &img_heights[i], &img_channels[i], 0);
            if (!data)
                fprintf(stderr, "[Warning] : In skybox::skybox(), failed to load texture '%s'\n", paths[i].c_str());

            //Determine the correct format for glTexImage2D based on the number of channels (img_channels).
            GLenum format;
            if (img_channels[i] == 1)
                format = GL_RED; //Single-channel grayscale image.
            else if (img_channels[i] == 3)
                format = GL_RGB; //Classical 3-channel image (e.g. jpg).
            else if (img_channels[i] == 4)
                format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, img_widths[i], img_heights[i], 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        //Check if all images have the same width, height, and channels. Otherwise the skybox may not render.
        bool img_consistency = true;
        for (int i = 1; i < 6; i++)
        {
            if (img_widths[i] != img_widths[0] || img_heights[i] != img_heights[0] || img_channels[i] != img_channels[0])
            {
                img_consistency = false;
                break;
            }
        }
        if (!img_consistency)
            fprintf(stderr, "[Warning] : In skybox::skybox(), all 6 images must have the same width, height, and channels.\n");
    }

    //Delete the skybox's resources.
    ~skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tex);
    }

    //Draw the skybox.
    void draw_triangles()
    {
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glBindVertexArray(vao);
        glDepthFunc(GL_LEQUAL); //Ensures that the skybox fragments will render behind everything else. (A bit dangerous to place it here. Be cautious.)
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS); //Restore the default depth test function for rendering the rest of the scene.
		glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};

#endif
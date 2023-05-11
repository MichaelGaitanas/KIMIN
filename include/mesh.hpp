#ifndef MESH_HPP
#define MESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

class meshvf
{
public:
    unsigned int vao, vbo; //vertex array and buffer object
    std::vector<float> main_buffer; //final form of the geometry data to draw
    unsigned int draw_size;

    meshvf(const char *objpath)
    {
        std::ifstream fp;
        fp.open(objpath);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", objpath);
            exit(EXIT_FAILURE);
        }

        //vertices (format : v x y z)
        std::vector<std::vector<float>> verts;
        float x,y,z;

        //indices (format : f i1 i2 i3)
        std::vector<std::vector<unsigned int>> inds;
        unsigned int i1,i2,i3;

        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'f') //then we have an indices line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d %d %d", &i1,&i2,&i3);
                inds.push_back({i1-1, i2-1, i3-1});
            }
        }

        //Now combine verts[][] and inds[][] to construct the main_buffer[]
        //which will have all the data needed for drawing.
        for (int i = 0; i < inds.size(); ++i)
        {
            main_buffer.push_back( verts[ inds[i][0] ][0] );
            main_buffer.push_back( verts[ inds[i][0] ][1] );
            main_buffer.push_back( verts[ inds[i][0] ][2] );

            main_buffer.push_back( verts[ inds[i][1] ][0] );
            main_buffer.push_back( verts[ inds[i][1] ][1] );
            main_buffer.push_back( verts[ inds[i][1] ][2] );

            main_buffer.push_back( verts[ inds[i][2] ][0] );
            main_buffer.push_back( verts[ inds[i][2] ][1] );
            main_buffer.push_back( verts[ inds[i][2] ][2] );
        }
        //Now main_buffer[] has the form : {x1,y1,z1, x2,y2,z2, x3,y3,z3, ... }, where
        //consecutive triads of vertices form triangles-faces of the object to be rendered

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        draw_size = (int)(main_buffer.size()/3); //to draw only faces
    }

    //delete the mesh
    ~meshvf()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //draw the mesh (triangles)
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, draw_size);
        glBindVertexArray(0);
    }
};

class meshvfn
{
public:
    unsigned int vao, vbo; //vertex array and buffer object
    std::vector<float> main_buffer; //final form of the geometry data to draw
    unsigned int draw_size;

    meshvfn(const char *objpath)
    {
        std::ifstream fp;
        fp.open(objpath);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", objpath);
            exit(EXIT_FAILURE);
        }

        //vertices (format : v x y z)
        std::vector<std::vector<float>> verts;
        float x,y,z;

        //normals (format : vn nx ny nz)
        std::vector<std::vector<float>> norms;
        float nx,ny,nz;

        //indices (format : f i11//i12 i21//i22 i31//i32)
        std::vector<std::vector<unsigned int>> inds;
        unsigned int i11,i12, i21,i22, i31,i32;

        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //then we have a vertex line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 'n') //then we have a normal line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "vn %f %f %f", &nx,&ny,&nz);
                norms.push_back({nx,ny,nz});
            }
            else if (line[0] == 'f') //then we have an indices line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d//%d %d//%d %d//%d", &i11,&i12, &i21,&i22, &i31,&i32);
                inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
            }
        }

        //Now combine verts[][], norms[][] and inds[][] to construct the main buffer main_buffer[]
        //which will have all the main_buffer needed for drawing.
        for (int i = 0; i < inds.size(); ++i)
        {
            main_buffer.push_back( verts[ inds[i][0] ][0] );
            main_buffer.push_back( verts[ inds[i][0] ][1] );
            main_buffer.push_back( verts[ inds[i][0] ][2] );
            main_buffer.push_back( norms[ inds[i][1] ][0] );
            main_buffer.push_back( norms[ inds[i][1] ][1] );
            main_buffer.push_back( norms[ inds[i][1] ][2] );

            main_buffer.push_back( verts[ inds[i][2] ][0] );
            main_buffer.push_back( verts[ inds[i][2] ][1] );
            main_buffer.push_back( verts[ inds[i][2] ][2] );
            main_buffer.push_back( norms[ inds[i][3] ][0] );
            main_buffer.push_back( norms[ inds[i][3] ][1] );
            main_buffer.push_back( norms[ inds[i][3] ][2] );

            main_buffer.push_back( verts[ inds[i][4] ][0] );
            main_buffer.push_back( verts[ inds[i][4] ][1] );
            main_buffer.push_back( verts[ inds[i][4] ][2] );
            main_buffer.push_back( norms[ inds[i][5] ][0] );
            main_buffer.push_back( norms[ inds[i][5] ][1] );
            main_buffer.push_back( norms[ inds[i][5] ][2] );
        }
        //main_buffer[] has now the form : {x1,y1,z1, nx1,ny1,nz1, x2,y2,z2, nx2,ny2,nz2 ... }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        draw_size = (int)(main_buffer.size()/6); //to draw faces and normals
    }

    //delete the mesh
    ~meshvfn()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //draw the mesh (triangles)
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, draw_size);
        glBindVertexArray(0);
    }
};

class skybox
{

public:

    unsigned int vao, vbo, ebo, tao;
    std::vector<float> main_buffer;
    unsigned int draw_size;

    skybox()
    {
        //vertices are hard coded because they are always the same
        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };

        //indices as well
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

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), &inds, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


        //All the faces of the cubemap (make sure they are in this exact order)
        std::string facesCubemap[6] =
        {
            "../skybox/starfield_4k/right.jpg",
            "../skybox/starfield_4k/left.jpg",
            "../skybox/starfield_4k/top.jpg",
            "../skybox/starfield_4k/bottom.jpg",
            "../skybox/starfield_4k/front.jpg",
            "../skybox/starfield_4k/back.jpg"
        };

        // Creates the cubemap texture object
        unsigned int tao;
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // These are very important to prevent seams
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        // This might help with seams on some systems
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // Cycles through all the textures and attaches them to the cubemap object
        for (unsigned int i = 0; i < 6; i++)
        {
            int width, height, nrChannels;
            unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                stbi_set_flip_vertically_on_load(false);
                glTexImage2D
                (
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGB,
                    width,
                    height,
                    0,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
                stbi_image_free(data);
            }
        }
    }

    //delete the skybox
    ~skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    //draw the skybox
    void draw()
    {
        glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
    }
};

#endif

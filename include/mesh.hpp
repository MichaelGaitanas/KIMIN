#ifndef MESH_HPP
#define MESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#define STB_IMAGE_IMPLEMENTATION
#include"../include/stb_image.h"

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

class meshvft
{
public:
    unsigned int vao, vbo, tao; //vertex array, buffer and texture object
    std::vector<float> main_buffer; //final form of the geometry data to draw
    unsigned int draw_size;

    meshvft(const char *objpath, const char *imgpath)
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

        //texture (format : vt tx ty)
        std::vector<std::vector<float>> texs;
        float tx,ty;

        //indices (format : f i11/i12 i21/i22 i31/i32
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
            else if (line[0] == 'v' && line[1] == 't') //then we have a texture line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "vt %f %f", &tx,&ty);
                texs.push_back({tx,ty});
            }
            else if (line[0] == 'f') //then we have a indices line
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d/%d %d/%d %d/%d", &i11,&i12, &i21,&i22, &i31,&i32);
                inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
            }
        }

        //Now combine verts[][], texs[][] and inds[][] to construct the main buffer main_buffer[]
        //which will have all the main_buffer needed for drawing.
        for (int i = 0; i < inds.size(); ++i)
        {
            main_buffer.push_back( verts[ inds[i][0] ][0] );
            main_buffer.push_back( verts[ inds[i][0] ][1] );
            main_buffer.push_back( verts[ inds[i][0] ][2] );
            main_buffer.push_back(  texs[ inds[i][1] ][0] );
            main_buffer.push_back(  texs[ inds[i][1] ][1] );

            main_buffer.push_back( verts[ inds[i][2] ][0] );
            main_buffer.push_back( verts[ inds[i][2] ][1] );
            main_buffer.push_back( verts[ inds[i][2] ][2] );
            main_buffer.push_back(  texs[ inds[i][3] ][0] );
            main_buffer.push_back(  texs[ inds[i][3] ][1] );

            main_buffer.push_back( verts[ inds[i][4] ][0] );
            main_buffer.push_back( verts[ inds[i][4] ][1] );
            main_buffer.push_back( verts[ inds[i][4] ][2] );
            main_buffer.push_back(  texs[ inds[i][5] ][0] );
            main_buffer.push_back(  texs[ inds[i][5] ][1] );
        }
        //main_buffer[] has now the form : {x1,y1,z1, tx1,ty1, x2,y2,z2, tx2,ty2 ... }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        draw_size = (int)(main_buffer.size()/5); //to draw faces and textures

        //load the image-texture
        int imgwidth, imgheight, imgchannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *imgdata = stbi_load(imgpath, &imgwidth, &imgheight, &imgchannels, 0);
        if (!imgdata)
        {
            printf("'%s' not found. Exiting...\n", imgpath);
            exit(EXIT_FAILURE);
        }

        //tell OpenGL how to apply the texture on the mesh
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_2D, tao);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgwidth, imgheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(imgdata); //free image resources
    }

    //delete the mesh
    ~meshvft()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //draw the mesh (triangles)
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, tao);
        glDrawArrays(GL_TRIANGLES, 0, draw_size);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

};

#endif

#ifndef SHADER_H
#define SHADER_H

#include<GL/glew.h>
#include<glm/glm.hpp>
#include<iostream>
#include<cstdio>
#include<fstream>

#include"typedef.h"

class shader
{
private:
    unsigned int ID; //Shader program ID. With this, we recognize which shader to use.

public:
    //Parse and read the vertex and fragment shader source files. Then compile both. Then link.
    shader(const char *vpath, const char *fpath)
    {
        //Read the vertex shader source code from its file.
        std::ifstream fpvertex(vpath);
        if (!fpvertex.is_open())
            fprintf(stderr, "Error : '%s' not found. Exiting...\n", vpath);

        std::string vtemp;
        vtemp.assign( (std::istreambuf_iterator<char>(fpvertex)), (std::istreambuf_iterator<char>()) );
        const char *vsource = vtemp.c_str();
        
        //Compile the vertex shader and check for errors.
        unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vsource, NULL);
        glCompileShader(vshader);
        int success;
        char infolog[1024];
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vshader, 1024, NULL, infolog);
            fprintf(stderr, "Error while compiling '%s'.\n", vpath);
            fprintf(stderr, "%s\n", infolog);
        }
        
        //Read the fragment shader source code from its file.
        std::ifstream fpfragment(fpath);
        if (!fpfragment.is_open())
            fprintf(stderr, "Error : '%s' not found. Exiting...\n", fpath);

        std::string ftemp;
        ftemp.assign( (std::istreambuf_iterator<char>(fpfragment)), (std::istreambuf_iterator<char>()) );
        const char *fsource = ftemp.c_str();
        
        //Compile the fragment shader and check for errors.
        unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fsource, NULL);
        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fshader, 1024, NULL, infolog);
            fprintf(stderr, "Error while compiling '%s'.\n", fpath);
            fprintf(stderr, "%s\n", infolog);
        }
        
        //Handle linking.
        ID = glCreateProgram();
        glAttachShader(ID, vshader);
        glAttachShader(ID, fshader);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 1024, NULL, infolog);
            fprintf(stderr, "Error while linking shader program ('%s' || '%s').\n", vpath, fpath);
            fprintf(stderr, "%s\n", infolog);
        }
        
        //We no longer need the vshader and fshader, so let's delete them from now.
        //We DO need however the ID, which will be kept for deletion in the destructor.
        glDeleteShader(vshader);
        glDeleteShader(fshader);
    }

    //Delete the shader.
    ~shader()
    {
        glDeleteProgram(ID);
    }
    
    //Activate the current shader.
    void use()
    {
        glUseProgram(ID);
    }

    //The following member functions are used to pass uniform variables to the shaders from the main code.
    
    //Pass to the currently active shader 1 int (uniform).
    void set_int_uniform(const std::string &name, int value)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform1i(location, value);
    }
    
    //Pass to the currently active shader 1 float (uniform).
    void set_float_uniform(const std::string &name, float value)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform1f(location, value);
    }
    
    //Pass to the currently active shader 2 floats (uniform).
    void set_vec2_uniform(const std::string &name, float x, float y)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform2f(location, x,y);
    }
    
    //Pass to the currently active shader 1 vector of 2 floats (uniform).
    void set_vec2_uniform(const std::string &name, glm::vec2 &v)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform2fv(location, 1, &v[0]);
    }
    
    //Pass to the currently active shader 3 floats (uniform).
    void set_vec3_uniform(const std::string &name, float x, float y, float z)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform3f(location, x,y,z);
    }
    
    //Pass to the currently active shader 1 vector of 3 floats (uniform).
    void set_vec3_uniform(const std::string &name, glm::vec3 &v)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform3fv(location, 1, &v[0]);
    }
    
    //Pass to the currently active shader 4 floats (uniform).
    void set_vec4_uniform(const std::string &name, float x, float y, float z, float w)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform4f(location, x,y,z,w);
    }
    
    //Pass to the currently active shader 1 vector of 4 floats (uniform).
    void set_vec4_uniform(const std::string &name, glm::vec4 &v)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniform4fv(location, 1, &v[0]);
    }
    
    //Pass to the currently active shader 1 2x2 float matrix (uniform).
    void set_mat2_uniform(const std::string &name, glm::mat2 &m)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix2fv(location, 1, GL_FALSE, &m[0][0]);
    }
    
    //Pass to the currently active shader 1 3x3 float matrix (uniform).
    void set_mat3_uniform(const std::string &name, glm::mat3 &m)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix3fv(location, 1, GL_FALSE, &m[0][0]);
    }
    
    //Pass to the currently active shader 1 4x4 float matrix (uniform).
    void set_mat4_uniform(const std::string &name, glm::mat4 &m)
    {
        unsigned location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
    }
};

#endif
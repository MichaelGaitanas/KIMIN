/* This class contains the shader logic/commands, such as loading, compiling, linking, etc. */

#ifndef SHADER_H
#define SHADER_H

#include<GL/glew.h>
#include<glm/glm.hpp>

#include<cstdio>
#include<fstream>
#include<string>
#include<iterator>

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
            fprintf(stderr, "[Warning] : shader() constructor failed to load '%s'. Proceeding...\n", vpath);

        std::string vtemp;
        vtemp.assign( (std::istreambuf_iterator<char>(fpvertex)), (std::istreambuf_iterator<char>()) );
        const char *vsource = vtemp.c_str();
        
        //Compile the vertex shader and check for errors.
        unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vsource, nullptr);
        glCompileShader(vshader);
        int success;
        char infolog[1024];
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vshader, 1024, nullptr, infolog);
            fprintf(stderr, "[Warning] : shader() failed at compiling '%s'.\n", vpath);
            fprintf(stderr, "%s\n", infolog);
            fprintf(stderr, "Proceeding...\n");
        }
        
        //Read the fragment shader source code from its file.
        std::ifstream fpfragment(fpath);
        if (!fpfragment.is_open())
            fprintf(stderr, "[Warning] : shader() constructor failed to load '%s'. Proceeding...\n", fpath);

        std::string ftemp;
        ftemp.assign( (std::istreambuf_iterator<char>(fpfragment)), (std::istreambuf_iterator<char>()) );
        const char *fsource = ftemp.c_str();
        
        //Compile the fragment shader and check for errors.
        unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fsource, nullptr);
        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fshader, 1024, nullptr, infolog);
            fprintf(stderr, "[Warning] : shader() failed at compiling '%s'.\n", fpath);
            fprintf(stderr, "%s\n", infolog);
            fprintf(stderr, "Proceeding...\n");
        }
        
        //Handle linking.
        ID = glCreateProgram();
        glAttachShader(ID, vshader);
        glAttachShader(ID, fshader);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 1024, nullptr, infolog);
            fprintf(stderr, "[Warning] : shader() failed at linking shader program ('%s' || '%s').\n", vpath, fpath);
            fprintf(stderr, "%s\n", infolog);
            fprintf(stderr, "Proceeding...\n");
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
    
    //Pass to the currently active shader 1 int.
    void set_int_uniform(const std::string &name, int value)
    {
        int location = glGetUniformLocation(ID, name.c_str());
        if (location >= 0)
            glUniform1i(location, value);
        else
            fprintf(stderr,"[Warning] : shader()::set_int_uniform() failed to pass uniform.\n");
    }

    //Pass to the currently active shader 1 float.
    void set_float_uniform(const std::string &name, float value)
    {
        int location = glGetUniformLocation(ID, name.c_str());
        if (location >= 0)
            glUniform1f(location, value);
        else
            fprintf(stderr,"[Warning] : shader()::set_float_uniform() failed to pass uniform.\n");
    }
    
    //Pass to the currently active shader 1 vector of 2 floats.
    void set_vec2_uniform(const std::string &name, const glm::vec2 &v)
    {
        int location = glGetUniformLocation(ID, name.c_str());
        if (location >= 0)
            glUniform2fv(location, 1, &v[0]);
        else
            fprintf(stderr,"[Warning] : shader()::set_vec2_uniform() failed to pass uniform.\n");
    }

    //Pass to the currently active shader 1 vector of 3 floats.
    void set_vec3_uniform(const std::string &name, const glm::vec3 &v)
    {
        int location = glGetUniformLocation(ID, name.c_str());
        if (location >= 0)
            glUniform3fv(location, 1, &v[0]);
        else
            fprintf(stderr,"[Warning] : shader()::set_vec3_uniform() failed to pass uniform.\n");
    }
    
    //Pass to the currently active shader 1 4x4 float matrix.
    void set_mat4_uniform(const std::string &name, const glm::mat4 &m)
    {
        int location = glGetUniformLocation(ID, name.c_str());
        if (location >= 0)
            glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
        else
            fprintf(stderr,"[Warning] : shader()::set_mat4_uniform() failed to pass uniform.\n");
    }
};

#endif
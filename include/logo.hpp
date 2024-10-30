#ifndef LOGO_HPP
#define LOGO_HPP

#include<GL/glew.h>

#include<cstdio>
#include<GLFW/glfw3.h>

#include"shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

void draw_logo_for_seconds(const char *img_path, const float seconds)
{
    //Initialize glfw along with some settings.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int win_width = mode->width;
    int win_height = mode->height;
    float win_aspect_ratio = win_width/(float)win_height;

    //Create the (transparent) window.
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Logo", nullptr, nullptr);
    if (window == nullptr)
    {
        fprintf(stderr, "Error : Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    //Glew setup.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Error : Failed to initialize glew. Exiting...\n");
        return;
    }

    //Gpu memory setup regarding image texture.
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures.

    //Load the image texture.
    int img_width, img_height, img_channels;
    float img_aspect_ratio;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *img_data = stbi_load(img_path, &img_width, &img_height, &img_channels, 0);
    if (!img_data)
    {
        fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", img_path);
        exit(EXIT_FAILURE);
    }
    img_aspect_ratio = img_width/(float)img_height;

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

    //Calculate mesh quad's dimensions based on aspect ratio.
    float quad_width = 1.0f, quad_height = 1.0f;
    if (img_aspect_ratio > win_aspect_ratio) //The image is wider than the window, so scale height.
        quad_height = win_aspect_ratio/img_aspect_ratio;
    else //Image is taller than the window, so scale width.
        quad_width = img_aspect_ratio/win_aspect_ratio;

    quad_width *= 0.5f;
    quad_height *= 0.5f;

    //Procedural quad mesh.
    float interleaved_buffer[] = {  //Positions.  //UVs.
                                    -quad_width,  quad_height, 0.0f,  0.0f, 1.0f,
                                    -quad_width, -quad_height, 0.0f,  0.0f, 0.0f,
                                     quad_width, -quad_height, 0.0f,  1.0f, 0.0f,
                                        
                                    -quad_width,  quad_height, 0.0f,  0.0f, 1.0f,
                                     quad_width, -quad_height, 0.0f,  1.0f, 0.0f,
                                     quad_width,  quad_height, 0.0f,  1.0f, 1.0f };

    //Gpu memory setup regarding position vertices and UVs.
    unsigned int vao, vbo;
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

    //Create the shader used to calculate each pixel's color for the quad mesh to render.
    shader shad_logo("../shaders/vertex/texture.vert","../shaders/fragment/texture.frag");

    while (glfwGetTime() <= seconds)
    {
        shad_logo.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &tex);

    glfwDestroyWindow(window);
    glfwTerminate();

    return;
}

#endif
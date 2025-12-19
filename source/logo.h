/* This header contains one function only : that which renders the logo frame in the beginning of KIMIN. */

#ifndef LOGO_H
#define LOGO_H

#include<cstdio>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include"shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

bool render_logo(const char *img_path, const float seconds)
{
    //Initialize glfw along with some settings.
    if(!glfwInit())
    {
        fprintf(stderr, "[Warning] : render_logo() failed to initialize glfw. Returned false.\n");
        glfwTerminate();
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    int win_width = mode->width, win_height = mode->height;
    float win_aspect_ratio = win_width/float(win_height);

    //Create the (transparent) window.
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Logo", nullptr, nullptr);
    if (window == nullptr)
    {
        fprintf(stderr, "[Warning] : 'render_logo()' failed to create glfw window. Returned false.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    //Glew setup.
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "[Warning] : 'render_logo()' failed to initialize glew. Returned false.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    //Load the image texture.
    int img_width, img_height, img_channels;
    float img_aspect_ratio;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *img_data = stbi_load(img_path, &img_width, &img_height, &img_channels, 0);
    if (!img_data)
    {
        fprintf(stderr, "[Warning] : 'render_logo()' failed to load stbi image. Returned false.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }
    img_aspect_ratio = img_width/float(img_height);

    //Determine the correct format based on the number of channels (img_channels).
    GLenum format;
    if (img_channels == 1)
        format = GL_RED; //Single-channel (grayscale image).
    else if (img_channels == 3)
        format = GL_RGB; //Classical 3-channel image (e.g. jpg).
    else if (img_channels == 4)
        format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).
    else
    {
        fprintf(stderr, "[Warning] : 'render_logo()' loaded unknown image format. Returned false.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    //GPU memory setup regarding image texture.
    unsigned tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is for textures with non-standard dimensions or single-channel textures.
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img_data); //Free image resources.

    //Calculate mesh quad's dimensions based on aspect ratio.
    float quad_width = 1.0f, quad_height = 1.0f;
    if (img_aspect_ratio > win_aspect_ratio) //The image is wider than the window, so scale height.
        quad_height = win_aspect_ratio/img_aspect_ratio;
    else //Image is taller than the window, so scale width.
        quad_width = img_aspect_ratio/win_aspect_ratio;

    const float unifrom_logo_scale_factor = 0.6f;
    quad_width  *= unifrom_logo_scale_factor;
    quad_height *= unifrom_logo_scale_factor;

    //Procedural quad mesh.
    float interleaved_buffer[] = {  //Positions.  //UVs.
                                    -quad_width,  quad_height, 0.0f,  0.0f, 1.0f,
                                    -quad_width, -quad_height, 0.0f,  0.0f, 0.0f,
                                     quad_width, -quad_height, 0.0f,  1.0f, 0.0f,
                                        
                                    -quad_width,  quad_height, 0.0f,  0.0f, 1.0f,
                                     quad_width, -quad_height, 0.0f,  1.0f, 0.0f,
                                     quad_width,  quad_height, 0.0f,  1.0f, 1.0f };

    //Gpu memory setup regarding position vertices and UVs.
    unsigned vao, vbo;
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
    shader sh_logo("../shaders/vertex/texture.vert","../shaders/fragment/texture.frag");
    sh_logo.use();
    sh_logo.set_int_uniform("sample_tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(vao);

    glfwSetTime(0.0);
    while (glfwGetTime() <= seconds)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &tex);

    //Kill the (transparent) window and completely terminate glfw. We will initialize it again for the main app window.
    glfwDestroyWindow(window);
    glfwTerminate();

    return true;
}

#endif
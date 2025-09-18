/* This class is called only by scene_panel.h and is responsible to 'assemble' the rendering logic of the 3D scene
   at every frame (bind framebuffers, clear color/depth buffers, switch shaders, apply matrix multiplications, etc...). */

#ifndef RENDERER3D_H
#define RENDERER3D_H

#include"constant.h"
#include"typedef.h"
#include"shader.h"
#include"polyhedron.h"
#include"light.h"
#include"camera.h"
#include"orbmesh.h"
#include"solution.h"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

class renderer3D
{
private:
    shader sh_depth, sh_dlight_shadow, sh_orb; //Shaders...
    unsigned int depth_fbo, depth_tex; //IDs to hold the depth fbo and the depth texture for the shadow map.
    glm::vec3 xaxis_col, yaxis_col, zaxis_col; //Colors of the body-frame axes (red, green, blue, respectively).
    polyhedron xaxis, yaxis, zaxis; //Body-frame axes models.

public:
    camera cam;
    light sunlight;
    orbmesh orb1, orb2, orb_sp;
    
    int depth_reso; //Shadow image resolution.
    glm::vec3 aster1_col, aster2_col;
    glm::vec3 orb1_col, orb2_col, orb_sp_col;

    bool render_aster1, render_aster2;
    bool render_axes1, render_axes2;
    bool render_orb1, render_orb2, render_orb_sp;
    bool orb1_match, orb2_match, orb_sp_match;

    int win_width, win_height;
    
    renderer3D() : sh_depth("../shaders/vertex/trans_dir_light_mvp.vert","../shaders/fragment/nothing.frag"),
                   sh_dlight_shadow("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dir_light_ad_shadow.frag"),
                   sh_orb("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag"),
                   depth_fbo(0),
                   depth_tex(0),
                   xaxis_col(glm::vec3(1.0f,0.0f,0.0f)),
                   yaxis_col(glm::vec3(0.0f,1.0f,0.0f)),
                   zaxis_col(glm::vec3(0.0f,0.0f,1.0f)),
                   xaxis(),
                   yaxis(),
                   zaxis(),
                   cam(),
                   sunlight(),
                   orb1(),
                   orb2(),
                   orb_sp(),
                   depth_reso(2048),
                   aster1_col(glm::vec3(1.0f)),
                   aster2_col(glm::vec3(1.0f)),
                   orb1_col(glm::vec3(0.0f,0.75f,0.75f)),
                   orb2_col(glm::vec3(0.0f,0.75f,0.75f)),
                   orb_sp_col(glm::vec3(0.0f,0.75f,0.75f)),
                   render_aster1(true),
                   render_aster2(true),
                   render_axes1(false),
                   render_axes2(false),
                   render_orb1(false),
                   render_orb2(false),
                   render_orb_sp(false),
                   orb1_match(false),
                   orb2_match(false),
                   orb_sp_match(false),
                   win_width(1),
                   win_height(1)
    {
        xaxis.load_obj_file("../obj/axes/xaxis.obj"); xaxis.gen_norms(); xaxis.set_as_gl_mesh();
        yaxis.load_obj_file("../obj/axes/yaxis.obj"); yaxis.gen_norms(); yaxis.set_as_gl_mesh();
        zaxis.load_obj_file("../obj/axes/zaxis.obj"); zaxis.gen_norms(); zaxis.set_as_gl_mesh();
    }

    ~renderer3D()
    {
        if (depth_tex)
            glDeleteTextures(1, &depth_tex);
        if (depth_fbo)
            glDeleteFramebuffers(1, &depth_fbo);
    }

    //(Re)set the depth framebuffer, used for shadowing. This is one of the resets that we can't run in scene_panel::setup() due to the separate thread issue.
    //So this will run only once in the render_3D_content() after the simulation is terminated or it will run every time the user changes the depth_reso from the gui exposed slider.
    void setup_depth_fbo()
    {
        if (depth_fbo)
        {
            glDeleteFramebuffers(1, &depth_fbo);
            glDeleteTextures(1, &depth_tex);
        }
        glGenFramebuffers(1, &depth_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
        glGenTextures(1, &depth_tex);
        glBindTexture(GL_TEXTURE_2D, depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, depth_reso, depth_reso, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); //Shadow mapping is highly sensitive to depth precision, hence the 32 bits.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr float border_col[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Pure white that is, coz white corresponds to maximum depth.
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border_col[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fprintf(stderr, "[Warning] : In renderer3D::setup_depth_fbo(), the depth framebuffer is not completed.\n");
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //Reset all GPU resources that depend on a finished simulation. It is called from the render thread when reset_essential == true.
    //This function is basically the continuation of the scene_panel::setup(const solution &sol), but unfortunately they run on different threads. Hence the separation.
    void reset_gpu_resources(solution &sol)
    {
        orb1.clear();
        orb2.clear();
        orb_sp.clear();
        sol.integr.properties.poly1.clear_gl_mesh();
        sol.integr.properties.poly2.clear_gl_mesh();
        orb1.set_as_gl_mesh(sol.x, sol.y, sol.z, static_cast<float>(sol.integr.com1_coeff));
        orb2.set_as_gl_mesh(sol.x, sol.y, sol.z, static_cast<float>(sol.integr.com2_coeff));
        if (sol.integr.properties.spacecraft_checkbox)
            orb_sp.set_as_gl_mesh(sol.x_sp, sol.y_sp, sol.z_sp, 1.0f);
        sol.integr.properties.poly1.set_as_gl_mesh();
        sol.integr.properties.poly2.set_as_gl_mesh();
        setup_depth_fbo();
    }

    //This function handles the rendering logic of the 3D content.
    void render_3D_content(solution &sol, const size_t i, bool &reset_essential)
    {
        //Prepare all the meshes for rendering, by running the appropriate CPU/GPU tasks.
        if (reset_essential)
        {
            reset_gpu_resources(sol);   
            reset_essential = false;
        }

        sunlight.set_geometry();
        cam.set_geometry(win_width/(float)win_height);

        sh_dlight_shadow.use();
        sh_dlight_shadow.set_mat4_uniform("projection", cam.projection);
        sh_dlight_shadow.set_mat4_uniform("view", cam.view);
        sh_dlight_shadow.set_mat4_uniform("light_pv", sunlight.pv);
        sh_dlight_shadow.set_vec3_uniform("light_dir", sunlight.dir);
        sh_depth.use();
        sh_depth.set_mat4_uniform("light_pv", sunlight.pv);

        glm::mat4 I = glm::mat4(1.0f);

        glm::mat4 T1R1 = glm::translate(I, (float)sol.integr.com1_coeff*glm::vec3(sol.x[i],sol.y[i],sol.z[i]))*
                         glm::rotate(I, glm::radians((float)sol.yaw1[i]),   glm::vec3(0.0f,0.0f,1.0f))*
                         glm::rotate(I, glm::radians((float)sol.pitch1[i]), glm::vec3(0.0f,1.0f,0.0f))*
                         glm::rotate(I, glm::radians((float)sol.roll1[i]),  glm::vec3(1.0f,0.0f,0.0f));
        glm::mat4 S1 = glm::scale(I, glm::vec3((float)sol.integr.brillouin1));

        glm::mat4 T2R2 = glm::translate(I, (float)sol.integr.com2_coeff*glm::vec3(sol.x[i],sol.y[i],sol.z[i]))*
                         glm::rotate(I, glm::radians((float)sol.yaw2[i]),   glm::vec3(0.0f,0.0f,1.0f))*
                         glm::rotate(I, glm::radians((float)sol.pitch2[i]), glm::vec3(0.0f,1.0f,0.0f))*
                         glm::rotate(I, glm::radians((float)sol.roll2[i]),  glm::vec3(1.0f,0.0f,0.0f));
        glm::mat4 S2 = glm::scale(I, glm::vec3((float)sol.integr.brillouin2));
        
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
        glViewport(0,0, depth_reso,depth_reso);
        glClear(GL_DEPTH_BUFFER_BIT);
        sh_depth.set_mat4_uniform("model", T1R1);
        if (render_aster1)
            sol.integr.properties.poly1.render();
        if (render_axes1)
        {
            sh_depth.set_mat4_uniform("model", T1R1*S1);
            xaxis.render(); yaxis.render(); zaxis.render();
        }
        sh_depth.set_mat4_uniform("model", T2R2);
        if (render_aster2)
            sol.integr.properties.poly2.render();
        if (render_axes2)
        {
            sh_depth.set_mat4_uniform("model", T2R2*S2);
            xaxis.render(); yaxis.render(); zaxis.render();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width,win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_tex);
        sh_dlight_shadow.use();
        sh_dlight_shadow.set_int_uniform("sample_shadow", 0);
        sh_dlight_shadow.set_mat4_uniform("model", T1R1);
        sh_dlight_shadow.set_vec3_uniform("mesh_col", aster1_col);
        if (render_aster1)
            sol.integr.properties.poly1.render();
        if (render_axes1)
        {
            sh_dlight_shadow.set_mat4_uniform("model", T1R1*S1);
            sh_dlight_shadow.set_vec3_uniform("mesh_col", xaxis_col); xaxis.render();
            sh_dlight_shadow.set_vec3_uniform("mesh_col", yaxis_col); yaxis.render();
            sh_dlight_shadow.set_vec3_uniform("mesh_col", zaxis_col); zaxis.render();
        }
        sh_dlight_shadow.set_mat4_uniform("model", T2R2);
        sh_dlight_shadow.set_vec3_uniform("mesh_col", aster2_col);
        if (render_aster2)
            sol.integr.properties.poly2.render();
        if (render_axes2)
        {
            sh_dlight_shadow.set_mat4_uniform("model", T2R2*S2);
            sh_dlight_shadow.set_vec3_uniform("mesh_col", xaxis_col); xaxis.render();
            sh_dlight_shadow.set_vec3_uniform("mesh_col", yaxis_col); yaxis.render();
            sh_dlight_shadow.set_vec3_uniform("mesh_col", zaxis_col); zaxis.render();
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        sh_orb.use();
        sh_orb.set_mat4_uniform("projection", cam.projection);
        sh_orb.set_mat4_uniform("view", cam.view);
        sh_orb.set_mat4_uniform("model", I);
        sh_orb.set_vec3_uniform("mesh_col", orb1_col);
        if (render_orb1)
            orb1.render();
        sh_orb.set_vec3_uniform("mesh_col", orb2_col);
        if (render_orb2)
            orb2.render();
        sh_orb.set_vec3_uniform("mesh_col", orb_sp_col);
        if (render_orb_sp)
            orb_sp.render();
    }
};

#endif
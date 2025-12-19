/* This class is responsible to assemble the rendering logic of the 3D scene at every frame (bind framebuffers, clear color/depth buffers, switch shaders, apply matrix multiplications, etc.). */

#ifndef RENDERER3D_H
#define RENDERER3D_H

#include<memory>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include"constants.h"
#include"typedef.h"
#include"solution.h"
#include"shader.h"
#include"polyhedron.h"
#include"dlight.h"
#include"camera.h"
#include"orbit.h"
#include"skybox.h"
#include"grid.h"
#include"sun.h"

class renderer3D
{
private:
    shader sh_depth, sh_dlight, sh_orb, sh_skybox, sh_sun, sh_grid;
    polyhedron xaxis, yaxis, zaxis;
    std::unique_ptr<skybox> sky;
    sun sunquad;

    unsigned depth_fbo, depth_tex; //IDs to hold the depth framebuffer and the depth texture for the shadow map algorithm.

public:
    camera cam;
    dlight sunlight;
    orbit orb1, orb2, orb_sp;
    grid infgrid;

    int depth_reso; //Actual depth image resolution in pixels (for the shadow map).

    glm::vec3 body1_col, body2_col, xaxis_col, yaxis_col, zaxis_col, orb1_col, orb2_col, orb_sp_col, sun_col;
    bool render_body1, render_body2, render_axes1, render_axes2, render_orb1, render_orb2, render_orb_sp, render_grid, render_skybox; //These correspond to the GUI checkboxes state.

    int win_width, win_height;
    
    renderer3D() : sh_depth("../shaders/vertex/trans_dlight_mvp.vert","../shaders/fragment/nothing.frag"),
                   sh_dlight("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dlight_ad_shadow.frag"),
                   sh_orb("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag"),
                   sh_skybox("../shaders/vertex/skybox.vert","../shaders/fragment/skybox.frag"),
                   sh_sun("../shaders/vertex/sun.vert", "../shaders/fragment/sun.frag"),
                   sh_grid("../shaders/vertex/grid.vert", "../shaders/fragment/grid.frag"),
                   xaxis(),
                   yaxis(),
                   zaxis(),
                   sky(nullptr),
                   sunquad(),
                   depth_fbo(0),
                   depth_tex(0),
                   cam(),
                   sunlight(),
                   orb1(),
                   orb2(),
                   orb_sp(),
                   infgrid(),
                   depth_reso(DEPTH_RESO_INIT),
                   body1_col(glm::vec3(0.8f)),
                   body2_col(glm::vec3(0.8f)),
                   xaxis_col(glm::vec3(1.0f,0.0f,0.0f)),
                   yaxis_col(glm::vec3(0.0f,1.0f,0.0f)),
                   zaxis_col(glm::vec3(0.0f,0.0f,1.0f)),
                   orb1_col(glm::vec3(0.7f,0.0f,0.0f)),
                   orb2_col(glm::vec3(0.0f,0.7f,0.0f)),
                   orb_sp_col(glm::vec3(0.0f,0.75f,0.75f)),
                   sun_col(glm::vec3(1.0f,0.65f,0.0f)),
                   render_body1(true),
                   render_body2(true),
                   render_axes1(false),
                   render_axes2(false),
                   render_orb1(false),
                   render_orb2(false),
                   render_orb_sp(false),
                   render_grid(false),
                   render_skybox(true),
                   win_width(1),
                   win_height(1)
    {
        xaxis.load_obj_file("../obj/axes/xaxis.obj"); xaxis.gen_norms(); xaxis.set_gl_mesh();
        yaxis.load_obj_file("../obj/axes/yaxis.obj"); yaxis.gen_norms(); yaxis.set_gl_mesh();
        zaxis.load_obj_file("../obj/axes/zaxis.obj"); zaxis.gen_norms(); zaxis.set_gl_mesh();
    }

    ~renderer3D()
    {
        if (depth_tex)
            glDeleteTextures(1, &depth_tex);
        if (depth_fbo)
            glDeleteFramebuffers(1, &depth_fbo);
    }

    //(Re)set the depth framebuffer, used for shadowing. This is one of the resets that we can't run in scene::setup() due to the separate thread issue.
    //So this will run only once in the render_3D_content() after the simulation is terminated or it will run every time the user changes the 'depth_reso' from the gui exposed slider.
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, depth_reso, depth_reso, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
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

    //This function resets all GPU-side resources that depend on a finished simulation.
    void reset_gpu_resources(solution &sol)
    {
        sol.integr.props.poly1.clear_gl_mesh();
        sol.integr.props.poly2.clear_gl_mesh();
        orb1.clear_gl_mesh();
        orb2.clear_gl_mesh();
        orb_sp.clear_gl_mesh();

        sol.integr.props.poly1.set_gl_mesh();
        sol.integr.props.poly2.set_gl_mesh();
        orb1.set_gl_mesh(sol.xmut, sol.ymut, sol.zmut, (float)sol.integr.m1);
        orb2.set_gl_mesh(sol.xmut, sol.ymut, sol.zmut, (float)sol.integr.m2);
        if (sol.integr.props.spacecraft_checkbox)
            orb_sp.set_gl_mesh(sol.xsp_com, sol.ysp_com, sol.zsp_com); //Spacecraft's orbit mesh in the COM frame of the binary
        
        //This will run only once no matter how many times the reset_gpu_resources() is called.
        if (!sky) sky = std::make_unique<skybox>("../skybox/starfield2k/right.jpg",
                                                 "../skybox/starfield2k/left.jpg",
                                                 "../skybox/starfield2k/top.jpg",
                                                 "../skybox/starfield2k/bottom.jpg",
                                                 "../skybox/starfield2k/front.jpg",
                                                 "../skybox/starfield2k/back.jpg");
        setup_depth_fbo();
    }

    //This function handles the rendering logic of the 3D content.
    void render_3D_content(solution &sol, const size_t iframe, bool &reset_gpu_flag)
    {
        if (reset_gpu_flag)
        {
            reset_gpu_resources(sol);
            reset_gpu_flag = false;
        }

        const glm::vec3 rsun = -glm::vec3(sol.xcom_helio[iframe],sol.ycom_helio[iframe],sol.zcom_helio[iframe]);
        const glm::vec3 r1 = float(sol.integr.m1)*glm::vec3(sol.xmut[iframe],sol.ymut[iframe],sol.zmut[iframe]);
        const glm::vec3 r2 = float(sol.integr.m2)*glm::vec3(sol.xmut[iframe],sol.ymut[iframe],sol.zmut[iframe]);
        const float sun_dist = glm::length(rsun);
        const glm::vec3 sun_dir = rsun/sun_dist;

        sunlight.set_geometry(float(OBJ_AXES_LENGTH*std::max(sol.integr.brillouin1, sol.integr.brillouin2) + sol.dist_mut[iframe]), sun_dir);
        cam.set_geometry(win_width/float(win_height));

        const glm::mat4 I = glm::mat4(1.0f);
        const glm::mat4 T1R1 = glm::translate(I, r1)*
                               glm::rotate(I, glm::radians(float(sol.yaw1[iframe])),   glm::vec3(0.0f,0.0f,1.0f))*
                               glm::rotate(I, glm::radians(float(sol.pitch1[iframe])), glm::vec3(0.0f,1.0f,0.0f))*
                               glm::rotate(I, glm::radians(float(sol.roll1[iframe])),  glm::vec3(1.0f,0.0f,0.0f));
        const glm::mat4 S1 = glm::scale(I, glm::vec3(sol.integr.brillouin1));
        const glm::mat4 T2R2 = glm::translate(I, r2)*
                               glm::rotate(I, glm::radians(float(sol.yaw2[iframe])),   glm::vec3(0.0f,0.0f,1.0f))*
                               glm::rotate(I, glm::radians(float(sol.pitch2[iframe])), glm::vec3(0.0f,1.0f,0.0f))*
                               glm::rotate(I, glm::radians(float(sol.roll2[iframe])),  glm::vec3(1.0f,0.0f,0.0f));
        const glm::mat4 S2 = glm::scale(I, glm::vec3(sol.integr.brillouin2));
        
        //Shadow rendering pass : render the meshes that account for shadow, but do so from the light's (orthographic) view. Shadow pass must always happen first.
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
        glViewport(0,0, depth_reso,depth_reso);
        glClear(GL_DEPTH_BUFFER_BIT);
        sh_dlight.use();
        sh_dlight.set_mat4_uniform("projection", cam.projection);
        sh_dlight.set_mat4_uniform("view", cam.view);
        sh_dlight.set_mat4_uniform("light_pv", sunlight.pv);
        sh_dlight.set_vec3_uniform("light_dir", sun_dir);
        sh_depth.use();
        sh_depth.set_mat4_uniform("light_pv", sunlight.pv);
        sh_depth.set_mat4_uniform("model", T1R1);
        if (render_body1)
            sol.integr.props.poly1.render();
        if (render_axes1)
        {
            sh_depth.set_mat4_uniform("model", T1R1*S1);
            xaxis.render(); yaxis.render(); zaxis.render();
        }
        sh_depth.set_mat4_uniform("model", T2R2);
        if (render_body2)
            sol.integr.props.poly2.render();
        if (render_axes2)
        {
            sh_depth.set_mat4_uniform("model", T2R2*S2);
            xaxis.render(); yaxis.render(); zaxis.render();
        }
        
        //Rest of the meshes rendering pass : now we render the meshes from the camera's (perspective) view.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width,win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Skybox rendering pass :
        if (sky && render_skybox)
        {
            const glm::mat4 sky_view  = glm::mat4(glm::mat3(cam.view)); //View but no translation part.
            const glm::mat4 sky_model = glm::rotate(I, glm::radians(180.0f), glm::vec3(0.0f,0.0f,1.0f))*
                                        glm::rotate(I, glm::radians(90.0f),  glm::vec3(1.0f,0.0f,0.0f));
            sh_skybox.use();
            sh_skybox.set_int_uniform("skybox", 0);
            sh_skybox.set_mat4_uniform("projection", cam.projection);
            sh_skybox.set_mat4_uniform("view",  sky_view);
            sh_skybox.set_mat4_uniform("model", sky_model);
            glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_CUBE_MAP, sky->tex);
            glDepthFunc(GL_LEQUAL); //Look at the shader skybox.vert : I have forced all fragments' depth values to be 1.0. So for the depth test to pass, I change the test operation to '<=' instead of the default '<'.
            glDepthMask(GL_FALSE); //This ain't needed for the particular order of rendering, but let it be some sort of guard for any future update...
            sky->render();
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS); //Restore to the default depth operation to '<'.
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        //Sun rendering pass :
        const float sun_ang_deg = glm::degrees(asinf(RSUN/sun_dist));
        sh_sun.use();
        sh_sun.set_mat4_uniform("projection",      cam.projection);
        sh_sun.set_mat4_uniform("view",            cam.view);
        sh_sun.set_vec3_uniform("light_dir_world", sun_dir);
        sh_sun.set_vec3_uniform("sun_color",       sun_col);
        sh_sun.set_float_uniform("sun_angular_radius_deg", sun_ang_deg);
        sh_sun.set_float_uniform("sun_distance", 10.0f*cam.max_dist); //Put the Sun comfortably 'far'. The aim is to make occlude only the skybox but no other mesh.
        sh_sun.set_float_uniform("sun_scale", 1.0f);
        sh_sun.set_float_uniform("sun_disc_intensity", sunquad.disc_intensity);
        sh_sun.set_float_uniform("sun_disc_edge_soft", sunquad.disc_edge_soft);
        sh_sun.set_float_uniform("sun_limb_strength",  sunquad.limb_strength);
        sh_sun.set_float_uniform("sun_limb_power",     sunquad.limb_power);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        sunquad.render();
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        //Polyhedra rendering pass :
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_tex);
        sh_dlight.use();
        sh_dlight.set_int_uniform("sample_shadow", 0);
        sh_dlight.set_mat4_uniform("model", T1R1);
        sh_dlight.set_vec3_uniform("mesh_col", body1_col);
        if (render_body1)
            sol.integr.props.poly1.render();
        if (render_axes1)
        {
            sh_dlight.set_mat4_uniform("model", T1R1*S1);
            sh_dlight.set_vec3_uniform("mesh_col", xaxis_col); xaxis.render();
            sh_dlight.set_vec3_uniform("mesh_col", yaxis_col); yaxis.render();
            sh_dlight.set_vec3_uniform("mesh_col", zaxis_col); zaxis.render();
        }
        sh_dlight.set_mat4_uniform("model", T2R2);
        sh_dlight.set_vec3_uniform("mesh_col", body2_col);
        if (render_body2)
            sol.integr.props.poly2.render();
        if (render_axes2)
        {
            sh_dlight.set_mat4_uniform("model", T2R2*S2);
            sh_dlight.set_vec3_uniform("mesh_col", xaxis_col); xaxis.render();
            sh_dlight.set_vec3_uniform("mesh_col", yaxis_col); yaxis.render();
            sh_dlight.set_vec3_uniform("mesh_col", zaxis_col); zaxis.render();
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        //Orbits rendering pass :
        if (render_orb1 || render_orb2 || render_orb_sp)
        {
            sh_orb.use();
            sh_orb.set_mat4_uniform("projection", cam.projection);
            sh_orb.set_mat4_uniform("view", cam.view);
            sh_orb.set_mat4_uniform("model", I);
            if (render_orb1)
            {
                sh_orb.set_vec3_uniform("mesh_col", orb1_col);
                orb1.render();
            }
            if (render_orb2)
            {
                sh_orb.set_vec3_uniform("mesh_col", orb2_col);
                orb2.render();
            }
            if (render_orb_sp)
            {
                sh_orb.set_vec3_uniform("mesh_col", orb_sp_col);
                orb_sp.render();
            }
        }
        
        //Infinite grid rendering pass :
        if (render_grid)
        {
            sh_grid.use();
            sh_grid.set_mat4_uniform("uProj", cam.projection);
            sh_grid.set_mat4_uniform("uView", cam.view);
            sh_grid.set_float_uniform("uFadeEnd", 2.0f*cam.dist);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            infgrid.render();
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glDisable(GL_BLEND);
        }
    }
};

#endif
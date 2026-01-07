/* This class is responsible to assemble the rendering logic of the 3D scene at every frame (bind framebuffers, clear color/depth buffers, switch shaders, apply matrix multiplications, etc.). */

#ifndef RENDERER3D_H
#define RENDERER3D_H

#include<memory>
#include<cmath>

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
#include"quad.h"

class renderer3D
{
private:
    shader sh_depth, sh_dlight, sh_orb, sh_skybox, sh_sun, sh_grid;
    polyhedron xaxis, yaxis, zaxis;
    std::unique_ptr<skybox> stars_ptr, starmap_ptr, galaxy_ptr;

    unsigned depth_fbo_id, depth_tex_id; //IDs to hold the depth framebuffer and the depth texture for the shadow map algorithm.

public:
    dlight sunlight;
    camera cam;
    quad sun, ecliptic_grid;
    orbit orb1, orb2, orb_sp;

    int depth_reso; //Actual depth image resolution in pixels (for the shadow map).

    bool render_body1, render_body2, render_axes1, render_axes2, render_orb1, render_orb2, render_orb_sp, render_ecliptic_grid, render_stars, render_starmap, render_galaxy, render_sun; //These correspond to the GUI checkboxes state.

    int win_width, win_height;
    
    renderer3D() : sh_depth("../shaders/vertex/trans_dlight_mvp.vert","../shaders/fragment/nothing.frag"),
                   sh_dlight("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/lambert.frag"),
                   sh_orb("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag"),
                   sh_skybox("../shaders/vertex/skybox.vert","../shaders/fragment/skybox.frag"),
                   sh_sun("../shaders/vertex/sun.vert", "../shaders/fragment/sun.frag"),
                   sh_grid("../shaders/vertex/grid.vert", "../shaders/fragment/grid.frag"),
                   xaxis(),
                   yaxis(),
                   zaxis(),
                   stars_ptr(nullptr),
                   starmap_ptr(nullptr),
                   galaxy_ptr(nullptr),
                   depth_fbo_id(0),
                   depth_tex_id(0),
                   sunlight(),
                   cam(),
                   sun(),
                   ecliptic_grid(),
                   orb1(),
                   orb2(),
                   orb_sp(),
                   depth_reso(DEPTH_RESO_INIT),
                   render_body1(true),
                   render_body2(true),
                   render_axes1(false),
                   render_axes2(false),
                   render_orb1(false),
                   render_orb2(false),
                   render_orb_sp(false),
                   render_ecliptic_grid(false),
                   render_stars(true),
                   render_starmap(false),
                   render_galaxy(false),
                   render_sun(true),
                   win_width(1),
                   win_height(1)
    {
        xaxis.load_obj_file("../obj/axes/xaxis.obj"); xaxis.gen_norms(); xaxis.set_gl_mesh();
        yaxis.load_obj_file("../obj/axes/yaxis.obj"); yaxis.gen_norms(); yaxis.set_gl_mesh();
        zaxis.load_obj_file("../obj/axes/zaxis.obj"); zaxis.gen_norms(); zaxis.set_gl_mesh();
    }

    ~renderer3D()
    {
        if (depth_tex_id)
            glDeleteTextures(1, &depth_tex_id);
        if (depth_fbo_id)
            glDeleteFramebuffers(1, &depth_fbo_id);
    }

    //(Re)set the depth framebuffer, used for shadowing. This is one of the resets that we can't run in scene::setup() due to the separate thread issue.
    //So this will run only once in the render_3D_content() after the simulation is terminated or it will run every time the user changes the 'depth_reso' from the gui exposed slider.
    void setup_depth_fbo()
    {
        if (depth_fbo_id)
        {
            glDeleteFramebuffers(1, &depth_fbo_id);
            glDeleteTextures(1, &depth_tex_id);
        }
        glGenFramebuffers(1, &depth_fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo_id);
        glGenTextures(1, &depth_tex_id);
        glBindTexture(GL_TEXTURE_2D, depth_tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, depth_reso, depth_reso, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        constexpr float border_col[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Pure white that is, coz white corresponds to maximum depth.
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border_col[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex_id, 0);
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
        orb1.set_gl_mesh(sol.xmut, sol.ymut, sol.zmut, float(sol.integr.m1));
        orb2.set_gl_mesh(sol.xmut, sol.ymut, sol.zmut, float(sol.integr.m2));
        if (sol.integr.props.spacecraft_checkbox)
            orb_sp.set_gl_mesh(sol.xsp_com, sol.ysp_com, sol.zsp_com); //Spacecraft's orbit mesh in the COM frame of the binary

        setup_depth_fbo();
        
        //This will run only once no matter how many times the reset_gpu_resources() is called.
        if (!stars_ptr)
             stars_ptr   = std::make_unique<skybox>("../skybox/stars2k/right.png",
                                                    "../skybox/stars2k/left.png",
                                                    "../skybox/stars2k/top.png",
                                                    "../skybox/stars2k/bottom.png",
                                                    "../skybox/stars2k/front.png",
                                                    "../skybox/stars2k/back.png");
        if (!starmap_ptr)
             starmap_ptr = std::make_unique<skybox>("../skybox/starmap2k/right.png",
                                                    "../skybox/starmap2k/left.png",
                                                    "../skybox/starmap2k/top.png",
                                                    "../skybox/starmap2k/bottom.png",
                                                    "../skybox/starmap2k/front.png",
                                                    "../skybox/starmap2k/back.png");
        if (!galaxy_ptr)
             galaxy_ptr  = std::make_unique<skybox>("../skybox/galaxy2k/right.png",
                                                    "../skybox/galaxy2k/left.png",
                                                    "../skybox/galaxy2k/top.png",
                                                    "../skybox/galaxy2k/bottom.png",
                                                    "../skybox/galaxy2k/front.png",
                                                    "../skybox/galaxy2k/back.png");
    }

    //This function handles the rendering logic of the 3D content.
    void render_3D_content(solution &sol, const size_t iframe, bool &reset_gpu_flag)
    {
        if (reset_gpu_flag)
        {
            reset_gpu_resources(sol);
            reset_gpu_flag = false;
        }

        const glm::vec3 r1com = float(sol.integr.m1)*glm::vec3(sol.xmut[iframe],sol.ymut[iframe],sol.zmut[iframe]);
        const glm::vec3 r2com = float(sol.integr.m2)*glm::vec3(sol.xmut[iframe],sol.ymut[iframe],sol.zmut[iframe]);
        glm::vec3 pivot;
        if (cam.mode == camera::MODE_COM)
            pivot = glm::vec3(0.0f);
        else if (cam.mode == camera::MODE_BODY1)
            pivot = r1com;
        else
            pivot = r2com;

        sunlight.set_geometry(float(OBJ_AXES_LENGTH*std::max(sol.integr.brillouin1, sol.integr.brillouin2) + sol.dist_mut[iframe]), -glm::vec3(sol.xcom_helio[iframe],sol.ycom_helio[iframe],sol.zcom_helio[iframe]) );
        cam.set_geometry(win_width/float(win_height), pivot);

        const glm::mat4 I = glm::mat4(1.0f);
        const glm::mat4 T1R1 = glm::translate(I, r1com)*
                               glm::rotate(I, glm::radians(float(sol.yaw1[iframe])),   glm::vec3(0.0f,0.0f,1.0f))*
                               glm::rotate(I, glm::radians(float(sol.pitch1[iframe])), glm::vec3(0.0f,1.0f,0.0f))*
                               glm::rotate(I, glm::radians(float(sol.roll1[iframe])),  glm::vec3(1.0f,0.0f,0.0f));
        const glm::mat4 S1 = glm::scale(I, glm::vec3(sol.integr.brillouin1));
        const glm::mat4 T2R2 = glm::translate(I, r2com)*
                               glm::rotate(I, glm::radians(float(sol.yaw2[iframe])),   glm::vec3(0.0f,0.0f,1.0f))*
                               glm::rotate(I, glm::radians(float(sol.pitch2[iframe])), glm::vec3(0.0f,1.0f,0.0f))*
                               glm::rotate(I, glm::radians(float(sol.roll2[iframe])),  glm::vec3(1.0f,0.0f,0.0f));
        const glm::mat4 S2 = glm::scale(I, glm::vec3(sol.integr.brillouin2));
        
        //Shadow rendering pass : render the meshes that account for shadow, but do so from the light's (orthographic) view. Shadow pass must always happen first.
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo_id);
        glViewport(0,0, depth_reso,depth_reso);
        glClear(GL_DEPTH_BUFFER_BIT);
        sh_dlight.use();
        sh_dlight.set_uniform_mat4("projection", cam.projection);
        sh_dlight.set_uniform_mat4("view", cam.view);
        sh_dlight.set_uniform_mat4("light_pv", sunlight.pv);
        sh_dlight.set_uniform_vec3("light_dir", sunlight.dir);
        sh_depth.use();
        sh_depth.set_uniform_mat4("light_pv", sunlight.pv);
        sh_depth.set_uniform_mat4("model", T1R1);
        if (render_body1)
            sol.integr.props.poly1.render();
        if (render_axes1)
        {
            sh_depth.set_uniform_mat4("model", T1R1*S1);
            xaxis.render(); yaxis.render(); zaxis.render();
        }
        sh_depth.set_uniform_mat4("model", T2R2);
        if (render_body2)
            sol.integr.props.poly2.render();
        if (render_axes2)
        {
            sh_depth.set_uniform_mat4("model", T2R2*S2);
            xaxis.render(); yaxis.render(); zaxis.render();
        }
        
        //Rest of the meshes rendering pass : now we render the meshes from the camera's (perspective) view.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width,win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Skybox rendering pass :
        if ((stars_ptr && starmap_ptr && galaxy_ptr) && (render_stars || render_starmap || render_galaxy))
        {
            const glm::mat4 sky_view  = glm::mat4(glm::mat3(cam.view)); //View but no translation part.
            const glm::mat4 sky_model = glm::rotate(I, glm::radians(180.0f), glm::vec3(0.0f,0.0f,1.0f))*
                                        glm::rotate(I, glm::radians(90.0f),  glm::vec3(1.0f,0.0f,0.0f));
            sh_skybox.use();
            sh_skybox.set_uniform_mat4("projection", cam.projection);
            sh_skybox.set_uniform_mat4("view",  sky_view);
            sh_skybox.set_uniform_mat4("model", sky_model);
            sh_skybox.set_uniform_int("skybox_stars", 0);
            sh_skybox.set_uniform_int("skybox_starmap", 1);
            sh_skybox.set_uniform_int("skybox_galaxy", 2);
            sh_skybox.set_uniform_int("render_stars",   render_stars   ? 1 : 0);
            sh_skybox.set_uniform_int("render_starmap", render_starmap ? 1 : 0);
            sh_skybox.set_uniform_int("render_galaxy",  render_galaxy  ? 1 : 0);
            //We are gonna blend 3 texture units in the skybox shader, so we activate, then bind the corresponding and finally unbind. 
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, stars_ptr->tex_id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, starmap_ptr->tex_id);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_CUBE_MAP, galaxy_ptr->tex_id);
            glDepthFunc(GL_LEQUAL); //Look at the shader skybox.vert : I have forced all fragments' depth values to be 1.0. Hence the depth test ALWAYS passes, because this line changes the test operation to '<=' instead of the default '<'.
            glDepthMask(GL_FALSE); //This ain't needed for the particular order of rendering, but let it be some sort of guard for any future update...
            stars_ptr->render(); //Any of the 3 skyboxes is correct to render due to the structure of the fragment shader.
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS); //Restore to the default depth operation to '<'.
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        //Sun rendering pass :
        if (render_sun)
        {
            sh_sun.use();
            sh_sun.set_uniform_mat4("projection", cam.projection);
            sh_sun.set_uniform_mat4("view", cam.view);
            sh_sun.set_uniform_vec3("light_dir", sunlight.dir);
            sh_sun.set_uniform_float("apparent_angular_radius", asinf(RSUN/sunlight.dist));
            sh_sun.set_uniform_float("quad_distance", CAM_SUN_MAX_DIST_SCALE*cam.max_dist);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            sun.render();
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glDisable(GL_BLEND);
        }
        
        //Lit polyhedra rendering pass :
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_tex_id);
        sh_dlight.use();
        sh_dlight.set_uniform_int("sample_shadow", 0);
        sh_dlight.set_uniform_mat4("model", T1R1);
        sh_dlight.set_uniform_vec3("mesh_col", BODY1_COL);
        if (render_body1)
            sol.integr.props.poly1.render();
        if (render_axes1)
        {
            sh_dlight.set_uniform_mat4("model", T1R1*S1);
            sh_dlight.set_uniform_vec3("mesh_col", XAXIS_COL); xaxis.render();
            sh_dlight.set_uniform_vec3("mesh_col", YAXIS_COL); yaxis.render();
            sh_dlight.set_uniform_vec3("mesh_col", ZAXIS_COL); zaxis.render();
        }
        sh_dlight.set_uniform_mat4("model", T2R2);
        sh_dlight.set_uniform_vec3("mesh_col", BODY2_COL);
        if (render_body2)
            sol.integr.props.poly2.render();
        if (render_axes2)
        {
            sh_dlight.set_uniform_mat4("model", T2R2*S2);
            sh_dlight.set_uniform_vec3("mesh_col", XAXIS_COL); xaxis.render();
            sh_dlight.set_uniform_vec3("mesh_col", YAXIS_COL); yaxis.render();
            sh_dlight.set_uniform_vec3("mesh_col", ZAXIS_COL); zaxis.render();
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        //Orbits rendering pass :
        if (render_orb1 || render_orb2 || render_orb_sp)
        {
            sh_orb.use();
            sh_orb.set_uniform_mat4("projection", cam.projection);
            sh_orb.set_uniform_mat4("view", cam.view);
            sh_orb.set_uniform_mat4("model", I);
            if (render_orb1)
            {
                sh_orb.set_uniform_vec3("mesh_col", ORB1_COL);
                orb1.render();
            }
            if (render_orb2)
            {
                sh_orb.set_uniform_vec3("mesh_col", ORB2_COL);
                orb2.render();
            }
            if (render_orb_sp)
            {
                sh_orb.set_uniform_vec3("mesh_col", ORB_SP_COL);
                orb_sp.render();
            }
        }
        
        //Ecliptic 'infinite' grid rendering pass :
        if (render_ecliptic_grid)
        {
            sh_grid.use();
            sh_grid.set_uniform_mat4("projection", cam.projection);
            sh_grid.set_uniform_mat4("view", cam.view);
            sh_grid.set_uniform_float("fade_end_dist", CAM_GRID_DIST_SCALE*cam.dist);
            sh_grid.set_uniform_vec3("grid_origin", pivot);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            ecliptic_grid.render();
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glDisable(GL_BLEND);
        }
    }
};

#endif
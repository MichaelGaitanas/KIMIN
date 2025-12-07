#ifndef SOLUTION_H
#define SOLUTION_H

#include<cstdio>
#include<cmath>
#include<cstring>
#include<string>

#ifdef USE_OPENMP
    #include<omp.h>
#endif

#include"constants.h"
#include"typedef.h"
#include"linalg.h"
#include"conversion.h"
#include"properties_panel.h"
#include"integrator.h"

class solution
{
public:
    integrator integr;

    //The following vector members constitute the DENSE version of the solution of the ODEs and each vector is used for :
    //1) Exporting to files.
    //2) 3D rendering.
    //3) Both.

    dvec t;

    //Mutual of the binary :
    dvec x, y, z, dist;
    dvec vx, vy, vz;
    dvec sma, ecc, inc, raan, argper, manom;

    //Body 1 :
    dvec roll1, pitch1, yaw1;
    dvec w1ix, w1iy, w1iz;

    //Body 2 :
    dvec roll2, pitch2, yaw2;
    dvec w2ix, w2iy, w2iz;

    //Spacecraft (in binary's COM frame) :
    dvec xsp,  ysp,  zsp;
    dvec vxsp, vysp, vzsp;

    solution() { } //This is needed to instantiate solution in the scene_panel class.

    solution(const integrator &integr) //This is needed to instantiate solution in the gui class.
    {
        this->integr = integr;
    }

    //This function fills with orbital data all the vector members.
    void construct(console_panel &console)
    {
        console.add_timed_text("[Solution] : Constructing solution... ");

        const size_t N = integr.orbit.size();
        t.resize(N);
        
        x.resize(N);   y.resize(N);   z.resize(N);   dist.resize(N);
        vx.resize(N);  vy.resize(N);  vz.resize(N);
        sma.resize(N); ecc.resize(N); inc.resize(N); raan.resize(N); argper.resize(N); manom.resize(N);
        
        roll1.resize(N); pitch1.resize(N); yaw1.resize(N);
        w1ix.resize(N);  w1iy.resize(N);   w1iz.resize(N);

        roll2.resize(N); pitch2.resize(N); yaw2.resize(N);       
        w2ix.resize(N);  w2iy.resize(N);   w2iz.resize(N);
        
        if (integr.properties.spacecraft_checkbox)
        {
            xsp.resize(N);  ysp.resize(N);  zsp.resize(N);
            vxsp.resize(N); vysp.resize(N); vzsp.resize(N);
        }

        #ifdef _OPENMP
            int total_threads = omp_get_max_threads();
            int half_threads  = (total_threads > 1 ? total_threads/2 : 1);
        #else
            constexpr int half_threads = 1;
        #endif
        (void)half_threads;

        size_t i;
        #ifdef _OPENMP
            #pragma omp parallel for schedule(static)\
                                     num_threads(half_threads)
        #endif
        for (i = 0; i < N; ++i)
        {
            //Remember, integr.orbit[i][] contains either (t, x,y,z, vx,vy,vz, q10,q11,q12,q13, w1bx,w1by,w1bz, q20,q21,q22,q23, w2bx,w2by,w2bz, xsp,ysp,zsp, vxsp,vysp,vzsp)
            //                                         or (t, x,y,z, vx,vy,vz, q10,q11,q12,q13, w1bx,w1by,w1bz, q20,q21,q22,q23, w2bx,w2by,w2bz)
            dvec3 r   = {integr.orbit[i][1],  integr.orbit[i][2],  integr.orbit[i][3]};
            dvec3 v   = {integr.orbit[i][4],  integr.orbit[i][5],  integr.orbit[i][6]};
            dvec4 q1  = {integr.orbit[i][7],  integr.orbit[i][8],  integr.orbit[i][9],  integr.orbit[i][10]};
            dvec3 w1b = {integr.orbit[i][11], integr.orbit[i][12], integr.orbit[i][13]};
            dvec4 q2  = {integr.orbit[i][14], integr.orbit[i][15], integr.orbit[i][16], integr.orbit[i][17]};
            dvec3 w2b = {integr.orbit[i][18], integr.orbit[i][19], integr.orbit[i][20]};

            dmat3 A1   = quat2mat(q1);
            dmat3 A2   = quat2mat(q2);
            dvec3 w1i  = body2iner(w1b,A1);
            dvec3 w2i  = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);

            dvec6 kep = cart2kep({r[0],r[1],r[2], v[0],v[1],v[2]}, G*(integr.properties.M1 + integr.properties.M2));
            
            t[i] = integr.orbit[i][0]/86400.0; //[days]

            x[i]    = r[0];
            y[i]    = r[1];
            z[i]    = r[2];
            dist[i] = length(r);

            vx[i] = v[0];
            vy[i] = v[1];
            vz[i] = v[2];

            sma[i]    = kep[0];
            ecc[i]    = kep[1];
            inc[i]    = kep[2]*180.0/PI;
            raan[i]   = kep[3]*180.0/PI;
            argper[i] = kep[4]*180.0/PI;
            manom[i]  = kep[5]*180.0/PI;

            roll1[i]  = rpy1[0]*180.0/PI;
            pitch1[i] = rpy1[1]*180.0/PI;
            yaw1[i]   = rpy1[2]*180.0/PI;

            w1ix[i] = w1i[0];
            w1iy[i] = w1i[1];
            w1iz[i] = w1i[2];

            roll2[i]  = rpy2[0]*180.0/PI;
            pitch2[i] = rpy2[1]*180.0/PI;
            yaw2[i]   = rpy2[2]*180.0/PI;

            w2ix[i] = w2i[0];
            w2iy[i] = w2i[1];
            w2iz[i] = w2i[2];

            if (integr.properties.spacecraft_checkbox)
            {
                dvec3 rsp = {integr.orbit[i][21], integr.orbit[i][22], integr.orbit[i][23]};
                dvec3 vsp = {integr.orbit[i][24], integr.orbit[i][25], integr.orbit[i][26]};

                xsp[i] = rsp[0];
                ysp[i] = rsp[1];
                zsp[i] = rsp[2];

                vxsp[i] = vsp[0];
                vysp[i] = vsp[1];
                vzsp[i] = vsp[2];
            }
        }
        console.add_text("Done.\n");
    }

    void export_files(console_panel &console)
    {
        console.add_timed_text("[Solution] : Exporting solution files... ");

        std::string sim_dir = std::string(SIM_ROOT_DIR) + integr.properties.sim_name;
        std::filesystem::create_directory(SIM_ROOT_DIR); //Create the root directory that shall host all simulations if it does not exist already.
        std::filesystem::create_directory(sim_dir); //Create the current simulation directory that shall host all solution files.

        //Create the .txt contents :
        FILE *fp_t    = fopen((sim_dir + "/time.txt"       ).c_str(), "w");
        FILE *fp_pos  = fopen((sim_dir + "/rel_pos.txt"    ).c_str(), "w");
        FILE *fp_vel  = fopen((sim_dir + "/rel_vel.txt"    ).c_str(), "w");
        FILE *fp_kep  = fopen((sim_dir + "/keplerian.txt"  ).c_str(), "w");
        FILE *fp_rpy1 = fopen((sim_dir + "/euler_rpy1.txt" ).c_str(), "w");
        FILE *fp_w1i  = fopen((sim_dir + "/ang_vel_w1i.txt").c_str(), "w");
        FILE *fp_rpy2 = fopen((sim_dir + "/euler_rpy2.txt" ).c_str(), "w");
        FILE *fp_w2i  = fopen((sim_dir + "/ang_vel_w2i.txt").c_str(), "w");
        //Export the solution vectors.
        for (size_t i = 0; i < t.size(); ++i)
        {
            fprintf(fp_t,        "%.16lf\n", t[i]);
            fprintf(fp_pos,      "%.16lf %.16lf %.16lf\n",                        x[i],      y[i],    z[i]);
            fprintf(fp_vel,      "%.16lf %.16lf %.16lf\n",                       vx[i],     vy[i],   vz[i]);
            fprintf(fp_kep,      "%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n", sma[i],    ecc[i],  inc[i], raan[i], argper[i], manom[i]);
            fprintf(fp_rpy1,     "%.16lf %.16lf %.16lf\n",                    roll1[i], pitch1[i], yaw1[i]);
            fprintf(fp_w1i,      "%.16lf %.16lf %.16lf\n",                     w1ix[i],   w1iy[i], w1iz[i]);
            fprintf(fp_rpy2,     "%.16lf %.16lf %.16lf\n",                    roll2[i], pitch2[i], yaw2[i]);
            fprintf(fp_w2i,      "%.16lf %.16lf %.16lf\n",                     w2ix[i],   w2iy[i], w2iz[i]); 
        }
        fclose(fp_t);
        fclose(fp_pos);
        fclose(fp_vel);
        fclose(fp_kep);
        fclose(fp_rpy1);
        fclose(fp_w1i);
        fclose(fp_rpy2);
        fclose(fp_w2i);

        //Export spacecraft.
        if (integr.properties.spacecraft_checkbox)
        {
            FILE *fp_pos_sp = fopen((sim_dir + "/pos_spacecraft.txt").c_str(), "w");
            FILE *fp_vel_sp = fopen((sim_dir + "/vel_spacecraft.txt").c_str(), "w");
            for (size_t i = 0; i < t.size(); ++i)
            {
                fprintf(fp_pos_sp, "%.16lf %.16lf %.16lf\n",  xsp[i],  ysp[i],  zsp[i]);
                fprintf(fp_vel_sp, "%.16lf %.16lf %.16lf\n", vxsp[i], vysp[i], vzsp[i]);
            }
            fclose(fp_pos_sp);
            fclose(fp_vel_sp);
        }

        //Export the collision status.
        FILE *fp_collision = fopen((sim_dir + "/collision.txt").c_str(),"w");
        fprintf(fp_collision,"Collision detected : %s", (integr.collision || integr.collision_sp) ? "Yes" : "No");
        fclose(fp_collision);

        //Export the input properties.
        FILE *fp_properties = fopen((sim_dir + "/properties.txt").c_str(),"w");
        fprintf(fp_properties,"Simulation name := \"%s\"\n\n", integr.properties.sim_name);
        
        if (integr.properties.ell_checkbox)
        {
            fprintf(fp_properties,"Shape models := \"Ellipsoids\"\n");
            fprintf(fp_properties,"    a1 := %.15g\n",  integr.properties.semiaxes1[0]);
            fprintf(fp_properties,"    b1 := %.15g\n",  integr.properties.semiaxes1[1]);
            fprintf(fp_properties,"    c1 := %.15g\n",  integr.properties.semiaxes1[2]);
            fprintf(fp_properties,"    a2 := %.15g\n",  integr.properties.semiaxes2[0]);
            fprintf(fp_properties,"    b2 := %.15g\n",  integr.properties.semiaxes2[1]);
            fprintf(fp_properties,"    c2 := %.15g\n\n",integr.properties.semiaxes2[2]);
        }
        else
        {
            fprintf(fp_properties,"Shape models := \".obj files\"\n");
            fprintf(fp_properties,"    file 1 := \"%s\"\n",  integr.properties.obj1_path.c_str());
            fprintf(fp_properties,"    file 2 := \"%s\"\n\n",integr.properties.obj2_path.c_str());
        }

        if (integr.properties.ord2_checkbox)
            fprintf(fp_properties,"Mutual potential order := 2\n\n");
        else if (integr.properties.ord3_checkbox)
            fprintf(fp_properties,"Mutual potential order := 3\n\n");
        else
            fprintf(fp_properties,"Mutual potential order := 4\n\n");

        fprintf(fp_properties,"Masses :\n");
        fprintf(fp_properties,"    M1 := %.15g\n",  integr.properties.M1);
        fprintf(fp_properties,"    M2 := %.15g\n\n",integr.properties.M2);

        fprintf(fp_properties,"Numerical integration :\n");
        if (integr.properties.integration_method == properties_panel::RKF78_FIXED)
        {
            fprintf(fp_properties,"    Method := \"RKF78 (fixed)\"\n");
            fprintf(fp_properties,"    Epoch := %.15g\n",integr.properties.epoch);
            fprintf(fp_properties,"    Duration := %.15g\n",integr.properties.dur);
            fprintf(fp_properties,"    Step := %.15g\n\n",integr.properties.step);
        }
        else if (integr.properties.integration_method == properties_panel::RKF78_ADAPTIVE)
        {
            fprintf(fp_properties,"    Method := \"RKF78 (adaptive)\"\n");
            fprintf(fp_properties,"    Epoch := %.15g\n",integr.properties.epoch);
            fprintf(fp_properties,"    Duration := %.15g\n",integr.properties.dur);
            fprintf(fp_properties,"    Target error := %.15g\n\n",integr.properties.target_error);
        }
        else if (integr.properties.integration_method == properties_panel::BSTOER_ADAPTIVE)
        {
            fprintf(fp_properties,"    Method := \"BStoer (adaptive)\"\n");
            fprintf(fp_properties,"    Epoch := %.15g\n",integr.properties.epoch);
            fprintf(fp_properties,"    Duration := %.15g\n",integr.properties.dur);
            fprintf(fp_properties,"    Target error := %.15g\n\n",integr.properties.target_error);
        }
        else //properties_panel::ABM5_FIXED
        {
            fprintf(fp_properties,"    Method := \"ABM5 (fixed)\"\n");
            fprintf(fp_properties,"    Epoch := %.15g\n",integr.properties.epoch);
            fprintf(fp_properties,"    Duration := %.15g\n",integr.properties.dur);
            fprintf(fp_properties,"    Step := %.15g\n\n",integr.properties.step);
        }

        fprintf(fp_properties,"Initial state :\n");
        if (integr.properties.pos_vel_var == properties_panel::CARTESIAN)
        {
            fprintf(fp_properties,"    Relative position and velocity := \"Cartesian\"\n");
            fprintf(fp_properties,"    x  := %.15g\n",   integr.properties.cart[0]);
            fprintf(fp_properties,"    y  := %.15g\n",   integr.properties.cart[1]);
            fprintf(fp_properties,"    z  := %.15g\n",   integr.properties.cart[2]);
            fprintf(fp_properties,"    vx := %.15g\n",   integr.properties.cart[3]);
            fprintf(fp_properties,"    vy := %.15g\n",   integr.properties.cart[4]);
            fprintf(fp_properties,"    vz := %.15g\n\n", integr.properties.cart[5]);
        }
        else //properties_panel::KEPLERIAN
        {
            fprintf(fp_properties,"    Relative position and velocity := \"Keplerian\"\n");
            fprintf(fp_properties,"    a  := %.15g\n",   integr.properties.kep[0]);
            fprintf(fp_properties,"    e  := %.15g\n",   integr.properties.kep[1]);
            fprintf(fp_properties,"    i  := %.15g\n",   integr.properties.kep[2]);
            fprintf(fp_properties,"    Om := %.15g\n",   integr.properties.kep[3]);
            fprintf(fp_properties,"    w  := %.15g\n",   integr.properties.kep[4]);
            fprintf(fp_properties,"    M  := %.15g\n\n", integr.properties.kep[5]);
        }

        if (integr.properties.orient_var == properties_panel::EULER_XYZ)
        {
            fprintf(fp_properties,"    Orientations := \"Euler angles (XYZ)\"\n");
            fprintf(fp_properties,"    Roll 1  := %.15g\n",   integr.properties.rpy1[0]);
            fprintf(fp_properties,"    Pitch 1 := %.15g\n",   integr.properties.rpy1[1]);
            fprintf(fp_properties,"    Yaw 1   := %.15g\n",   integr.properties.rpy1[2]);
            fprintf(fp_properties,"    Roll 2  := %.15g\n",   integr.properties.rpy2[0]);
            fprintf(fp_properties,"    Pitch 2 := %.15g\n",   integr.properties.rpy2[1]);
            fprintf(fp_properties,"    Yaw 2   := %.15g\n\n", integr.properties.rpy2[2]);
        }
        else //properties_panel::QUATERNION
        {
            fprintf(fp_properties,"    Orientations := \"Quaternions (WXYZ)\"\n");
            fprintf(fp_properties,"    q10 := %.15g\n",   integr.properties.q1[0]);
            fprintf(fp_properties,"    q11 := %.15g\n",   integr.properties.q1[1]);
            fprintf(fp_properties,"    q12 := %.15g\n",   integr.properties.q1[2]);
            fprintf(fp_properties,"    q13 := %.15g\n",   integr.properties.q1[3]);
            fprintf(fp_properties,"    q20 := %.15g\n",   integr.properties.q2[0]);
            fprintf(fp_properties,"    q21 := %.15g\n",   integr.properties.q2[1]);
            fprintf(fp_properties,"    q22 := %.15g\n",   integr.properties.q2[2]);
            fprintf(fp_properties,"    q23 := %.15g\n\n", integr.properties.q2[3]);
        }

        if (integr.properties.angvel_frame == properties_panel::INERTIAL)
        {
            fprintf(fp_properties,"    Angular velocities := \"At inertial frame\"\n");
            fprintf(fp_properties,"    w1ix := %.15g\n",   integr.properties.w1i[0]);
            fprintf(fp_properties,"    w1iy := %.15g\n",   integr.properties.w1i[1]);
            fprintf(fp_properties,"    w1iz := %.15g\n",   integr.properties.w1i[2]);
            fprintf(fp_properties,"    w2ix := %.15g\n",   integr.properties.w2i[0]);
            fprintf(fp_properties,"    w2iy := %.15g\n",   integr.properties.w2i[1]);
            fprintf(fp_properties,"    w2iz := %.15g\n\n", integr.properties.w2i[2]);
        }
        else //properties_panel::BODY
        {
            fprintf(fp_properties,"    Angular velocities := \"At body frames\"\n");
            fprintf(fp_properties,"    w1bx := %.15g\n",   integr.properties.w1b[0]);
            fprintf(fp_properties,"    w1by := %.15g\n",   integr.properties.w1b[1]);
            fprintf(fp_properties,"    w1bz := %.15g\n",   integr.properties.w1b[2]);
            fprintf(fp_properties,"    w2bx := %.15g\n",   integr.properties.w2b[0]);
            fprintf(fp_properties,"    w2by := %.15g\n",   integr.properties.w2b[1]);
            fprintf(fp_properties,"    w2bz := %.15g\n\n", integr.properties.w2b[2]);
        }

        fprintf(fp_properties,"    COM motion relative to world :\n");
        fprintf(fp_properties,"    x  := %.15g\n",   integr.properties.rcom[0]);
        fprintf(fp_properties,"    y  := %.15g\n",   integr.properties.rcom[1]);
        fprintf(fp_properties,"    z  := %.15g\n",   integr.properties.rcom[2]);
        fprintf(fp_properties,"    vx := %.15g\n",   integr.properties.vcom[0]);
        fprintf(fp_properties,"    vy := %.15g\n",   integr.properties.vcom[1]);
        fprintf(fp_properties,"    vz := %.15g\n\n", integr.properties.vcom[2]);

        if (integr.properties.collision_no)
            fprintf(fp_properties,"Collision shapes := \"No collision\"\n\n");
        else if (integr.properties.collision_spheres)
            fprintf(fp_properties,"Collision shapes := \"Spheres\"\n\n");
        else
            fprintf(fp_properties,"Collision shapes := \"Polyhedra\"\n\n");

        if (integr.properties.impactors_checkbox)
        {
            fprintf(fp_properties,"Assume kinetic impactors := \"Yes\"\n");
            fprintf(fp_properties,"    Body 1 :\n");
            fprintf(fp_properties,"        Mass (dry + fuel) :\n");
            fprintf(fp_properties,"            m := %.15g\n", integr.properties.mD1);
            fprintf(fp_properties,"        Velocity (relative to body) :\n");
            fprintf(fp_properties,"            vx := %.15g\n", integr.properties.vD1[0]);
            fprintf(fp_properties,"            vy := %.15g\n", integr.properties.vD1[1]);
            fprintf(fp_properties,"            vz := %.15g\n", integr.properties.vD1[2]);
            fprintf(fp_properties,"        Momentum enhancement factor (ejecta) :\n");
            fprintf(fp_properties,"            beta := %.15g\n", integr.properties.beta1);
            fprintf(fp_properties,"        Impact epoch :\n");
            fprintf(fp_properties,"            t := %.15g\n", integr.properties.tD1/86400.0);
            fprintf(fp_properties,"    Body 2 :\n");
            fprintf(fp_properties,"        Mass (dry + fuel) :\n");
            fprintf(fp_properties,"            m := %.15g\n", integr.properties.mD2);
            fprintf(fp_properties,"        Velocity (relative to body) :\n");
            fprintf(fp_properties,"            vx := %.15g\n", integr.properties.vD2[0]);
            fprintf(fp_properties,"            vy := %.15g\n", integr.properties.vD2[1]);
            fprintf(fp_properties,"            vz := %.15g\n", integr.properties.vD2[2]);
            fprintf(fp_properties,"        Momentum enhancement factor (ejecta) :\n");
            fprintf(fp_properties,"            beta := %.15g\n", integr.properties.beta2);
            fprintf(fp_properties,"        Impact epoch :\n");
            fprintf(fp_properties,"            t := %.15g\n\n", integr.properties.tD2/86400.0);
        }
        else
            fprintf(fp_properties, "Assume kinetic impactors := \"No\"\n\n");

        if (integr.properties.spacecraft_checkbox)
        {
            fprintf(fp_properties,"Assume spacecraft orbiter := \"Yes\"\n");
            if (integr.properties.pos_vel_sp_var == properties_panel::CARTESIAN_SP)
            {
                fprintf(fp_properties,"    Position and velocity := \"Cartesian (binary COM)\"\n");
                fprintf(fp_properties,"        x  := %.15g\n",   integr.properties.cart_sp[0]);
                fprintf(fp_properties,"        y  := %.15g\n",   integr.properties.cart_sp[1]);
                fprintf(fp_properties,"        z  := %.15g\n",   integr.properties.cart_sp[2]);
                fprintf(fp_properties,"        vx := %.15g\n",   integr.properties.cart_sp[3]);
                fprintf(fp_properties,"        vy := %.15g\n",   integr.properties.cart_sp[4]);
                fprintf(fp_properties,"        vz := %.15g\n\n", integr.properties.cart_sp[5]);
            }
            else if (integr.properties.pos_vel_sp_var == properties_panel::CARTESIAN_SP1)
            {
                fprintf(fp_properties,"    Position and velocity := \"Cartesian (body 1)\"\n");
                fprintf(fp_properties,"        x  := %.15g\n",   integr.properties.cart_sp1[0]);
                fprintf(fp_properties,"        y  := %.15g\n",   integr.properties.cart_sp1[1]);
                fprintf(fp_properties,"        z  := %.15g\n",   integr.properties.cart_sp1[2]);
                fprintf(fp_properties,"        vx := %.15g\n",   integr.properties.cart_sp1[3]);
                fprintf(fp_properties,"        vy := %.15g\n",   integr.properties.cart_sp1[4]);
                fprintf(fp_properties,"        vz := %.15g\n\n", integr.properties.cart_sp1[5]);
            }
            else if (integr.properties.pos_vel_sp_var == properties_panel::CARTESIAN_SP2)
            {
                fprintf(fp_properties,"    Position and velocity := \"Cartesian (body 2)\"\n");
                fprintf(fp_properties,"        x  := %.15g\n",   integr.properties.cart_sp2[0]);
                fprintf(fp_properties,"        y  := %.15g\n",   integr.properties.cart_sp2[1]);
                fprintf(fp_properties,"        z  := %.15g\n",   integr.properties.cart_sp2[2]);
                fprintf(fp_properties,"        vx := %.15g\n",   integr.properties.cart_sp2[3]);
                fprintf(fp_properties,"        vy := %.15g\n",   integr.properties.cart_sp2[4]);
                fprintf(fp_properties,"        vz := %.15g\n\n", integr.properties.cart_sp2[5]);
            }
            else if (integr.properties.pos_vel_sp_var == properties_panel::KEPLERIAN_SP)
            {
                fprintf(fp_properties,"    Position and velocity := \"Keplerian (binary COM)\"\n");
                fprintf(fp_properties,"        a  := %.15g\n",   integr.properties.kep_sp[0]);
                fprintf(fp_properties,"        e  := %.15g\n",   integr.properties.kep_sp[1]);
                fprintf(fp_properties,"        i  := %.15g\n",   integr.properties.kep_sp[2]);
                fprintf(fp_properties,"        Om := %.15g\n",   integr.properties.kep_sp[3]);
                fprintf(fp_properties,"        w  := %.15g\n",   integr.properties.kep_sp[4]);
                fprintf(fp_properties,"        M  := %.15g\n\n", integr.properties.kep_sp[5]);
            }
            else if (integr.properties.pos_vel_sp_var == properties_panel::KEPLERIAN_SP1)
            {
                fprintf(fp_properties,"    Position and velocity := \"Keplerian (body 1)\"\n");
                fprintf(fp_properties,"        a  := %.15g\n",   integr.properties.kep_sp1[0]);
                fprintf(fp_properties,"        e  := %.15g\n",   integr.properties.kep_sp1[1]);
                fprintf(fp_properties,"        i  := %.15g\n",   integr.properties.kep_sp1[2]);
                fprintf(fp_properties,"        Om := %.15g\n",   integr.properties.kep_sp1[3]);
                fprintf(fp_properties,"        w  := %.15g\n",   integr.properties.kep_sp1[4]);
                fprintf(fp_properties,"        M  := %.15g\n\n", integr.properties.kep_sp1[5]);
            }
            else if (integr.properties.pos_vel_sp_var == properties_panel::KEPLERIAN_SP2)
            {
                fprintf(fp_properties,"    Position and velocity := \"Keplerian (body 2)\"\n");
                fprintf(fp_properties,"        a  := %.15g\n",   integr.properties.kep_sp2[0]);
                fprintf(fp_properties,"        e  := %.15g\n",   integr.properties.kep_sp2[1]);
                fprintf(fp_properties,"        i  := %.15g\n",   integr.properties.kep_sp2[2]);
                fprintf(fp_properties,"        Om := %.15g\n",   integr.properties.kep_sp2[3]);
                fprintf(fp_properties,"        w  := %.15g\n",   integr.properties.kep_sp2[4]);
                fprintf(fp_properties,"        M  := %.15g\n\n", integr.properties.kep_sp2[5]);
            }

            if (integr.properties.srp_checkbox)
            {
                fprintf(fp_properties,"    Account for SRP := \"Yes\"\n");
                fprintf(fp_properties,"        SRP parameters : \n");
                fprintf(fp_properties,"            ρ  := %.15g\n", integr.properties.sp_refl);
                fprintf(fp_properties,"            A  := %.15g\n", integr.properties.sp_area);
                fprintf(fp_properties,"            m  := %.15g\n", integr.properties.sp_mass);
                fprintf(fp_properties,"        Sun's position (binary COM) : \n");
                fprintf(fp_properties,"            Dist := %.15g\n", integr.properties.sun_dist);
                fprintf(fp_properties,"            Lon  := %.15g\n", integr.properties.sun_lon);
                fprintf(fp_properties,"            Lat  := %.15g\n", integr.properties.sun_lat);
                if (integr.properties.srp_shadow_checkbox)
                    fprintf(fp_properties,"    Account for shadows := \"Yes\"\n");
                else
                    fprintf(fp_properties,"    Account for shadows := \"No\"\n");
            }
            else
                fprintf(fp_properties,"    Account for SRP := \"No\"\n");
        }
        else
            fprintf(fp_properties,"Assume spacecraft orbiter := \"No\"\n\n");
        
        fclose(fp_properties);

        console.add_text("Done.\n");
    }
};

#endif
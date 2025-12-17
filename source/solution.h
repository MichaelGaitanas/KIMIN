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
#include"properties.h"
#include"integrator.h"

class solution
{
public:
    integrator integr;

    //The following vector members constitute the DENSE version of the solution of the ODEs and each vector is used for either exporting to files or 3D rendering or both.

    dvec t;

    //Mutual state of the binary :
    dvec xmut, ymut, zmut, dist_mut;

    //Body 1 :
    dvec roll1, pitch1, yaw1;
    dvec w1ix, w1iy, w1iz;

    //Body 2 :
    dvec roll2, pitch2, yaw2;
    dvec w2ix, w2iy, w2iz;

    //COM (in Heliocentric frame) :
    dvec xcom_helio, ycom_helio, zcom_helio;

    //Spacecraft :
    //In Heliocentric frame :
    dvec xsp_helio, ysp_helio, zsp_helio;
    //In binary's COM frame :
    dvec xsp_com, ysp_com, zsp_com;

    solution() { } //This is needed to instantiate solution in the scene class.

    solution(const integrator &integr) //This is needed to instantiate solution in the gui class.
    {
        this->integr = integr;
    }

    //This function fills with orbital data all the vector members.
    void construct(console &cons)
    {
        cons.print("[Solution] : Constructing solution... ");

        const size_t N = integr.orbit.size();
        t.resize(N);
        
        xmut.resize(N); ymut.resize(N); zmut.resize(N); dist_mut.resize(N);
        
        roll1.resize(N); pitch1.resize(N); yaw1.resize(N);
        w1ix.resize(N);  w1iy.resize(N);   w1iz.resize(N);

        roll2.resize(N); pitch2.resize(N); yaw2.resize(N);       
        w2ix.resize(N);  w2iy.resize(N);   w2iz.resize(N);

        xcom_helio.resize(N); ycom_helio.resize(N); zcom_helio.resize(N);
        
        if (integr.props.spacecraft_checkbox)
        {
            xsp_helio.resize(N); ysp_helio.resize(N); zsp_helio.resize(N);
            xsp_com.resize(N);   ysp_com.resize(N);   zsp_com.resize(N);
        }

        #ifdef _OPENMP
            int total_threads = omp_get_max_threads();
            int used_threads  = (total_threads > 1 ? total_threads/2 : 1);
        #else
            constexpr int used_threads = 1;
        #endif
        (void)used_threads;

        size_t i;
        #ifdef _OPENMP
            #pragma omp parallel for schedule(static)\
                                     num_threads(used_threads)
        #endif
        for (i = 0; i < N; ++i)
        {
            //Remember, integr.orbit[i][] contains either (t, rmut,vmut, q1,w1b, q2,w2b, rcom_helio,vcom_helio, rsp_helio,vsp_helio)
            //                                         or (t, rmut,vmut, q1,w1b, q2,w2b, rcom_helio,vcom_helio)
            dvec3 rmut       = {integr.orbit[i][1],  integr.orbit[i][2],  integr.orbit[i][3]};
            //dvec3 vmut     = {integr.orbit[i][4],  integr.orbit[i][5],  integr.orbit[i][6]};
            dvec4 q1         = {integr.orbit[i][7],  integr.orbit[i][8],  integr.orbit[i][9],  integr.orbit[i][10]};
            dvec3 w1b        = {integr.orbit[i][11], integr.orbit[i][12], integr.orbit[i][13]};
            dvec4 q2         = {integr.orbit[i][14], integr.orbit[i][15], integr.orbit[i][16], integr.orbit[i][17]};
            dvec3 w2b        = {integr.orbit[i][18], integr.orbit[i][19], integr.orbit[i][20]};
            dvec3 rcom_helio = {integr.orbit[i][21], integr.orbit[i][22], integr.orbit[i][23]};

            dmat3 A1   = quat2mat(q1);
            dmat3 A2   = quat2mat(q2);
            dvec3 w1i  = body2iner(w1b,A1);
            dvec3 w2i  = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);
            
            t[i] = integr.orbit[i][0]/86400.0; //Back in [days].

            xmut[i]     = rmut[0];
            ymut[i]     = rmut[1];
            zmut[i]     = rmut[2];
            dist_mut[i] = length(rmut);

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

            //Remember these are in [km] because we converted the input properties from [AU] to [km] in integrator::prepare().
            xcom_helio[i] = rcom_helio[0];
            ycom_helio[i] = rcom_helio[1];
            zcom_helio[i] = rcom_helio[2];

            if (integr.props.spacecraft_checkbox)
            {
                dvec3 rsp_helio = {integr.orbit[i][27], integr.orbit[i][28], integr.orbit[i][29]};
                //Same here, all are in [km].
                xsp_helio[i] = rsp_helio[0];
                ysp_helio[i] = rsp_helio[1];
                zsp_helio[i] = rsp_helio[2];

                //Again [km].
                xsp_com[i] = xsp_helio[i] - xcom_helio[i];
                ysp_com[i] = ysp_helio[i] - ycom_helio[i];
                zsp_com[i] = zsp_helio[i] - zcom_helio[i];
            }
        }
        cons.print("Done.\n");
    }

    void export_files(console &cons)
    {
        cons.print("[Solution] : Exporting solution files... ");

        std::string sim_dir = std::string(SIM_ROOT_DIR) + integr.props.sim_name;
        std::filesystem::create_directory(SIM_ROOT_DIR); //Create the root directory that shall host all simulations if it does not exist already.
        std::filesystem::create_directory(sim_dir); //Create the current simulation directory that shall host all solution files.

        //Export binary history :
        FILE *fp_t    = fopen((sim_dir + "/time.txt"       ).c_str(), "w");
        FILE *fp_rmut = fopen((sim_dir + "/mutual_pos.txt" ).c_str(), "w");
        FILE *fp_rpy1 = fopen((sim_dir + "/euler_rpy1.txt" ).c_str(), "w");
        FILE *fp_w1i  = fopen((sim_dir + "/ang_vel_w1i.txt").c_str(), "w");
        FILE *fp_rpy2 = fopen((sim_dir + "/euler_rpy2.txt" ).c_str(), "w");
        FILE *fp_w2i  = fopen((sim_dir + "/ang_vel_w2i.txt").c_str(), "w");
        for (size_t i = 0; i < t.size(); ++i)
        {
            fprintf(fp_t,    "%.16lf\n", t[i]);
            fprintf(fp_rmut, "%.16lf %.16lf %.16lf\n",  xmut[i],   ymut[i], zmut[i]);
            fprintf(fp_rpy1, "%.16lf %.16lf %.16lf\n", roll1[i], pitch1[i], yaw1[i]);
            fprintf(fp_w1i,  "%.16lf %.16lf %.16lf\n",  w1ix[i],   w1iy[i], w1iz[i]);
            fprintf(fp_rpy2, "%.16lf %.16lf %.16lf\n", roll2[i], pitch2[i], yaw2[i]);
            fprintf(fp_w2i,  "%.16lf %.16lf %.16lf\n",  w2ix[i],   w2iy[i], w2iz[i]); 
        }
        fclose(fp_t);
        fclose(fp_rmut);
        fclose(fp_rpy1);
        fclose(fp_w1i);
        fclose(fp_rpy2);
        fclose(fp_w2i);

        //Export spacecraft history :
        if (integr.props.spacecraft_checkbox)
        {
            FILE *fp_rsp_helio = fopen((sim_dir + "/spacecraft_pos_helio.txt").c_str(), "w");
            for (size_t i = 0; i < t.size(); ++i)
                fprintf(fp_rsp_helio, "%.16lf %.16lf %.16lf\n", xsp_helio[i]/AU2KM, ysp_helio[i]/AU2KM, zsp_helio[i]/AU2KM); //Deliberatly saved as [AU] in the file.
            fclose(fp_rsp_helio);
        }

        //Export collision status :
        FILE *fp_collision = fopen((sim_dir + "/collision.txt").c_str(),"w");
        fprintf(fp_collision,"Collision detected : %s", (integr.collision_mut || integr.collision_sp) ? "Yes" : "No");
        fclose(fp_collision);

        //Export input properties :
        FILE *fp_props = fopen((sim_dir + "/properties.txt").c_str(),"w");
        fprintf(fp_props,"Simulation name := \"%s\"\n\n", integr.props.sim_name);
        
        if (integr.props.ell_checkbox)
        {
            fprintf(fp_props,"Shape models := \"Ellipsoids\"\n");
            fprintf(fp_props,"    a1 := %.15g\n",  integr.props.semiaxes1[0]);
            fprintf(fp_props,"    b1 := %.15g\n",  integr.props.semiaxes1[1]);
            fprintf(fp_props,"    c1 := %.15g\n",  integr.props.semiaxes1[2]);
            fprintf(fp_props,"    a2 := %.15g\n",  integr.props.semiaxes2[0]);
            fprintf(fp_props,"    b2 := %.15g\n",  integr.props.semiaxes2[1]);
            fprintf(fp_props,"    c2 := %.15g\n\n",integr.props.semiaxes2[2]);
        }
        else
        {
            fprintf(fp_props,"Shape models := \".obj files\"\n");
            fprintf(fp_props,"    file 1 := \"%s\"\n",  integr.props.obj1_path.c_str());
            fprintf(fp_props,"    file 2 := \"%s\"\n\n",integr.props.obj2_path.c_str());
        }

        if (integr.props.ord2_checkbox)
            fprintf(fp_props,"Potential expansion order := 2\n\n");
        else if (integr.props.ord3_checkbox)
            fprintf(fp_props,"Potential expansion order := 3\n\n");
        else
            fprintf(fp_props,"Potential expansion order := 4\n\n");

        fprintf(fp_props,"Masses :\n");
        fprintf(fp_props,"    M1 := %.15g\n",  integr.props.M1);
        fprintf(fp_props,"    M2 := %.15g\n\n",integr.props.M2);

        fprintf(fp_props,"Numerical integration :\n");
        if (integr.props.integration_method == properties::RKF78_FIXED)
        {
            fprintf(fp_props,"    Method := \"RKF78 (fixed)\"\n");
            fprintf(fp_props,"    Epoch := %.15g\n",integr.props.epoch);
            fprintf(fp_props,"    Duration := %.15g\n",integr.props.dur);
            fprintf(fp_props,"    Step := %.15g\n\n",integr.props.step);
        }
        else if (integr.props.integration_method == properties::RKF78_ADAPTIVE)
        {
            fprintf(fp_props,"    Method := \"RKF78 (adaptive)\"\n");
            fprintf(fp_props,"    Epoch := %.15g\n",integr.props.epoch);
            fprintf(fp_props,"    Duration := %.15g\n",integr.props.dur);
            fprintf(fp_props,"    Target error := %.15g\n\n",integr.props.target_error);
        }
        else if (integr.props.integration_method == properties::BSTOER_ADAPTIVE)
        {
            fprintf(fp_props,"    Method := \"BStoer (adaptive)\"\n");
            fprintf(fp_props,"    Epoch := %.15g\n",integr.props.epoch);
            fprintf(fp_props,"    Duration := %.15g\n",integr.props.dur);
            fprintf(fp_props,"    Target error := %.15g\n\n",integr.props.target_error);
        }
        else //properties::ABM5_FIXED
        {
            fprintf(fp_props,"    Method := \"ABM5 (fixed)\"\n");
            fprintf(fp_props,"    Epoch := %.15g\n",integr.props.epoch);
            fprintf(fp_props,"    Duration := %.15g\n",integr.props.dur);
            fprintf(fp_props,"    Step := %.15g\n\n",integr.props.step);
        }

        fprintf(fp_props,"Initial state :\n");
        if (integr.props.pos_vel_mut_var == properties::CARTESIAN_MUT)
        {
            fprintf(fp_props,"    Mutual position and velocity := \"Cartesian\"\n");
            fprintf(fp_props,"    x  := %.15g\n",   integr.props.cart_mut[0]);
            fprintf(fp_props,"    y  := %.15g\n",   integr.props.cart_mut[1]);
            fprintf(fp_props,"    z  := %.15g\n",   integr.props.cart_mut[2]);
            fprintf(fp_props,"    vx := %.15g\n",   integr.props.cart_mut[3]);
            fprintf(fp_props,"    vy := %.15g\n",   integr.props.cart_mut[4]);
            fprintf(fp_props,"    vz := %.15g\n\n", integr.props.cart_mut[5]);
        }
        else //properties::KEPLERIAN_MUT
        {
            fprintf(fp_props,"    Mutual position and velocity := \"Keplerian\"\n");
            fprintf(fp_props,"    a  := %.15g\n",   integr.props.kep_mut[0]);
            fprintf(fp_props,"    e  := %.15g\n",   integr.props.kep_mut[1]);
            fprintf(fp_props,"    i  := %.15g\n",   integr.props.kep_mut[2]);
            fprintf(fp_props,"    Om := %.15g\n",   integr.props.kep_mut[3]);
            fprintf(fp_props,"    w  := %.15g\n",   integr.props.kep_mut[4]);
            fprintf(fp_props,"    M  := %.15g\n\n", integr.props.kep_mut[5]);
        }

        if (integr.props.orient_var == properties::EULER_XYZ)
        {
            fprintf(fp_props,"    Orientations := \"Euler angles (XYZ)\"\n");
            fprintf(fp_props,"    Roll 1  := %.15g\n",   integr.props.rpy1[0]);
            fprintf(fp_props,"    Pitch 1 := %.15g\n",   integr.props.rpy1[1]);
            fprintf(fp_props,"    Yaw 1   := %.15g\n",   integr.props.rpy1[2]);
            fprintf(fp_props,"    Roll 2  := %.15g\n",   integr.props.rpy2[0]);
            fprintf(fp_props,"    Pitch 2 := %.15g\n",   integr.props.rpy2[1]);
            fprintf(fp_props,"    Yaw 2   := %.15g\n\n", integr.props.rpy2[2]);
        }
        else //properties::QUATERNION
        {
            fprintf(fp_props,"    Orientations := \"Quaternions (WXYZ)\"\n");
            fprintf(fp_props,"    q10 := %.15g\n",   integr.props.q1[0]);
            fprintf(fp_props,"    q11 := %.15g\n",   integr.props.q1[1]);
            fprintf(fp_props,"    q12 := %.15g\n",   integr.props.q1[2]);
            fprintf(fp_props,"    q13 := %.15g\n",   integr.props.q1[3]);
            fprintf(fp_props,"    q20 := %.15g\n",   integr.props.q2[0]);
            fprintf(fp_props,"    q21 := %.15g\n",   integr.props.q2[1]);
            fprintf(fp_props,"    q22 := %.15g\n",   integr.props.q2[2]);
            fprintf(fp_props,"    q23 := %.15g\n\n", integr.props.q2[3]);
        }

        if (integr.props.angvel_frame == properties::ANGVEL_HELIO)
        {
            fprintf(fp_props,"    Angular velocities := \"Heliocentric (inertial)\"\n");
            fprintf(fp_props,"    w1x := %.15g\n",   integr.props.w1i[0]);
            fprintf(fp_props,"    w1y := %.15g\n",   integr.props.w1i[1]);
            fprintf(fp_props,"    w1z := %.15g\n",   integr.props.w1i[2]);
            fprintf(fp_props,"    w2x := %.15g\n",   integr.props.w2i[0]);
            fprintf(fp_props,"    w2y := %.15g\n",   integr.props.w2i[1]);
            fprintf(fp_props,"    w2z := %.15g\n\n", integr.props.w2i[2]);
        }
        else //properties::ANGVEL_BODY
        {
            fprintf(fp_props,"    Angular velocities := \"Body frames\"\n");
            fprintf(fp_props,"    w11 := %.15g\n",   integr.props.w1b[0]);
            fprintf(fp_props,"    w12 := %.15g\n",   integr.props.w1b[1]);
            fprintf(fp_props,"    w13 := %.15g\n",   integr.props.w1b[2]);
            fprintf(fp_props,"    w21 := %.15g\n",   integr.props.w2b[0]);
            fprintf(fp_props,"    w22 := %.15g\n",   integr.props.w2b[1]);
            fprintf(fp_props,"    w23 := %.15g\n\n", integr.props.w2b[2]);
        }

        if (integr.props.pos_vel_com_var == properties::CARTESIAN_COM_HELIO)
        {
            fprintf(fp_props,"    Binary COM (Heliocentric) := \"Cartesian\"\n");
            //Remember the gui property cart_com_helio[0-2] was in [AU], but the copy was converted in [km] in integrator::prepare().
            //We want to save in the file the [AU] version to match with the user input.
            fprintf(fp_props,"    x  := %.15g\n",   integr.props.cart_com_helio[0]/AU2KM);
            fprintf(fp_props,"    y  := %.15g\n",   integr.props.cart_com_helio[1]/AU2KM);
            fprintf(fp_props,"    z  := %.15g\n",   integr.props.cart_com_helio[2]/AU2KM);
            fprintf(fp_props,"    vx := %.15g\n",   integr.props.cart_com_helio[3]);
            fprintf(fp_props,"    vy := %.15g\n",   integr.props.cart_com_helio[4]);
            fprintf(fp_props,"    vz := %.15g\n\n", integr.props.cart_com_helio[5]);
        }
        else //properties::KEPLERIAN_COM_HELIO
        {
            fprintf(fp_props,"    Binary COM (Heliocentric) := \"Keplerian\"\n");
            fprintf(fp_props,"    a  := %.15g\n",   integr.props.kep_com_helio[0]);
            fprintf(fp_props,"    e  := %.15g\n",   integr.props.kep_com_helio[1]);
            fprintf(fp_props,"    i  := %.15g\n",   integr.props.kep_com_helio[2]);
            fprintf(fp_props,"    Om := %.15g\n",   integr.props.kep_com_helio[3]);
            fprintf(fp_props,"    w  := %.15g\n",   integr.props.kep_com_helio[4]);
            fprintf(fp_props,"    M  := %.15g\n\n", integr.props.kep_com_helio[5]);
        }

        if (integr.props.sun_gravity)
            fprintf(fp_props,"Assume Sun's gravity := \"Yes\"\n");
        else
            fprintf(fp_props,"Assume Sun's gravity := \"No\"\n");

        if (integr.props.collision_no)
            fprintf(fp_props,"Collision shapes := \"No collision\"\n\n");
        else if (integr.props.collision_spheres)
            fprintf(fp_props,"Collision shapes := \"Spheres\"\n\n");
        else
            fprintf(fp_props,"Collision shapes := \"Polyhedra\"\n\n");

        if (integr.props.impactors_checkbox)
        {
            fprintf(fp_props,"Assume kinetic impactors := \"Yes\"\n");
            fprintf(fp_props,"    Body 1 :\n");
            fprintf(fp_props,"        Mass (dry + fuel) :\n");
            fprintf(fp_props,"            m := %.15g\n", integr.props.mD1);
            fprintf(fp_props,"        Velocity (relative to body) :\n");
            fprintf(fp_props,"            vx := %.15g\n", integr.props.vD1[0]);
            fprintf(fp_props,"            vy := %.15g\n", integr.props.vD1[1]);
            fprintf(fp_props,"            vz := %.15g\n", integr.props.vD1[2]);
            fprintf(fp_props,"        Momentum enhancement factor (ejecta) :\n");
            fprintf(fp_props,"            beta := %.15g\n", integr.props.beta1);
            fprintf(fp_props,"        Impact epoch :\n");
            fprintf(fp_props,"            t := %.15g\n", integr.props.tD1/86400.0);
            fprintf(fp_props,"    Body 2 :\n");
            fprintf(fp_props,"        Mass (dry + fuel) :\n");
            fprintf(fp_props,"            m := %.15g\n", integr.props.mD2);
            fprintf(fp_props,"        Velocity (relative to body) :\n");
            fprintf(fp_props,"            vx := %.15g\n", integr.props.vD2[0]);
            fprintf(fp_props,"            vy := %.15g\n", integr.props.vD2[1]);
            fprintf(fp_props,"            vz := %.15g\n", integr.props.vD2[2]);
            fprintf(fp_props,"        Momentum enhancement factor (ejecta) :\n");
            fprintf(fp_props,"            beta := %.15g\n", integr.props.beta2);
            fprintf(fp_props,"        Impact epoch :\n");
            fprintf(fp_props,"            t := %.15g\n\n", integr.props.tD2/86400.0);
        }
        else
            fprintf(fp_props, "Assume kinetic impactors := \"No\"\n\n");

        if (integr.props.spacecraft_checkbox)
        {
            fprintf(fp_props,"Assume spacecraft orbiter := \"Yes\"\n");
            if (integr.props.pos_vel_sp_var == properties::CARTESIAN_SP_COM)
            {
                fprintf(fp_props,"    Position and velocity := \"Cartesian (binary COM)\"\n");
                fprintf(fp_props,"        x  := %.15g\n",   integr.props.cart_sp_com[0]);
                fprintf(fp_props,"        y  := %.15g\n",   integr.props.cart_sp_com[1]);
                fprintf(fp_props,"        z  := %.15g\n",   integr.props.cart_sp_com[2]);
                fprintf(fp_props,"        vx := %.15g\n",   integr.props.cart_sp_com[3]);
                fprintf(fp_props,"        vy := %.15g\n",   integr.props.cart_sp_com[4]);
                fprintf(fp_props,"        vz := %.15g\n\n", integr.props.cart_sp_com[5]);
            }
            else if (integr.props.pos_vel_sp_var == properties::CARTESIAN_SP_COM1)
            {
                fprintf(fp_props,"    Position and velocity := \"Cartesian (body 1)\"\n");
                fprintf(fp_props,"        x  := %.15g\n",   integr.props.cart_sp_com1[0]);
                fprintf(fp_props,"        y  := %.15g\n",   integr.props.cart_sp_com1[1]);
                fprintf(fp_props,"        z  := %.15g\n",   integr.props.cart_sp_com1[2]);
                fprintf(fp_props,"        vx := %.15g\n",   integr.props.cart_sp_com1[3]);
                fprintf(fp_props,"        vy := %.15g\n",   integr.props.cart_sp_com1[4]);
                fprintf(fp_props,"        vz := %.15g\n\n", integr.props.cart_sp_com1[5]);
            }
            else if (integr.props.pos_vel_sp_var == properties::CARTESIAN_SP_COM2)
            {
                fprintf(fp_props,"    Position and velocity := \"Cartesian (body 2)\"\n");
                fprintf(fp_props,"        x  := %.15g\n",   integr.props.cart_sp_com2[0]);
                fprintf(fp_props,"        y  := %.15g\n",   integr.props.cart_sp_com2[1]);
                fprintf(fp_props,"        z  := %.15g\n",   integr.props.cart_sp_com2[2]);
                fprintf(fp_props,"        vx := %.15g\n",   integr.props.cart_sp_com2[3]);
                fprintf(fp_props,"        vy := %.15g\n",   integr.props.cart_sp_com2[4]);
                fprintf(fp_props,"        vz := %.15g\n\n", integr.props.cart_sp_com2[5]);
            }
            else if (integr.props.pos_vel_sp_var == properties::KEPLERIAN_SP_COM)
            {
                fprintf(fp_props,"    Position and velocity := \"Keplerian (binary COM)\"\n");
                fprintf(fp_props,"        a  := %.15g\n",   integr.props.kep_sp_com[0]);
                fprintf(fp_props,"        e  := %.15g\n",   integr.props.kep_sp_com[1]);
                fprintf(fp_props,"        i  := %.15g\n",   integr.props.kep_sp_com[2]);
                fprintf(fp_props,"        Om := %.15g\n",   integr.props.kep_sp_com[3]);
                fprintf(fp_props,"        w  := %.15g\n",   integr.props.kep_sp_com[4]);
                fprintf(fp_props,"        M  := %.15g\n\n", integr.props.kep_sp_com[5]);
            }
            else if (integr.props.pos_vel_sp_var == properties::KEPLERIAN_SP_COM1)
            {
                fprintf(fp_props,"    Position and velocity := \"Keplerian (body 1)\"\n");
                fprintf(fp_props,"        a  := %.15g\n",   integr.props.kep_sp_com1[0]);
                fprintf(fp_props,"        e  := %.15g\n",   integr.props.kep_sp_com1[1]);
                fprintf(fp_props,"        i  := %.15g\n",   integr.props.kep_sp_com1[2]);
                fprintf(fp_props,"        Om := %.15g\n",   integr.props.kep_sp_com1[3]);
                fprintf(fp_props,"        w  := %.15g\n",   integr.props.kep_sp_com1[4]);
                fprintf(fp_props,"        M  := %.15g\n\n", integr.props.kep_sp_com1[5]);
            }
            else if (integr.props.pos_vel_sp_var == properties::KEPLERIAN_SP_COM2)
            {
                fprintf(fp_props,"    Position and velocity := \"Keplerian (body 2)\"\n");
                fprintf(fp_props,"        a  := %.15g\n",   integr.props.kep_sp_com2[0]);
                fprintf(fp_props,"        e  := %.15g\n",   integr.props.kep_sp_com2[1]);
                fprintf(fp_props,"        i  := %.15g\n",   integr.props.kep_sp_com2[2]);
                fprintf(fp_props,"        Om := %.15g\n",   integr.props.kep_sp_com2[3]);
                fprintf(fp_props,"        w  := %.15g\n",   integr.props.kep_sp_com2[4]);
                fprintf(fp_props,"        M  := %.15g\n\n", integr.props.kep_sp_com2[5]);
            }

            if (integr.props.srp_checkbox)
            {
                fprintf(fp_props,"    Account for SRP := \"Yes\"\n");
                fprintf(fp_props,"        SRP parameters : \n");
                fprintf(fp_props,"            ρ  := %.15g\n", integr.props.sp_refl);
                fprintf(fp_props,"            A  := %.15g\n", integr.props.sp_area);
                fprintf(fp_props,"            m  := %.15g\n", integr.props.sp_mass);
                if (integr.props.srp_shadow_checkbox)
                    fprintf(fp_props,"    Account for shadows := \"Yes\"\n");
                else
                    fprintf(fp_props,"    Account for shadows := \"No\"\n");
            }
            else
                fprintf(fp_props,"    Account for SRP := \"No\"\n");
        }
        else
            fprintf(fp_props,"Assume spacecraft orbiter := \"No\"\n\n");
        
        fclose(fp_props);

        cons.print("Done.\n");
    }
};

#endif
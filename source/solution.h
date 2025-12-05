#ifndef SOLUTION_H
#define SOLUTION_H

#include<cstdio>
#include<cmath>
#include<cstring>
#include<string>
#include<algorithm>

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

    //The following members are exactly the same (copies) with what the integrator evaluated, but stored in 1-D vectors.
    dvec t;
    dvec x, y, z;
    dvec vx, vy, vz;
    dvec w1bx, w1by, w1bz;
    dvec w2bx, w2by, w2bz;
    dvec xsp, ysp, zsp;

    //The following members were NOT directly evaluated by the integrator. Instead, we use what the integrator evaluated to evaluate them implicitly.
    dvec dist, vel; //Binary's mutual distance and mutual velocity magnitude.
    dvec roll1, pitch1, yaw1, relyaw1; 
    dvec roll2, pitch2, yaw2, relyaw2;
    dvec w1ix, w1iy, w1iz;
    dvec w2ix, w2iy, w2iz;
    dvec sma, ecc, inc, raan, argper, manom;
    dvec denergy, dmomentum; //Relative errors, i.e. |(E[i] - E[0])/E[0]| and |(L[i] - L[0])/L[0]|
    dvec d1sp, d2sp;

    solution() { } //This is needed in the scene_panel class.

    solution(const integrator &integr) //And this is needed in the gui class (deep copy of the integrator).
    {
        this->integr = integr;
    }

    //This function fills with orbital data all the vector members.
    void construct(console_panel &console)
    {
        console.add_timed_text("[Solution] : Constructing solution... ");

        const size_t N = integr.orbit.size();
        
        t.resize(N);
        
        x.resize(N);     y.resize(N);     z.resize(N);
        vx.resize(N);    vy.resize(N);    vz.resize(N);
        w1bx.resize(N);  w1by.resize(N);  w1bz.resize(N);
        w2bx.resize(N);  w2by.resize(N);  w2bz.resize(N);
        xsp.resize(N);   ysp.resize(N);   zsp.resize(N);

        dist.resize(N);    vel.resize(N);
        roll1.resize(N);   pitch1.resize(N);    yaw1.resize(N),  relyaw1.resize(N);
        roll2.resize(N);   pitch2.resize(N);    yaw2.resize(N),  relyaw2.resize(N);
        w1ix.resize(N);    w1iy.resize(N);      w1iz.resize(N);
        w2ix.resize(N);    w2iy.resize(N);      w2iz.resize(N);
        sma.resize(N);     ecc.resize(N);       inc.resize(N);   raan.resize(N); argper.resize(N); manom.resize(N);
        denergy.resize(N); dmomentum.resize(N);

        d1sp.resize(N); d2sp.resize(N);

        double energy0, momentum0;
        for (size_t i = 0; i < N; ++i)
        {
            //Extract the integr.orbit[][] matrix into temporary variables for readability.
            //Remember : integr.orbit contains (t, x,y,z, vx,vy,vz, q10,q11,q12,q13, w1bx,w1by,w1bz, q20,q21,q22,q23, w2bx,w2by,w2bz, xsp,ysp,zsp) at each line i.
            dvec3  r    = {integr.orbit[i][1],  integr.orbit[i][2],  integr.orbit[i][3]};
            dvec3  v    = {integr.orbit[i][4],  integr.orbit[i][5],  integr.orbit[i][6]};
            dvec4  q1   = {integr.orbit[i][7],  integr.orbit[i][8],  integr.orbit[i][9],  integr.orbit[i][10]};
            dvec3  w1b  = {integr.orbit[i][11], integr.orbit[i][12], integr.orbit[i][13]};
            dvec4  q2   = {integr.orbit[i][14], integr.orbit[i][15], integr.orbit[i][16], integr.orbit[i][17]};
            dvec3  w2b  = {integr.orbit[i][18], integr.orbit[i][19], integr.orbit[i][20]};
            dvec3  rsp  = {integr.orbit[i][21], integr.orbit[i][22], integr.orbit[i][23]};

            dmat3 A1   = quat2mat(q1);
            dmat3 A2   = quat2mat(q2);
            dvec3 w1i  = body2iner(w1b,A1);
            dvec3 w2i  = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);

            dvec3 rcyl = cart2cyl(r);
            
            //Calculate libration angle (relative yaw) of body 1 & 2 :
            double temp = rpy1[2] - rcyl[1]; //phi1 = thita1z - thita
            while (temp > PI)
                temp -= 2.0*PI;
            while (temp <= -PI)
                temp += 2.0*PI;
            double libration1 = temp;
            temp = rpy2[2] - rcyl[1];  //phi2 = thita2z - thita
            while (temp > PI)
                temp -= 2.0*PI;
            while (temp <= -PI)
                temp += 2.0*PI;
            double libration2 = temp;

            dvec6 kep  = cart2kep({r[0],r[1],r[2], v[0],v[1],v[2]}, G*(integr.properties.M1 + integr.properties.M2));
            
            //Kinetic energy part (evaluated in body frames - that's ok coz energy is scalar and scalars are preserved under rotations).
            double energy = 0.5*integr.m*dot(v,v) + 0.5*dot( dot(w1b, integr.I1), w1b) + 0.5*dot( dot(w2b, integr.I2), w2b);

            //Potential energy part.
            if (integr.properties.ord2_checkbox)
                energy += mut_pot_integrals_ord2(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else if (integr.properties.ord3_checkbox)
                energy += mut_pot_integrals_ord3(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else
                energy += mut_pot_integrals_ord4(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            
            //Momentum magnitude (now we must evaluate it in the C.O.M. frame because this is a vector).
            //Note : All 3 components of the momentum vector are conserved in time, but I just choose to store and plot the magnitude only.
            double momentum = length( integr.m*cross(r,v) + dot(A1, dot(integr.I1, w1b)) + dot(A2, dot(integr.I2, w2b)) );

            t[i] = integr.orbit[i][0]/86400.0; //[days]

            x[i] = r[0];
            y[i] = r[1];
            z[i] = r[2];

            vx[i] = v[0];
            vy[i] = v[1];
            vz[i] = v[2];

            w1bx[i] = w1b[0];
            w1by[i] = w1b[1];
            w1bz[i] = w1b[2];

            w2bx[i] = w2b[0];
            w2by[i] = w2b[1];
            w2bz[i] = w2b[2];

            xsp[i] = rsp[0];
            ysp[i] = rsp[1];
            zsp[i] = rsp[2];

            d1sp[i] = length(rsp - integr.m1*r);
            d2sp[i] = length(rsp - integr.m2*r);

            dist[i] = rcyl[0];
            vel[i]  = length(v);

            roll1[i]   = rpy1[0]*180.0/PI;
            pitch1[i]  = rpy1[1]*180.0/PI;
            yaw1[i]    = rpy1[2]*180.0/PI;
            relyaw1[i] = libration1*180.0/PI;

            roll2[i]   = rpy2[0]*180.0/PI;
            pitch2[i]  = rpy2[1]*180.0/PI;
            yaw2[i]    = rpy2[2]*180.0/PI;
            relyaw2[i] = libration2*180.0/PI;

            w1ix[i] = w1i[0];
            w1iy[i] = w1i[1];
            w1iz[i] = w1i[2];

            w2ix[i] = w2i[0];
            w2iy[i] = w2i[1];
            w2iz[i] = w2i[2];

            sma[i]    = kep[0];
            ecc[i]    = kep[1];
            inc[i]    = kep[2]*180.0/PI;
            raan[i]   = kep[3]*180.0/PI;
            argper[i] = kep[4]*180.0/PI;
            manom[i]  = kep[5]*180.0/PI;

            if (i == 0)
            {
                energy0 = energy;
                momentum0 = momentum;
                denergy[0] = dmomentum[0] = 0.0; //No error initially by default...
            }
            else
            {
                //Calculate the relative energy error if energy0 != 0, otherwise calculate the absolute error to avoid division by zero.
                if (fabs(energy0) > 1e-16)
                    denergy[i] = fabs((energy - energy0)/energy0);
                else
                    denergy[i] = fabs(energy - energy0);
            
                //Same here with momentum.
                if (fabs(momentum0) > 1e-16)
                    dmomentum[i] = fabs((momentum - momentum0)/momentum0);
                else
                    dmomentum[i] = fabs(momentum - momentum0);
            }
        }
        integr.orbit.clear();
        integr.orbit.shrink_to_fit();
        
        console.add_text("Done.\n");
    }

    void reduce(const dvec &original, dvec &reduced)
    {
        reduced.resize(SOL2D_SIZE);
        const size_t N = original.size();
        double step = (N - 1.0)/(SOL2D_SIZE - 1.0); 
        for (size_t i = 0; i < SOL2D_SIZE; ++i)
        {
            size_t j = (size_t)(i*step);
            if (j >= N)
                j = N-1;
            reduced[i] = original[j];
        }
    }

    //Create a reduced version of the solution.
    solution get_reduced_solution()
    {
        solution sol2D;

        sol2D.integr = integr; //Copy integrator's members (cheap because the expensive 'orbit' member is already freed).

        if (SOL2D_SIZE == 0 || t.size() == 0)
            return sol2D;

        if (t.size() <= SOL2D_SIZE)
            return *this;

        reduce(t,            sol2D.t);
        reduce(x,            sol2D.x);
        reduce(y,            sol2D.y);
        reduce(z,            sol2D.z);
        reduce(vx,           sol2D.vx);
        reduce(vy,           sol2D.vy);
        reduce(vz,           sol2D.vz);
        reduce(w1bx,         sol2D.w1bx);
        reduce(w1by,         sol2D.w1by);
        reduce(w1bz,         sol2D.w1bz);
        reduce(w2bx,         sol2D.w2bx);
        reduce(w2by,         sol2D.w2by);
        reduce(w2bz,         sol2D.w2bz);
        reduce(xsp,          sol2D.xsp);
        reduce(ysp,          sol2D.ysp);
        reduce(zsp,          sol2D.zsp);
        reduce(d1sp,         sol2D.d1sp);
        reduce(d2sp,         sol2D.d2sp);
        
        reduce(dist,         sol2D.dist);
        reduce(vel,          sol2D.vel);
        reduce(roll1,        sol2D.roll1);
        reduce(pitch1,       sol2D.pitch1);
        reduce(yaw1,         sol2D.yaw1);
        reduce(relyaw1,      sol2D.relyaw1);
        reduce(roll2,        sol2D.roll2);
        reduce(pitch2,       sol2D.pitch2);
        reduce(yaw2,         sol2D.yaw2);
        reduce(relyaw2,      sol2D.relyaw2);
        reduce(w1ix,         sol2D.w1ix);
        reduce(w1iy,         sol2D.w1iy);
        reduce(w1iz,         sol2D.w1iz);
        reduce(w2ix,         sol2D.w2ix);
        reduce(w2iy,         sol2D.w2iy);
        reduce(w2iz,         sol2D.w2iz);
        reduce(sma,          sol2D.sma);
        reduce(ecc,          sol2D.ecc);
        reduce(inc,          sol2D.inc);
        reduce(raan,         sol2D.raan);
        reduce(argper,       sol2D.argper);
        reduce(manom,        sol2D.manom);
        reduce(denergy,      sol2D.denergy);
        reduce(dmomentum,    sol2D.dmomentum);
        
        return sol2D;
    }

    void export_files(console_panel &console)
    {
        console.add_timed_text("[Solution] : Exporting solution files... ");

        std::string sim_dir = std::string(SIM_DIR_ROOT) + integr.properties.sim_name;
        std::filesystem::create_directory(SIM_DIR_ROOT); //Create the root directory that shall host all simulations if it does not exist already.
        std::filesystem::create_directory(sim_dir); //Create the current simulation directory that shall host all solution files.

        //Create the .txt contents :
        FILE *fp_t        = fopen((sim_dir + "/time.txt"              ).c_str(), "w");
        FILE *fp_pos      = fopen((sim_dir + "/rel_pos.txt"           ).c_str(), "w");
        FILE *fp_vel      = fopen((sim_dir + "/rel_vel.txt"           ).c_str(), "w");
        FILE *fp_w1b      = fopen((sim_dir + "/ang_vel_w1b.txt"       ).c_str(), "w");
        FILE *fp_w2b      = fopen((sim_dir + "/ang_vel_w2b.txt"       ).c_str(), "w");
        FILE *fp_sp       = fopen((sim_dir + "/spacecraft.txt"        ).c_str(), "w");

        FILE *fp_rpy1     = fopen((sim_dir + "/euler_rpy1.txt"        ).c_str(), "w");
        FILE *fp_w1i      = fopen((sim_dir + "/ang_vel_w1i.txt"       ).c_str(), "w");
        FILE *fp_rpy2     = fopen((sim_dir + "/euler_rpy2.txt"        ).c_str(), "w");
        FILE *fp_w2i      = fopen((sim_dir + "/ang_vel_w2i.txt"       ).c_str(), "w");
        FILE *fp_kep      = fopen((sim_dir + "/keplerian.txt"         ).c_str(), "w");
        FILE *fp_ener_mom = fopen((sim_dir + "/ener_mom_rel_error.txt").c_str(), "w");

        //Export the solution vectors.
        for (size_t i = 0; i < t.size(); ++i)
        {
            fprintf(fp_t,        "%.16lf\n", t[i]);
            fprintf(fp_pos,      "%.16lf %.16lf %.16lf %.16lf\n",                 x[i],    y[i],    z[i], dist[i]);
            fprintf(fp_vel,      "%.16lf %.16lf %.16lf %.16lf\n",                vx[i],   vy[i],   vz[i],  vel[i]);
            fprintf(fp_w1b,      "%.16lf %.16lf %.16lf\n",                     w1bx[i], w1by[i], w1bz[i]);
            fprintf(fp_w2b,      "%.16lf %.16lf %.16lf\n",                     w2bx[i], w2by[i], w2bz[i]);
            fprintf(fp_sp,       "%.16lf %.16lf %.16lf\n",                      xsp[i],  ysp[i],  zsp[i]);

            fprintf(fp_rpy1,     "%.16lf %.16lf %.16lf\n",                      roll1[i], pitch1[i], yaw1[i]);
            fprintf(fp_w1i,      "%.16lf %.16lf %.16lf\n",                       w1ix[i],   w1iy[i], w1iz[i]);
            fprintf(fp_rpy2,     "%.16lf %.16lf %.16lf\n",                      roll2[i], pitch2[i], yaw2[i]);
            fprintf(fp_w2i,      "%.16lf %.16lf %.16lf\n",                       w2ix[i],   w2iy[i], w2iz[i]);
            fprintf(fp_kep,      "%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",   sma[i],    ecc[i],  inc[i], raan[i], argper[i], manom[i]); 
            fprintf(fp_ener_mom, "%.16lf %.16lf\n",                           denergy[i],       dmomentum[i]);
        }

        fclose(fp_t);
        fclose(fp_pos);
        fclose(fp_vel);
        fclose(fp_w1b);
        fclose(fp_w2b);
        fclose(fp_sp);

        fclose(fp_rpy1);
        fclose(fp_w1i);
        fclose(fp_rpy2);
        fclose(fp_w2i);
        fclose(fp_kep);
        fclose(fp_ener_mom);

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

        fprintf(fp_properties,"    C.O.M. motion relative to world :\n");
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
            fprintf(fp_properties,"    Initial position (relative to C.O.M.) :\n");
            fprintf(fp_properties,"        x  := %.15g\n",   integr.properties.rsp[0]);
            fprintf(fp_properties,"        y  := %.15g\n",   integr.properties.rsp[1]);
            fprintf(fp_properties,"        z  := %.15g\n",   integr.properties.rsp[2]);
            fprintf(fp_properties,"    Initial velocity (relative to C.O.M.) :\n");
            fprintf(fp_properties,"        vx := %.15g\n",   integr.properties.vsp[0]);
            fprintf(fp_properties,"        vy := %.15g\n",   integr.properties.vsp[1]);
            fprintf(fp_properties,"        vz := %.15g\n\n", integr.properties.vsp[2]);

            if (integr.properties.srp_checkbox)
            {
                fprintf(fp_properties,"    Account for SRP := \"Yes\"\n");
                fprintf(fp_properties,"        SRP parameters : \n");
                fprintf(fp_properties,"            ρ  := %.15g\n", integr.properties.sp_refl);
                fprintf(fp_properties,"            A  := %.15g\n", integr.properties.sp_area);
                fprintf(fp_properties,"            m  := %.15g\n", integr.properties.sp_mass);
                fprintf(fp_properties,"        Sun's position (relative to C.O.M.) : \n");
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
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

    dvec t;

    //Mutual :
    dvec x, y, z, dist;
    dvec vx, vy, vz, vel;
    dvec sma, ecc, inc, raan, argper, manom;
    dvec denergy, dmomentum; //Relative errors, i.e. |(E[i] - E[0])/E[0]| and |(L[i] - L[0])/L[0]|

    //Body 1 :
    dvec roll1, pitch1, yaw1, relyaw1;
    dvec w1ix, w1iy, w1iz;
    dvec w1bx, w1by, w1bz;

    //Body 2 :
    dvec roll2, pitch2, yaw2, relyaw2;
    dvec w2ix, w2iy, w2iz;
    dvec w2bx, w2by, w2bz;

    //Spacecraft :
    //In binary's COM frame.
    dvec xsp,  ysp,  zsp,  dist_sp;
    dvec vxsp, vysp, vzsp, vel_sp;
    dvec sma_sp, ecc_sp, inc_sp, raan_sp, argper_sp, manom_sp;
    //In body 1 frame.
    dvec xsp1,  ysp1,  zsp1,  dist_sp1;
    dvec vxsp1, vysp1, vzsp1, vel_sp1;
    dvec sma_sp1, ecc_sp1, inc_sp1, raan_sp1, argper_sp1, manom_sp1;
    //In body 2 frame.
    dvec xsp2,  ysp2,  zsp2,  dist_sp2;
    dvec vxsp2, vysp2, vzsp2, vel_sp2;
    dvec sma_sp2, ecc_sp2, inc_sp2, raan_sp2, argper_sp2, manom_sp2;

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
        
        //Mutual :
        x.resize(N);       y.resize(N);       z.resize(N);       dist.resize(N);
        vx.resize(N);      vy.resize(N);      vz.resize(N);      vel.resize(N);
        sma.resize(N);     ecc.resize(N);     inc.resize(N);     raan.resize(N);  argper.resize(N);  manom.resize(N);
        denergy.resize(N); dmomentum.resize(N);
        
        //Body 1 :
        roll1.resize(N);  pitch1.resize(N);  yaw1.resize(N);  relyaw1.resize(N);
        w1ix.resize(N);   w1iy.resize(N);    w1iz.resize(N);
        w1bx.resize(N);   w1by.resize(N);    w1bz.resize(N);

        //Body 2 :
        roll2.resize(N);  pitch2.resize(N);  yaw2.resize(N),  relyaw2.resize(N);       
        w2ix.resize(N);   w2iy.resize(N);    w2iz.resize(N);
        w2bx.resize(N);   w2by.resize(N);    w2bz.resize(N);
        
        //Spacecraft :
        if (integr.properties.spacecraft_checkbox)
        {
            xsp.resize(N);    ysp.resize(N);    zsp.resize(N);    dist_sp.resize(N);
            vxsp.resize(N);   vysp.resize(N);   vzsp.resize(N);   vel_sp.resize(N);
            sma_sp.resize(N); ecc_sp.resize(N); inc_sp.resize(N); raan_sp.resize(N); argper_sp.resize(N); manom_sp.resize(N);

            xsp1.resize(N);    ysp1.resize(N);    zsp1.resize(N);    dist_sp1.resize(N);
            vxsp1.resize(N);   vysp1.resize(N);   vzsp1.resize(N);   vel_sp1.resize(N);
            sma_sp1.resize(N); ecc_sp1.resize(N); inc_sp1.resize(N); raan_sp1.resize(N); argper_sp1.resize(N); manom_sp1.resize(N);

            xsp2.resize(N);    ysp2.resize(N);    zsp2.resize(N);    dist_sp2.resize(N);
            vxsp2.resize(N);   vysp2.resize(N);   vzsp2.resize(N);   vel_sp2.resize(N);
            sma_sp2.resize(N); ecc_sp2.resize(N); inc_sp2.resize(N); raan_sp2.resize(N); argper_sp2.resize(N); manom_sp2.resize(N);
        } //Otherwise, there's no reason to occupy memory for the spacecraft solution...

        double energy0, momentum0;
        for (size_t i = 0; i < N; ++i)
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

            dvec6 kep = cart2kep({r[0],r[1],r[2], v[0],v[1],v[2]}, G*(integr.properties.M1 + integr.properties.M2));
            
            //Kinetic energy part (evaluated in body frames - that's ok coz energy is scalar and scalars are preserved under rotations) :
            double energy = 0.5*integr.m*dot(v,v) + 0.5*dot( dot(w1b, integr.I1), w1b) + 0.5*dot( dot(w2b, integr.I2), w2b);

            //Potential energy part :
            if (integr.properties.ord2_checkbox)
                energy += mut_pot_integrals_ord2(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else if (integr.properties.ord3_checkbox)
                energy += mut_pot_integrals_ord3(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else
                energy += mut_pot_integrals_ord4(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            
            //Momentum magnitude (now evaluated it in the C.O.M. frame because it is a vector) :
            //Note : All 3 components of the momentum vector are conserved in time, but I just choose to store and plot the magnitude only.
            double momentum = length( integr.m*cross(r,v) + dot(A1, dot(integr.I1, w1b)) + dot(A2, dot(integr.I2, w2b)) );

            t[i] = integr.orbit[i][0]/86400.0; //[days]

            //Mutual :
            x[i]    = r[0];
            y[i]    = r[1];
            z[i]    = r[2];
            dist[i] = rcyl[0];

            vx[i]   = v[0];
            vy[i]   = v[1];
            vz[i]   = v[2];
            vel[i]  = length(v);

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

            //Body 1 :
            roll1[i]   = rpy1[0]*180.0/PI;
            pitch1[i]  = rpy1[1]*180.0/PI;
            yaw1[i]    = rpy1[2]*180.0/PI;
            relyaw1[i] = libration1*180.0/PI;

            w1ix[i] = w1i[0];
            w1iy[i] = w1i[1];
            w1iz[i] = w1i[2];

            w1bx[i] = w1b[0];
            w1by[i] = w1b[1];
            w1bz[i] = w1b[2];

            //Body 2 :
            roll2[i]   = rpy2[0]*180.0/PI;
            pitch2[i]  = rpy2[1]*180.0/PI;
            yaw2[i]    = rpy2[2]*180.0/PI;
            relyaw2[i] = libration2*180.0/PI;

            w2ix[i] = w2i[0];
            w2iy[i] = w2i[1];
            w2iz[i] = w2i[2];

            w2bx[i] = w2b[0];
            w2by[i] = w2b[1];
            w2bz[i] = w2b[2];

            //Spacecraft :
            if (integr.properties.spacecraft_checkbox)
            {
                dvec3 rsp = {integr.orbit[i][21], integr.orbit[i][22], integr.orbit[i][23]};
                dvec3 vsp = {integr.orbit[i][24], integr.orbit[i][25], integr.orbit[i][26]};

                dvec3 rsp1 = rsp - integr.m1*r, vsp1 = vsp - integr.m1*v;
                dvec3 rsp2 = rsp - integr.m2*r, vsp2 = vsp - integr.m2*v;
                dvec6 kep_sp  = cart2kep({ rsp[0],  rsp[1],  rsp[2],   vsp[0],  vsp[1],  vsp[2]}, G*(integr.properties.M1 + integr.properties.M2));
                dvec6 kep_sp1 = cart2kep({rsp1[0], rsp1[1], rsp1[2],  vsp1[0], vsp1[1], vsp1[2]}, G*integr.properties.M1);
                dvec6 kep_sp2 = cart2kep({rsp2[0], rsp2[1], rsp2[2],  vsp2[0], vsp2[1], vsp2[2]}, G*integr.properties.M2);

                xsp[i]     = rsp[0];
                ysp[i]     = rsp[1];
                zsp[i]     = rsp[2];
                dist_sp[i] = length(rsp);

                vxsp[i]   = vsp[0];
                vysp[i]   = vsp[1];
                vzsp[i]   = vsp[2];
                vel_sp[i] = length(vsp);

                sma_sp[i]    = kep_sp[0];
                ecc_sp[i]    = kep_sp[1];
                inc_sp[i]    = kep_sp[2]*180.0/PI;
                raan_sp[i]   = kep_sp[3]*180.0/PI;
                argper_sp[i] = kep_sp[4]*180.0/PI;
                manom_sp[i]  = kep_sp[5]*180.0/PI;

                ///////////////////////////////////////////////////////////

                xsp1[i]     = rsp1[0];
                ysp1[i]     = rsp1[1];
                zsp1[i]     = rsp1[2];
                dist_sp1[i] = length(rsp1);

                vxsp1[i]   = vsp1[0];
                vysp1[i]   = vsp1[1];
                vzsp1[i]   = vsp1[2];
                vel_sp1[i] = length(vsp1);

                sma_sp1[i]    = kep_sp1[0];
                ecc_sp1[i]    = kep_sp1[1];
                inc_sp1[i]    = kep_sp1[2]*180.0/PI;
                raan_sp1[i]   = kep_sp1[3]*180.0/PI;
                argper_sp1[i] = kep_sp1[4]*180.0/PI;
                manom_sp1[i]  = kep_sp1[5]*180.0/PI;

                ///////////////////////////////////////////////////////////

                xsp2[i]     = rsp2[0];
                ysp2[i]     = rsp2[1];
                zsp2[i]     = rsp2[2];
                dist_sp2[i] = length(rsp2);

                vxsp2[i]   = vsp2[0];
                vysp2[i]   = vsp2[1];
                vzsp2[i]   = vsp2[2];
                vel_sp2[i] = length(vsp2);

                sma_sp2[i]    = kep_sp2[0];
                ecc_sp2[i]    = kep_sp2[1];
                inc_sp2[i]    = kep_sp2[2]*180.0/PI;
                raan_sp2[i]   = kep_sp2[3]*180.0/PI;
                argper_sp2[i] = kep_sp2[4]*180.0/PI;
                manom_sp2[i]  = kep_sp2[5]*180.0/PI;
            }
        }
        integr.orbit.clear();
        integr.orbit.shrink_to_fit();
        
        console.add_text("Done.\n");
    }

    void reduce(const dvec &original, dvec &reduced)
    {
        reduced.resize(N_SOL2D);
        const size_t N = original.size();
        double step = (N - 1.0)/(N_SOL2D - 1.0); 
        for (size_t i = 0; i < N_SOL2D; ++i)
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
        sol2D.integr = integr; //Copy integrator's members (cheap because the expensive 'orbit' member is freed at the end of construct()).

        if (N_SOL2D == 0 || t.size() == 0)
            return sol2D;

        if (t.size() <= N_SOL2D)
            return *this;

        reduce(t,          sol2D.t);
        reduce(x,          sol2D.x);
        reduce(y,          sol2D.y);
        reduce(z,          sol2D.z);
        reduce(dist,       sol2D.dist);
        reduce(vx,         sol2D.vx);
        reduce(vy,         sol2D.vy);
        reduce(vz,         sol2D.vz);
        reduce(vel,        sol2D.vel);
        reduce(sma,        sol2D.sma);
        reduce(ecc,        sol2D.ecc);
        reduce(inc,        sol2D.inc);
        reduce(raan,       sol2D.raan);
        reduce(argper,     sol2D.argper);
        reduce(manom,      sol2D.manom);
        reduce(denergy,    sol2D.denergy);
        reduce(dmomentum,  sol2D.dmomentum);

        reduce(roll1,      sol2D.roll1);
        reduce(pitch1,     sol2D.pitch1);
        reduce(yaw1,       sol2D.yaw1);
        reduce(relyaw1,    sol2D.relyaw1);
        reduce(w1ix,       sol2D.w1ix);
        reduce(w1iy,       sol2D.w1iy);
        reduce(w1iz,       sol2D.w1iz);
        reduce(w1bx,       sol2D.w1bx);
        reduce(w1by,       sol2D.w1by);
        reduce(w1bz,       sol2D.w1bz);

        reduce(roll2,      sol2D.roll2);
        reduce(pitch2,     sol2D.pitch2);
        reduce(yaw2,       sol2D.yaw2);
        reduce(relyaw2,    sol2D.relyaw2);
        reduce(w2ix,       sol2D.w2ix);
        reduce(w2iy,       sol2D.w2iy);
        reduce(w2iz,       sol2D.w2iz);
        reduce(w2bx,       sol2D.w2bx);
        reduce(w2by,       sol2D.w2by);
        reduce(w2bz,       sol2D.w2bz);

        if (integr.properties.spacecraft_checkbox)
        {
            reduce(xsp,        sol2D.xsp);
            reduce(ysp,        sol2D.ysp);
            reduce(zsp,        sol2D.zsp);
            reduce(dist_sp,    sol2D.dist_sp);
            reduce(vxsp,       sol2D.vxsp);
            reduce(vysp,       sol2D.vysp);
            reduce(vzsp,       sol2D.vzsp);
            reduce(vel_sp,     sol2D.vel_sp);
            reduce(sma_sp,     sol2D.sma_sp);
            reduce(ecc_sp,     sol2D.ecc_sp);
            reduce(inc_sp,     sol2D.inc_sp);
            reduce(raan_sp,    sol2D.raan_sp);
            reduce(argper_sp,  sol2D.argper_sp);
            reduce(manom_sp,   sol2D.manom_sp);

            reduce(xsp1,       sol2D.xsp1);
            reduce(ysp1,       sol2D.ysp1);
            reduce(zsp1,       sol2D.zsp1);
            reduce(dist_sp1,   sol2D.dist_sp1);
            reduce(vxsp1,      sol2D.vxsp1);
            reduce(vysp1,      sol2D.vysp1);
            reduce(vzsp1,      sol2D.vzsp1);
            reduce(vel_sp1,    sol2D.vel_sp1);
            reduce(sma_sp1,    sol2D.sma_sp1);
            reduce(ecc_sp1,    sol2D.ecc_sp1);
            reduce(inc_sp1,    sol2D.inc_sp1);
            reduce(raan_sp1,   sol2D.raan_sp1);
            reduce(argper_sp1, sol2D.argper_sp1);
            reduce(manom_sp1,  sol2D.manom_sp1);

            reduce(xsp2,       sol2D.xsp2);
            reduce(ysp2,       sol2D.ysp2);
            reduce(zsp2,       sol2D.zsp2);
            reduce(dist_sp2,   sol2D.dist_sp2);
            reduce(vxsp2,      sol2D.vxsp2);
            reduce(vysp2,      sol2D.vysp2);
            reduce(vzsp2,      sol2D.vzsp2);
            reduce(vel_sp2,    sol2D.vel_sp2);
            reduce(sma_sp2,    sol2D.sma_sp2);
            reduce(ecc_sp2,    sol2D.ecc_sp2);
            reduce(inc_sp2,    sol2D.inc_sp2);
            reduce(raan_sp2,   sol2D.raan_sp2);
            reduce(argper_sp2, sol2D.argper_sp2);
            reduce(manom_sp2,  sol2D.manom_sp2);
        }
        
        return sol2D;
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
        FILE *fp_w1b  = fopen((sim_dir + "/ang_vel_w1b.txt").c_str(), "w");
        FILE *fp_w2b  = fopen((sim_dir + "/ang_vel_w2b.txt").c_str(), "w");
        FILE *fp_rpy1 = fopen((sim_dir + "/euler_rpy1.txt" ).c_str(), "w");
        FILE *fp_w1i  = fopen((sim_dir + "/ang_vel_w1i.txt").c_str(), "w");
        FILE *fp_rpy2 = fopen((sim_dir + "/euler_rpy2.txt" ).c_str(), "w");
        FILE *fp_w2i  = fopen((sim_dir + "/ang_vel_w2i.txt").c_str(), "w");
        FILE *fp_kep  = fopen((sim_dir + "/keplerian.txt"  ).c_str(), "w");
        //Export the solution vectors.
        for (size_t i = 0; i < t.size(); ++i)
        {
            fprintf(fp_t,        "%.16lf\n", t[i]);
            fprintf(fp_pos,      "%.16lf %.16lf %.16lf %.16lf\n",                 x[i],    y[i],    z[i], dist[i]);
            fprintf(fp_vel,      "%.16lf %.16lf %.16lf %.16lf\n",                vx[i],   vy[i],   vz[i],  vel[i]);
            fprintf(fp_w1b,      "%.16lf %.16lf %.16lf\n",                     w1bx[i], w1by[i], w1bz[i]);
            fprintf(fp_w2b,      "%.16lf %.16lf %.16lf\n",                     w2bx[i], w2by[i], w2bz[i]);
            fprintf(fp_rpy1,     "%.16lf %.16lf %.16lf\n",                      roll1[i], pitch1[i], yaw1[i]);
            fprintf(fp_w1i,      "%.16lf %.16lf %.16lf\n",                       w1ix[i],   w1iy[i], w1iz[i]);
            fprintf(fp_rpy2,     "%.16lf %.16lf %.16lf\n",                      roll2[i], pitch2[i], yaw2[i]);
            fprintf(fp_w2i,      "%.16lf %.16lf %.16lf\n",                       w2ix[i],   w2iy[i], w2iz[i]);
            fprintf(fp_kep,      "%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",   sma[i],    ecc[i],  inc[i], raan[i], argper[i], manom[i]); 
        }
        fclose(fp_t);
        fclose(fp_pos);
        fclose(fp_vel);
        fclose(fp_w1b);
        fclose(fp_w2b);
        fclose(fp_rpy1);
        fclose(fp_w1i);
        fclose(fp_rpy2);
        fclose(fp_w2i);
        fclose(fp_kep);

        //Export spacecraft.
        if (integr.properties.spacecraft_checkbox)
        {
            FILE *fp_sp = fopen((sim_dir + "/spacecraft.txt").c_str(), "w");
            for (size_t i = 0; i < t.size(); ++i)
                fprintf(fp_sp, "%.16lf %.16lf %.16lf\n", xsp[i], ysp[i], zsp[i]);
            fclose(fp_sp);
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
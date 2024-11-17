#ifndef SOLUTION_H
#define SOLUTION_H

#include<cstdio>
#include<cmath>
#include<atomic>

#include"constant.h"
#include"typedef.h"
#include"linalg.h"
#include"conversion.h"
#include"integrator.h"

class solution
{
public:
    integrator integr;

    dvec t;

    dvec x, y, z;
    dvec vx, vy, vz;
    dvec q10, q11, q12, q13;
    dvec q20, q21, q22, q23;
    dvec w1bx, w1by, w1bz;
    dvec w2bx, w2by, w2bz;

    dvec dist, vel;
    dvec roll1, pitch1, yaw1; 
    dvec roll2, pitch2, yaw2;
    dvec w1ix, w1iy, w1iz;
    dvec w2ix, w2iy, w2iz;
    dvec sma, ecc, inc, raan, argper, manom;
    dvec ener_rel_err, mom_rel_err;

    void copy_integrator(const integrator &integr)
    {
        this->integr = integr;
    }

    void construct(std::atomic<bool> &abort_flag, std::atomic<float> &progress, console_panel &console)
    {
        console.add_time_and_then_text("[Info] : Solution construction started.");
        progress.store(0.0f);

        //Clear all the solution vectors, because the user might run more that 1 simulation (we don't want to append to the previous solution the new one...).

        t.clear();

        x.clear();   y.clear();  z.clear();
        vx.clear(); vy.clear(); vz.clear();

        q10.clear(); q11.clear(); q12.clear(); q13.clear();
        q20.clear(); q21.clear(); q22.clear(); q23.clear();

        w1bx.clear(); w1by.clear(); w1bz.clear();
        w2bx.clear(); w2by.clear(); w2bz.clear();

        dist.clear(); vel.clear();
        roll1.clear(); pitch1.clear(); yaw1.clear();
        roll2.clear(); pitch2.clear(); yaw2.clear();
        w1ix.clear(); w1iy.clear(); w1iz.clear();
        w2ix.clear(); w2iy.clear(); w2iz.clear();
        sma.clear(); ecc.clear(); inc.clear(); raan.clear(); argper.clear(); manom.clear();
        ener_rel_err.clear(); mom_rel_err.clear();

        double energy_at_t0, momentum_at_t0;

        for (size_t i = 0; i < integr.orbit.size(); ++i)
        {
            //Extract the integr.orbit[][] matrix into temporary variables for readability (though one could operate directly on integr.orbit[][]).
            //Remember integr.orbit contains : (t, x,y,z, vx,vy,vz, q10,q11,q12,q13, w1bx,w1by,w1bz, q20,q21,q22,q23, w2bx,w2by,w2bz) at each line i.
            dvec3  r   = dvec3{integr.orbit[i][1],  integr.orbit[i][2],  integr.orbit[i][3]};
            dvec3  v   = dvec3{integr.orbit[i][4],  integr.orbit[i][5],  integr.orbit[i][6]};
            dvec4  q1  = dvec4{integr.orbit[i][7],  integr.orbit[i][8],  integr.orbit[i][9],  integr.orbit[i][10]};
            dvec3  w1b = dvec3{integr.orbit[i][11], integr.orbit[i][12], integr.orbit[i][13]};
            dvec4  q2  = dvec4{integr.orbit[i][14], integr.orbit[i][15], integr.orbit[i][16], integr.orbit[i][17]};
            dvec3  w2b = dvec3{integr.orbit[i][18], integr.orbit[i][19], integr.orbit[i][20]};

            dmat3 A1   = quat2mat(q1);
            dmat3 A2   = quat2mat(q2);
            dvec3 w1i  = body2iner(w1b,A1);
            dvec3 w2i  = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);
            dvec6 kep  = cart2kep(dvec6{r[0],r[1],r[2], v[0],v[1],v[2]}, G*(integr.properties.M1 + integr.properties.M2));
            
            double energy = 0.5*integr.m*dot(v,v) + 0.5*dot( dot(w1b, integr.I1), w1b) + 0.5*dot( dot(w2b, integr.I2), w2b); //Kinetic energy part.
            //Potential energy part.
            if (integr.properties.ord2_checkbox)
                energy += mut_pot_integrals_ord2(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else if (integr.properties.ord3_checkbox)
                energy += mut_pot_integrals_ord3(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else
                energy += mut_pot_integrals_ord4(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            
            //Momentum magnitude. Note : All 3 components of the momentum vector are conserved in time. We just choose to store and plot the magnitude only.
            //However, keep in mind that if one wants to store all 3 components, then, the momentum vector must be evaluated in the inertial frame
            //because it is the inertial frame in which it is conserved.
            double momentum = length( integr.m*cross(r,v) + dot(integr.I1, w1b) + dot(integr.I2, w2b) );
            //dvec3 momentum = integr.m*cross(r,v) + body2iner(dot(integr.I1, w1b), A1) + body2iner(dot(integr.I2, w2b), A2);
            
            //This is meant to compute the corresponding relative errors. See below.
            if (i == 0)
            {
                energy_at_t0 = energy;
                momentum_at_t0 = momentum;
            }

            t.push_back(integr.orbit[i][0]/86400.0);

            x.push_back(r[0]);
            y.push_back(r[1]);
            z.push_back(r[2]);

            vx.push_back(v[0]);
            vy.push_back(v[1]);
            vz.push_back(v[2]);

            q10.push_back(q1[0]);
            q11.push_back(q1[1]);
            q12.push_back(q1[2]);
            q13.push_back(q1[3]);

            q20.push_back(q2[0]);
            q21.push_back(q2[1]);
            q22.push_back(q2[2]);
            q23.push_back(q2[3]);

            w1bx.push_back(w1b[0]);
            w1by.push_back(w1b[1]);
            w1bz.push_back(w1b[2]);

            w2bx.push_back(w2b[0]);
            w2by.push_back(w2b[1]);
            w2bz.push_back(w2b[2]);

            dist.push_back(length(r));
            vel.push_back(length(v));

            roll1.push_back(rpy1[0]*180.0/pi);
            pitch1.push_back(rpy1[1]*180.0/pi);
            yaw1.push_back(rpy1[2]*180.0/pi);

            roll2.push_back(rpy2[0]*180.0/pi);
            pitch2.push_back(rpy2[1]*180.0/pi);
            yaw2.push_back(rpy2[2]*180.0/pi);

            w1ix.push_back(w1i[0]);
            w1iy.push_back(w1i[1]);
            w1iz.push_back(w1i[2]);

            w2ix.push_back(w2i[0]);
            w2iy.push_back(w2i[1]);
            w2iz.push_back(w2i[2]);

            sma.push_back(kep[0]);
            ecc.push_back(kep[1]);
            inc.push_back(kep[2]*180.0/pi);
            raan.push_back(kep[3]*180.0/pi);
            argper.push_back(kep[4]*180.0/pi);
            manom.push_back(kep[5]*180.0/pi);

            ener_rel_err.push_back(fabs((energy - energy_at_t0)/energy_at_t0)); //0 at t = 0.
            mom_rel_err.push_back(fabs((momentum - momentum_at_t0)/momentum_at_t0)); //0 at t = 0.
        }

        if (!abort_flag.load())
        {
            progress.store(1.0f);
            console.add_time_and_then_text("[Info] : Solution construction ended.");
        }
    }

    void export_txt_files(const char *sim_name)
    {
        //Create the 'simulations' directory that will store all other simulation sub-directories.
        bool root_sim_dir = std::filesystem::create_directory("../simulations");
        //Create the current simulation directory 'sim_name' that will store the .txt files.
        bool current_sim_dir = std::filesystem::create_directory("../simulations/" + str(sim_name));

        //Create the txt contents
        FILE *file_t        = fopen(("../simulations/" + str(sim_name) + "/time.txt"    ).c_str(), "w");
        FILE *file_pos      = fopen(("../simulations/" + str(sim_name) + "/pos.txt"     ).c_str(), "w");
        FILE *file_vel      = fopen(("../simulations/" + str(sim_name) + "/vel.txt"     ).c_str(), "w");
        FILE *file_q1       = fopen(("../simulations/" + str(sim_name) + "/quat1.txt"   ).c_str(), "w");
        FILE *file_w1b      = fopen(("../simulations/" + str(sim_name) + "/w1b.txt"     ).c_str(), "w");
        FILE *file_q2       = fopen(("../simulations/" + str(sim_name) + "/quat2.txt"   ).c_str(), "w");
        FILE *file_w2b      = fopen(("../simulations/" + str(sim_name) + "/w2b.txt"     ).c_str(), "w");

        FILE *file_rpy1     = fopen(("../simulations/" + str(sim_name) + "/rpy1.txt"    ).c_str(), "w");
        FILE *file_w1i      = fopen(("../simulations/" + str(sim_name) + "/w1i.txt"     ).c_str(), "w");
        FILE *file_rpy2     = fopen(("../simulations/" + str(sim_name) + "/rpy2.txt"    ).c_str(), "w");
        FILE *file_w2i      = fopen(("../simulations/" + str(sim_name) + "/w2i.txt"     ).c_str(), "w");
        FILE *file_kep      = fopen(("../simulations/" + str(sim_name) + "/kep.txt"     ).c_str(), "w");
        FILE *file_ener_mom = fopen(("../simulations/" + str(sim_name) + "/ener_mom.txt").c_str(), "w");

        for (size_t i = 0; i < integr.orbit.size(); ++i)
        {
            fprintf(file_t,        "%.16lf\n",                                    integr.orbit[i][0]);
            fprintf(file_pos,      "%.16lf %.16lf %.16lf %.16lf\n",               integr.orbit[i][1],  integr.orbit[i][2],  integr.orbit[i][3], dist[i]);
            fprintf(file_vel,      "%.16lf %.16lf %.16lf %.16lf\n",               integr.orbit[i][4],  integr.orbit[i][5],  integr.orbit[i][6], vel[i]);
            fprintf(file_q1,       "%.16lf %.16lf %.16lf %.16lf\n",               integr.orbit[i][7],  integr.orbit[i][8],  integr.orbit[i][9],  integr.orbit[i][10]);
            fprintf(file_w1b,      "%.16lf %.16lf %.16lf\n",                      integr.orbit[i][11], integr.orbit[i][12], integr.orbit[i][13]);
            fprintf(file_q2,       "%.16lf %.16lf %.16lf %.16lf\n",               integr.orbit[i][14], integr.orbit[i][15], integr.orbit[i][16], integr.orbit[i][17]);
            fprintf(file_w2b,      "%.16lf %.16lf %.16lf\n",                      integr.orbit[i][18], integr.orbit[i][19], integr.orbit[i][20]);

            fprintf(file_rpy1,     "%.16lf %.16lf %.16lf\n",                      roll1[i], pitch1[i], yaw1[i]);
            fprintf(file_w1i,      "%.16lf %.16lf %.16lf\n",                      w1ix[i], w1iy[i], w1iz[i]);
            fprintf(file_rpy2,     "%.16lf %.16lf %.16lf\n",                      roll2[i], pitch2[i], yaw2[i]);
            fprintf(file_w2i,      "%.16lf %.16lf %.16lf\n",                      w2ix[i], w2iy[i], w2iz[i]);
            fprintf(file_kep,      "%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n", sma[i], ecc[i], inc[i], raan[i], argper[i], manom[i]); 
            fprintf(file_ener_mom, "%.16lf %.16lf\n",                             ener_rel_err[i], mom_rel_err[i]);
        }

        fclose(file_t);
        fclose(file_pos);
        fclose(file_vel);
        fclose(file_q1);
        fclose(file_w1b);
        fclose(file_q2);
        fclose(file_w2b);

        fclose(file_rpy1);
        fclose(file_w1i);
        fclose(file_rpy2);
        fclose(file_w2i);
        fclose(file_kep);
        fclose(file_ener_mom);

        FILE *file_collision = fopen(("../simulations/" + str(sim_name) + "/collision.txt").c_str(),"w");
        fprintf(file_collision,"Collision detected : %s", integr.collision ? "Yes" : "No");
        fclose(file_collision);
    }
};

#endif
#ifndef SOLUTION_H
#define SOLUTION_H

#include<cstdio>
#include<cmath>
#include<cstring>
#include<string>
#include<atomic>
#include<algorithm>

#include"constant.h"
#include"typedef.h"
#include"linalg.h"
#include"conversion.h"
#include"integrator.h"

class solution
{
public:
    integrator integr;

    //The following members are exactly the same (copies) with what the integrator evaluated, but stored in 1-D vectors (from t to w2bz).
    dvec t;
    dvec x, y, z;
    dvec vx, vy, vz;
    dvec q10, q11, q12, q13;
    dvec w1bx, w1by, w1bz;
    dvec q20, q21, q22, q23;
    dvec w2bx, w2by, w2bz;

    //The following members were NOT directly evaluated by the integrator. Instead, we use what the integrator evaluated to evaluate the following.
    dvec dist, vel; //Both are scalars. They are the corresponding magnitudes of (x,y,z) and (vx,vy,vz).
    dvec roll1, pitch1, yaw1; 
    dvec roll2, pitch2, yaw2;
    dvec w1ix, w1iy, w1iz;
    dvec w2ix, w2iy, w2iz;
    dvec sma, ecc, inc, raan, argper, manom;
    dvec ener_rel_err, mom_rel_err;

    solution() { } //This is needed in the scene_panel class.

    solution(const integrator &integr) //And this is needed in the gui class (deep copy of the integrator).
    {
        this->integr = integr;
    }

    //This function fills with orbital data all the vector members.
    void construct(console_panel &console)
    {
        console.add_timed_text("[Solution] : Constructing solution... ");

        double energy_at_t0, momentum_at_t0;

        const size_t N = integr.orbit.size();
        
        t.resize(N);
        
        x.resize(N);    y.resize(N);    z.resize(N);
        vx.resize(N);   vy.resize(N);   vz.resize(N);
        q10.resize(N);  q11.resize(N);  q12.resize(N);  q13.resize(N);
        w1bx.resize(N); w1by.resize(N); w1bz.resize(N);
        q20.resize(N);  q21.resize(N);  q22.resize(N);  q23.resize(N);
        w2bx.resize(N); w2by.resize(N); w2bz.resize(N);

        dist.resize(N);  vel.resize(N);
        roll1.resize(N); pitch1.resize(N); yaw1.resize(N);
        roll2.resize(N); pitch2.resize(N); yaw2.resize(N);
        w1ix.resize(N);  w1iy.resize(N);   w1iz.resize(N);
        w2ix.resize(N);  w2iy.resize(N);   w2iz.resize(N);
        sma.resize(N);   ecc.resize(N); inc.resize(N);  raan.resize(N); argper.resize(N); manom.resize(N);
        ener_rel_err.resize(N); mom_rel_err.resize(N);

        for (size_t i = 0; i < N; ++i)
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
            
            //Kinetic energy part.
            double energy = 0.5*integr.m*dot(v,v) + 0.5*dot( dot(w1b, integr.I1), w1b) + 0.5*dot( dot(w2b, integr.I2), w2b);

            //Potential energy part.
            if (integr.properties.ord2_checkbox)
                energy += mut_pot_integrals_ord2(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else if (integr.properties.ord3_checkbox)
                energy += mut_pot_integrals_ord3(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            else
                energy += mut_pot_integrals_ord4(r, integr.properties.M1, integr.J1, A1, integr.properties.M2, integr.J2, A2);
            
            //Momentum magnitude. Note : All 3 components of the momentum vector are conserved in time. We just choose to store and plot the magnitude only.
            double momentum = length( integr.m*cross(r,v) + dot(A1, dot(integr.I1, w1b)) + dot(A2, dot(integr.I2, w2b)) );

            t[i] = integr.orbit[i][0]/86400.0;

            x[i] = r[0];
            y[i] = r[1];
            z[i] = r[2];

            vx[i] = v[0];
            vy[i] = v[1];
            vz[i] = v[2];

            q10[i] = q1[0];
            q11[i] = q1[1];
            q12[i] = q1[2];
            q13[i] = q1[3];

            w1bx[i] = w1b[0];
            w1by[i] = w1b[1];
            w1bz[i] = w1b[2];

            q20[i] = q2[0];
            q21[i] = q2[1];
            q22[i] = q2[2];
            q23[i] = q2[3];

            w2bx[i] = w2b[0];
            w2by[i] = w2b[1];
            w2bz[i] = w2b[2];

            dist[i] = length(r);
            vel[i]  = length(v);

            roll1[i]  = rpy1[0]*180.0/pi;
            pitch1[i] = rpy1[1]*180.0/pi;
            yaw1[i]   = rpy1[2]*180.0/pi;

            roll2[i]  = rpy2[0]*180.0/pi;
            pitch2[i] = rpy2[1]*180.0/pi;
            yaw2[i]   = rpy2[2]*180.0/pi;

            w1ix[i] = w1i[0];
            w1iy[i] = w1i[1];
            w1iz[i] = w1i[2];

            w2ix[i] = w2i[0];
            w2iy[i] = w2i[1];
            w2iz[i] = w2i[2];

            sma[i]    = kep[0];
            ecc[i]    = kep[1];
            inc[i]    = kep[2]*180.0/pi;
            raan[i]   = kep[3]*180.0/pi;
            argper[i] = kep[4]*180.0/pi;
            manom[i]  = kep[5]*180.0/pi;

            if (i == 0)
            {
                energy_at_t0 = energy;
                momentum_at_t0 = momentum;
                ener_rel_err[0] = 0.0;
                mom_rel_err[0]  = 0.0;
            }
            else
            {
                if (fabs(energy_at_t0) > 1e-16)
                    ener_rel_err[i] = fabs((energy - energy_at_t0)/energy_at_t0);
                else //Fallback to absolute error to avoid division by zero.
                    ener_rel_err[i] = fabs(energy - energy_at_t0);
            
                if (fabs(momentum_at_t0) > 1e-16)
                    mom_rel_err[i] = fabs((momentum - momentum_at_t0)/momentum_at_t0);
                else //The same...
                    mom_rel_err[i] = fabs(momentum - momentum_at_t0);
            }
        }
        console.add_text("Done.\n");
    }

    void reduce_vector(dvec &vec, const size_t final_size)
    {
        double step = (vec.size() - 1.0)/(final_size - 1.0);
        dvec reduced(final_size);
        for (size_t i = 0; i < final_size; ++i)
        {
            size_t index = (size_t)(i*step);
            reduced[i] = vec[index];
        }
        vec = std::move(reduced); //Fast copy.
    }

    void reduce(const size_t final_size)
    {
        //If final_size is zero or larger than the current size, there's nothing to reduce.
        if (final_size == 0 || t.size() <= final_size)
            return;

        //Reduce all solution member vectors.
        reduce_vector(t, final_size);
        reduce_vector(x, final_size);
        reduce_vector(y, final_size);
        reduce_vector(z, final_size);
        reduce_vector(vx, final_size);
        reduce_vector(vy, final_size);
        reduce_vector(vz, final_size);
        reduce_vector(q10, final_size);
        reduce_vector(q11, final_size);
        reduce_vector(q12, final_size);
        reduce_vector(q13, final_size);
        reduce_vector(w1bx, final_size);
        reduce_vector(w1by, final_size);
        reduce_vector(w1bz, final_size);
        reduce_vector(q20, final_size);
        reduce_vector(q21, final_size);
        reduce_vector(q22, final_size);
        reduce_vector(q23, final_size);
        reduce_vector(w2bx, final_size);
        reduce_vector(w2by, final_size);
        reduce_vector(w2bz, final_size);
        reduce_vector(dist, final_size);
        reduce_vector(vel, final_size);
        reduce_vector(roll1, final_size);
        reduce_vector(pitch1, final_size);
        reduce_vector(yaw1, final_size);
        reduce_vector(roll2, final_size);
        reduce_vector(pitch2, final_size);
        reduce_vector(yaw2, final_size);
        reduce_vector(w1ix, final_size);
        reduce_vector(w1iy, final_size);
        reduce_vector(w1iz, final_size);
        reduce_vector(w2ix, final_size);
        reduce_vector(w2iy, final_size);
        reduce_vector(w2iz, final_size);
        reduce_vector(sma, final_size);
        reduce_vector(ecc, final_size);
        reduce_vector(inc, final_size);
        reduce_vector(raan, final_size);
        reduce_vector(argper, final_size);
        reduce_vector(manom, final_size);
        reduce_vector(ener_rel_err, final_size);
        reduce_vector(mom_rel_err, final_size);
    }

    //Create and return a reduced (downsample) version of the solution.
    solution get_reduced_solution(const size_t final_size) const
    {
        solution sol_copy = *this; //Copy the already existing solution.
        sol_copy.reduce(final_size); //Reduce all vectors in the copy.
        return sol_copy;
    }

    void export_files(console_panel &console)
    {
        console.add_timed_text("[Solution] : Exporting solution files... ");

        const char *sim_name = integr.properties.sim_name;
        //Create the 'simulations' (root) directory that will store all other simulation sub-directories.
        std::filesystem::create_directory("../simulations");
        //Create the current simulation directory 'sim_name' that will store the solution files.
        std::filesystem::create_directory("../simulations/" + std::string(sim_name));

        //Create the txt contents
        FILE *file_t        = fopen(("../simulations/" + std::string(sim_name) + "/time.txt"              ).c_str(), "w");
        FILE *file_pos      = fopen(("../simulations/" + std::string(sim_name) + "/rel_pos.txt"           ).c_str(), "w");
        FILE *file_vel      = fopen(("../simulations/" + std::string(sim_name) + "/rel_vel.txt"           ).c_str(), "w");
        FILE *file_q1       = fopen(("../simulations/" + std::string(sim_name) + "/quaternion1.txt"       ).c_str(), "w");
        FILE *file_w1b      = fopen(("../simulations/" + std::string(sim_name) + "/ang_vel_w1b.txt"       ).c_str(), "w");
        FILE *file_q2       = fopen(("../simulations/" + std::string(sim_name) + "/quaternion2.txt"       ).c_str(), "w");
        FILE *file_w2b      = fopen(("../simulations/" + std::string(sim_name) + "/ang_vel_w2b.txt"       ).c_str(), "w");

        FILE *file_rpy1     = fopen(("../simulations/" + std::string(sim_name) + "/euler_rpy1.txt"        ).c_str(), "w");
        FILE *file_w1i      = fopen(("../simulations/" + std::string(sim_name) + "/ang_vel_w1i.txt"       ).c_str(), "w");
        FILE *file_rpy2     = fopen(("../simulations/" + std::string(sim_name) + "/euler_rpy2.txt"        ).c_str(), "w");
        FILE *file_w2i      = fopen(("../simulations/" + std::string(sim_name) + "/ang_vel_w2i.txt"       ).c_str(), "w");
        FILE *file_kep      = fopen(("../simulations/" + std::string(sim_name) + "/keplerian.txt"         ).c_str(), "w");
        FILE *file_ener_mom = fopen(("../simulations/" + std::string(sim_name) + "/ener_mom_rel_error.txt").c_str(), "w");

        for (size_t i = 0; i < t.size(); ++i)
        {
            fprintf(file_t,        "%.16lf\n", t[i]);
            fprintf(file_pos,      "%.16lf %.16lf %.16lf %.16lf\n",                 x[i],    y[i],    z[i], dist[i]);
            fprintf(file_vel,      "%.16lf %.16lf %.16lf %.16lf\n",                vx[i],   vy[i],   vz[i],  vel[i]);
            fprintf(file_q1,       "%.16lf %.16lf %.16lf %.16lf\n",               q10[i],  q11[i],  q12[i],  q13[i]);
            fprintf(file_w1b,      "%.16lf %.16lf %.16lf\n",                     w1bx[i], w1by[i], w1bz[i]);
            fprintf(file_q2,       "%.16lf %.16lf %.16lf %.16lf\n",               q20[i],  q21[i],  q22[i],  q23[i]);
            fprintf(file_w2b,      "%.16lf %.16lf %.16lf\n",                     w2bx[i], w2by[i], w2bz[i]);

            fprintf(file_rpy1,     "%.16lf %.16lf %.16lf\n",                      roll1[i], pitch1[i], yaw1[i]);
            fprintf(file_w1i,      "%.16lf %.16lf %.16lf\n",                       w1ix[i],   w1iy[i], w1iz[i]);
            fprintf(file_rpy2,     "%.16lf %.16lf %.16lf\n",                      roll2[i], pitch2[i], yaw2[i]);
            fprintf(file_w2i,      "%.16lf %.16lf %.16lf\n",                       w2ix[i],   w2iy[i], w2iz[i]);
            fprintf(file_kep,      "%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",   sma[i],    ecc[i],  inc[i], raan[i], argper[i], manom[i]); 
            fprintf(file_ener_mom, "%.16lf %.16lf\n",                      ener_rel_err[i], mom_rel_err[i]);
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

        FILE *file_collision = fopen(("../simulations/" + std::string(sim_name) + "/collision.txt").c_str(),"w");
        fprintf(file_collision,"Collision detected : %s", integr.collision ? "Yes" : "No");
        fclose(file_collision);

        console.add_text("Done.\n");
    }
};

#endif
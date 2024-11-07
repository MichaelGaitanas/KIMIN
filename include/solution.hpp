#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include<cstdio>
#include<cmath>

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"
#include"conversion.hpp"
#include"integrator.hpp"

class solution
{
private:
    integrator integr;

public:
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

    void construct()
    {
        double energy_at_t0, momentum_at_t0;

        for (size_t i = 0; i < integr.orbit.size(); ++i)
        {
            //Extract the integr.orbit[][] matrix into temporary variables for readability (though one could operate directly on integr.orbit[][]).
            //Remember integr.orbit contains : (t, x,y,z, vx,vy,vz, q10,q11,q12,q13, w1bx,w1by,w1bz, q20,q21,q22,q23, w2bx,w2by,w2bz) at each line i.
            double t   = integr.orbit[i][0];
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
            double momentum = length( integr.m*cross(r,v) + dot(A1, dot(integr.I1, w1b)) + dot(A2, dot(integr.I2, w2b)) );
            
            //This is to compute the corresponding relative errors. See below.
            if (i == 0)
            {
                energy_at_t0 = energy;
                momentum_at_t0 = momentum;
            }

            //To be continued.
            
            t.push_back(temp_t/86400.0);
            x.push_back(temp_r[0]);
            y.push_back(temp_r[1]);
            z.push_back(temp_r[2]);
            dist.push_back(length(temp_r));
            roll1.push_back(temp_rpy1[0]*180.0/pi);
            pitch1.push_back(temp_rpy1[1]*180.0/pi);
            yaw1.push_back(temp_rpy1[2]*180.0/pi);
            roll2.push_back(temp_rpy2[0]*180.0/pi);
            pitch2.push_back(temp_rpy2[1]*180.0/pi);
            yaw2.push_back(temp_rpy2[2]*180.0/pi);
            q10.push_back(temp_q1[0]);
            q11.push_back(temp_q1[1]);
            q12.push_back(temp_q1[2]);
            q13.push_back(temp_q1[3]);
            q20.push_back(temp_q2[0]);
            q21.push_back(temp_q2[1]);
            q22.push_back(temp_q2[2]);
            q23.push_back(temp_q2[3]);
            vx.push_back(temp_v[0]);
            vy.push_back(temp_v[1]);
            vz.push_back(temp_v[2]);
            vmag.push_back(length(temp_v));
            w1ix.push_back(temp_w1i[0]);
            w1iy.push_back(temp_w1i[1]);
            w1iz.push_back(temp_w1i[2]);
            w1bx.push_back(temp_w1b[0]);
            w1by.push_back(temp_w1b[1]);
            w1bz.push_back(temp_w1b[2]);
            w2ix.push_back(temp_w2i[0]);
            w2iy.push_back(temp_w2i[1]);
            w2iz.push_back(temp_w2i[2]);
            w2bx.push_back(temp_w2b[0]);
            w2by.push_back(temp_w2b[1]);
            w2bz.push_back(temp_w2b[2]);
            a.push_back(temp_kep[0]);
            e.push_back(temp_kep[1]);
            inc.push_back(temp_kep[2]*180.0/pi);
            Om.push_back(temp_kep[3]*180.0/pi);
            w.push_back(temp_kep[4]*180.0/pi);
            M.push_back(temp_kep[5]*180.0/pi);
            ener_rel_err.push_back(fabs((ener - energy_at_t0)/energy_at_t0));
            mom_rel_err.push_back(fabs((mom - momentum_at_t0)/momentum_at_t0));
        }
        max_dist = *max_element(dist.begin(), dist.end());
    }

    void export_txt_files(const char *sim_name)
    {
        //1) create the 'simulations' directory that will store all other simulation directories
        bool root_sim_dir = std::filesystem::create_directory("../simulations");

        //2) create the current simulation directory 'sim_name' that will store the .txt files
        bool current_sim_dir = std::filesystem::create_directory("../simulations/" + str(sim_name));

        //3) create the txt content
        FILE *file_t        = fopen(("../simulations/" + str(sim_name) + "/time.txt"    ).c_str(), "w");
        FILE *file_pos      = fopen(("../simulations/" + str(sim_name) + "/pos.txt"     ).c_str(), "w");
        FILE *file_vel      = fopen(("../simulations/" + str(sim_name) + "/vel.txt"     ).c_str(), "w");
        FILE *file_q1       = fopen(("../simulations/" + str(sim_name) + "/quat1.txt"   ).c_str(), "w");
        FILE *file_w1i      = fopen(("../simulations/" + str(sim_name) + "/w1i.txt"     ).c_str(), "w");
        FILE *file_w1b      = fopen(("../simulations/" + str(sim_name) + "/w1b.txt"     ).c_str(), "w");
        FILE *file_rpy1     = fopen(("../simulations/" + str(sim_name) + "/rpy1.txt"    ).c_str(), "w");
        FILE *file_q2       = fopen(("../simulations/" + str(sim_name) + "/quat2.txt"   ).c_str(), "w");
        FILE *file_w2i      = fopen(("../simulations/" + str(sim_name) + "/w2i.txt"     ).c_str(), "w");
        FILE *file_w2b      = fopen(("../simulations/" + str(sim_name) + "/w2b.txt"     ).c_str(), "w");
        FILE *file_rpy2     = fopen(("../simulations/" + str(sim_name) + "/rpy2.txt"    ).c_str(), "w");
        FILE *file_kep      = fopen(("../simulations/" + str(sim_name) + "/kep.txt"     ).c_str(), "w");
        FILE *file_ener_mom = fopen(("../simulations/" + str(sim_name) + "/ener_mom.txt").c_str(), "w");
        for (int i = 0; i < t.size(); ++i)
        {
            fprintf(file_t,        "%.16lf\n",                                    t[i]);
            fprintf(file_pos,      "%.16lf %.16lf %.16lf %.16lf\n",               x[i],y[i],z[i], dist[i]);
            fprintf(file_vel,      "%.16lf %.16lf %.16lf %.16lf\n",               vx[i],vy[i],vz[i], vmag[i]);
            fprintf(file_q1,       "%.16lf %.16lf %.16lf %.16lf\n",               q10[i],q11[i],q12[i],q13[i]);
            fprintf(file_rpy1,     "%.16lf %.16lf %.16lf\n",                      roll1[i],pitch1[i],yaw1[i]);
            fprintf(file_w1b,      "%.16lf %.16lf %.16lf\n",                      w1bx[i],w1by[i],w1bz[i]);
            fprintf(file_w1i,      "%.16lf %.16lf %.16lf\n",                      w1ix[i],w1iy[i],w1iz[i]);
            fprintf(file_q2,       "%.16lf %.16lf %.16lf %.16lf\n",               q20[i],q21[i],q22[i],q23[i]);
            fprintf(file_rpy2,     "%.16lf %.16lf %.16lf\n",                      roll2[i],pitch2[i],yaw2[i]);
            fprintf(file_w2b,      "%.16lf %.16lf %.16lf\n",                      w2bx[i],w2by[i],w2bz[i]);
            fprintf(file_w2i,      "%.16lf %.16lf %.16lf\n",                      w2ix[i],w2iy[i],w2iz[i]);
            fprintf(file_kep,      "%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n", a[i],e[i],inc[i],Om[i],w[i],M[i]); 
            fprintf(file_ener_mom, "%.16lf %.16lf\n",                             ener_rel_err[i], mom_rel_err[i]);
        }
        fclose(file_t);
        fclose(file_pos);
        fclose(file_vel);
        fclose(file_q1);
        fclose(file_rpy1);
        fclose(file_w1b);
        fclose(file_w1i);
        fclose(file_q2);
        fclose(file_rpy2);
        fclose(file_w2b);
        fclose(file_w2i);
        fclose(file_kep);
        fclose(file_ener_mom);

        FILE *file_collision = fopen(("../simulations/" + str(sim_name) + "/collision.txt").c_str(),"w");
        fprintf(file_collision,"Collision detected : %s", collision ? "Yes" : "No");
        fclose(file_collision);
    }
};

#endif
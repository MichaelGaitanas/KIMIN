#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include<cstdio>
#include<cmath>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"obj.hpp"
#include"conversion.hpp"
#include"mascon.hpp"
#include"ellipsoid.hpp"
#include"rigidbody.hpp"
#include"potential.hpp"
#include"force.hpp"
#include"torque.hpp"

#include"properties.hpp"

class Solution
{

public:

    //time
    dvec t;

    //positions
    dvec x,y,z, dist;
    dvec roll1,pitch1,yaw1; 
    dvec roll2,pitch2,yaw2;
    dvec q10,q11,q12,q13;
    dvec q20,q21,q22,q23;

    //velocities
    dvec vx,vy,vz, vmag;
    dvec w1ix,w1iy,w1iz;
    dvec w1bx,w1by,w1bz;
    dvec w2ix,w2iy,w2iz;
    dvec w2bx,w2by,w2bz;

    //Keplerian elements
    dvec a,e,inc,Om,w,M;

    //constants of motion relative errors
    dvec ener_rel_err, mom_rel_err;

    bool collision;

    //Default constructor will be called for the members.

    void export_txt_files(const char *simname)
    {
        //1) create the 'simulations' directory that will store all other simulation directories
        bool root_sim_dir = std::filesystem::create_directory("../simulations");

        //2) create the current simulation directory 'simname' that will store the .txt files
        bool current_sim_dir = std::filesystem::create_directory("../simulations/" + str(simname));

        //3) create the txt content
        FILE *file_t        = fopen(("../simulations/" + str(simname) + "/time.txt"    ).c_str(),"w");
        FILE *file_pos      = fopen(("../simulations/" + str(simname) + "/pos.txt"     ).c_str(),"w");
        FILE *file_vel      = fopen(("../simulations/" + str(simname) + "/vel.txt"     ).c_str(),"w");
        FILE *file_q1       = fopen(("../simulations/" + str(simname) + "/quat1.txt"   ).c_str(),"w");
        FILE *file_w1i      = fopen(("../simulations/" + str(simname) + "/w1i.txt"     ).c_str(),"w");
        FILE *file_w1b      = fopen(("../simulations/" + str(simname) + "/w1b.txt"     ).c_str(),"w");
        FILE *file_rpy1     = fopen(("../simulations/" + str(simname) + "/rpy1.txt"    ).c_str(),"w");
        FILE *file_q2       = fopen(("../simulations/" + str(simname) + "/quat2.txt"   ).c_str(),"w");
        FILE *file_w2i      = fopen(("../simulations/" + str(simname) + "/w2i.txt"     ).c_str(),"w");
        FILE *file_w2b      = fopen(("../simulations/" + str(simname) + "/w2b.txt"     ).c_str(),"w");
        FILE *file_rpy2     = fopen(("../simulations/" + str(simname) + "/rpy2.txt"    ).c_str(),"w");
        FILE *file_kep      = fopen(("../simulations/" + str(simname) + "/kep.txt"     ).c_str(),"w");
        FILE *file_ener_mom = fopen(("../simulations/" + str(simname) + "/ener_mom.txt").c_str(),"w");
        for (int i = 0; i < t.size(); ++i)
        {
            fprintf(file_t,        "%.16lf\n",                                    t[i]);
            fprintf(file_pos,      "%.16lf %.16lf %.16lf\n",                      x[i],y[i],z[i], dist[i]);
            fprintf(file_vel,      "%.16lf %.16lf %.16lf\n",                      vx[i],vy[i],vz[i], vmag[i]);
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

        FILE *file_steps = fopen(("../simulations/" + str(simname) + "/steps.txt").c_str(),"w");
        fprintf(file_steps,"%d\n",(int)(t.size() - 1));
        fclose(file_steps);

        FILE *file_collision = fopen(("../simulations/" + str(simname) + "/collision.txt").c_str(),"w");
        fprintf(file_collision,"%d\n",collision);
        fclose(file_collision);

        return;
    }
};

#endif
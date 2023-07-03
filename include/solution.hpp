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

#include"integrator.hpp"

class Solution : public Integrator
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

    //plotting parameters
    double max_dist;
    
    Solution() { }

    Solution(const Integrator &integrator) : Integrator(integrator)
    {
        if(ell_checkbox){
            obj_path1 = obj_path2 = "../obj/sphere.obj";
        }
        
        double ener0, mom0;
        for (int i = 0; i < msol.size(); ++i)
        {
            //organize msol[][] into temporary variables
            double temp_t    = msol[i][0];
            dvec3  temp_r    = {msol[i][1], msol[i][2], msol[i][3]};
            dvec3  temp_v    = {msol[i][4], msol[i][5], msol[i][6]};
            dvec4  temp_q1   = {msol[i][7], msol[i][8], msol[i][9], msol[i][10]};
            dvec3  temp_w1b  = {msol[i][11], msol[i][12], msol[i][13]};
            dvec4  temp_q2   = {msol[i][14], msol[i][15], msol[i][16], msol[i][17]};
            dvec3  temp_w2b  = {msol[i][18], msol[i][19], msol[i][20]};
            dmat3  temp_A1   = quat2mat(temp_q1);
            dmat3  temp_A2   = quat2mat(temp_q2);
            dvec3  temp_w1i  = body2iner(temp_w1b,temp_A1);
            dvec3  temp_w2i  = body2iner(temp_w2b,temp_A2);
            dvec3  temp_rpy1 = quat2ang(temp_q1);
            dvec3  temp_rpy2 = quat2ang(temp_q2);
            dvec6  temp_kep  = cart2kep({temp_r[0],temp_r[1],temp_r[2], temp_v[0],temp_v[1],temp_v[2]}, G*(M1+M2));
            
            double ener = 0.5*((M1*M2)/(M1+M2))*dot(temp_v,temp_v) + 0.5*dot( dot(temp_w1b,I1), temp_w1b) + 0.5*dot( dot(temp_w2b,I2), temp_w2b);
            if (ord2_checkbox)
                ener += mut_pot_integrals_ord2(temp_r, M1,J1,temp_A1, M2,J2,temp_A2);
            else if (ord3_checkbox)
                ener += mut_pot_integrals_ord3(temp_r, M1,J1,temp_A1, M2,J2,temp_A2);
            else if (ord4_checkbox)
                ener += mut_pot_integrals_ord4(temp_r, M1,J1,temp_A1, M2,J2,temp_A2);
            
            double mom = length( (M1*M2/(M1+M2))*cross(temp_r,temp_v) + dot(temp_A1, dot(I1,temp_w1b)) + dot(temp_A2, dot(I2,temp_w2b)) );
            
            if (i == 0)
            {
                ener0 = ener;
                mom0 = mom;
            }
            
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
            ener_rel_err.push_back(fabs((ener - ener0)/ener0));
            mom_rel_err.push_back(fabs((mom - mom0)/mom0));
        }
        max_dist = *max_element(dist.begin(), dist.end());
    }

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
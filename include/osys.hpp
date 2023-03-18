#ifndef OSYS_HPP
#define OSYS_HPP

#include<cstdio>
#include<filesystem>
#include<fstream>
#include<vector>

#include"typedef.hpp"

/*
void export_txt_files()
{
    //1) create the 'simulations' directory that will store all other simulation directories
    bool root_sim_dir = std::filesystem::create_directory("../simulations");

    //2) create the current simulation directory 'simname' that will store the .txt files
    bool current_sim_dir = std::filesystem::create_directory("../simulations/" + str(simname));

    //3) create the txt content

    FILE *fpt = fopen(("../simulations/" + str(simname) + "/time.txt").c_str(), "w");
    FILE *fprv = fopen(("../simulations/" + str(simname) + "/pos_vel.txt").c_str(),"w");
    FILE *fpq1 = fopen(("../simulations/" + str(simname) + "/quat1.txt").c_str(),"w");
    FILE *fpw1i = fopen(("../simulations/" + str(simname) + "/w1i.txt").c_str(),"w");
    FILE *fpw1b = fopen(("../simulations/" + str(simname) + "/w1b.txt").c_str(),"w");
    FILE *fprpy1 = fopen(("../simulations/" + str(simname) + "/rpy1.txt").c_str(),"w");
    FILE *fpq2 = fopen(("../simulations/" + str(simname) + "/quat2.txt").c_str(),"w");
    FILE *fpw2i = fopen(("../simulations/" + str(simname) + "/w2i.txt").c_str(),"w");
    FILE *fpw2b = fopen(("../simulations/" + str(simname) + "/w2b.txt").c_str(),"w");
    FILE *fprpy2 = fopen(("../simulations/" + str(simname) + "/rpy2.txt").c_str(),"w");
    FILE *fpEL = fopen(("../simulations/" + str(simname) + "/ener_mom.txt").c_str(),"w");
    for (int i = 0; i < sol.size(); ++i)
    {
        double t = sol[i][0];
        dvec3 r = {sol[i][1], sol[i][2], sol[i][3]};
        dvec3 v = {sol[i][4], sol[i][5], sol[i][6]};
        dvec4 q1 = {sol[i][7], sol[i][8], sol[i][9], sol[i][10]};
        dvec3 w1b = {sol[i][11], sol[i][12], sol[i][13]};
        dvec4 q2 = {sol[i][14], sol[i][15], sol[i][16], sol[i][17]};
        dvec3 w2b = {sol[i][18], sol[i][19], sol[i][20]};
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);
        dvec3 w1i = body2iner(w1b,A1);
        dvec3 w2i = body2iner(w2b,A2);
        dvec3 rpy1 = quat2ang(q1);
        dvec3 rpy2 = quat2ang(q2);

        double ener;
        if (ord2_checkbox)
            ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_integrals_ord2(r, M1,J1,A1, M2,J2,A2);
        else if (ord3_checkbox)
            ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_integrals_ord3(r, M1,J1,A1, M2,J2,A2);
        else if (ord4_checkbox)
            ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_integrals_ord4(r, M1,J1,A1, M2,J2,A2);
        else
            ener = ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_masc(r, M1,masc1,A1, M2,masc2,A2);

        //The momentum vector calculation does not depend on the order of the expansion, so it's always the same.
        dvec3 mom = (M1*M2/(M1+M2))*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b));

        fprintf(fpt,"%.16lf\n",t);
        fprintf(fprv,"%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",r[0],r[1],r[2], v[0],v[1],v[2]);
        fprintf(fpq1,"%.16lf %.16lf %.16lf %.16lf\n",q1[0],q1[1],q1[2],q1[3]);
        fprintf(fprpy1,"%.16lf %.16lf %.16lf\n",rpy1[0],rpy1[1],rpy1[2]);
        fprintf(fpw1b,"%.16lf %.16lf %.16lf\n",w1b[0],w1b[1],w1b[2]);
        fprintf(fpw1i,"%.16lf %.16lf %.16lf\n",w1i[0],w1i[1],w1i[2]);
        fprintf(fpq2,"%.16lf %.16lf %.16lf %.16lf\n",q2[0],q2[1],q2[2],q2[3]);
        fprintf(fprpy2,"%.16lf %.16lf %.16lf\n",rpy2[0],rpy2[1],rpy2[2]);
        fprintf(fpw2b,"%.16lf %.16lf %.16lf\n",w2b[0],w2b[1],w2b[2]);
        fprintf(fpw2i,"%.16lf %.16lf %.16lf\n",w2i[0],w2i[1],w2i[2]); 
        fprintf(fpEL,"%.16lf %.16lf %.16lf %.16lf\n", ener, mom[0],mom[1],mom[2]);
    }
    fclose(fpt);
    fclose(fprv);
    fclose(fpq1);
    fclose(fprpy1);
    fclose(fpw1b);
    fclose(fpw1i);
    fclose(fpq2);
    fclose(fprpy2);
    fclose(fpw2b);
    fclose(fpw2i);
    fclose(fpEL);

    FILE *fpsteps = fopen(("../simulations/" + str(simname) + "/steps.txt").c_str(),"w");
    fprintf(fpsteps,"%d\n",(int)(sol.size() - 1));
    fclose(fpsteps);

    FILE *fpcollision = fopen(("../simulations/" + str(simname) + "/collision.txt").c_str(),"w");
    fprintf(fpcollision,"%d\n",collision);
    fclose(fpcollision);

    return;
}
*/



#endif
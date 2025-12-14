#ifndef SOLUTION2D_H
#define SOLUTION2D_H

#include<cstdio>
#include<cmath>
#include<algorithm>

#ifdef USE_OPENMP
    #include<omp.h>
#endif

#include"constants.h"
#include"typedef.h"
#include"linalg.h"
#include"conversion.h"
#include"properties.h"
#include"integrator.h"
#include"solution.h"

class solution2D
{
public:
    //The following vector members constitute the REDUCED version of the solution of the ODEs and each vector is used only for GUI plotting.
    dvec t;

    //Mutual of the binary :
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

    solution2D() { }

    void clear()
    {
        t.clear();

        x.clear();       y.clear();         z.clear();    dist.clear();
        vx.clear();      vy.clear();        vz.clear();   vel.clear();
        sma.clear();     ecc.clear();       inc.clear();  raan.clear(); argper.clear(); manom.clear();
        denergy.clear(); dmomentum.clear();
        roll1.clear();   pitch1.clear();    yaw1.clear(); relyaw1.clear();
        w1ix.clear();    w1iy.clear();      w1iz.clear();
        w1bx.clear();    w1by.clear();      w1bz.clear();
        roll2.clear();   pitch2.clear();    yaw2.clear(); relyaw2.clear();
        w2ix.clear();    w2iy.clear();      w2iz.clear();
        w2bx.clear();    w2by.clear();      w2bz.clear();

        xsp.clear();     ysp.clear();     zsp.clear();     dist_sp.clear();
        vxsp.clear();    vysp.clear();    vzsp.clear();    vel_sp.clear();
        sma_sp.clear();  ecc_sp.clear();  inc_sp.clear();  raan_sp.clear();  argper_sp.clear();  manom_sp.clear();
        xsp1.clear();    ysp1.clear();    zsp1.clear();    dist_sp1.clear();
        vxsp1.clear();   vysp1.clear();   vzsp1.clear();   vel_sp1.clear();
        sma_sp1.clear(); ecc_sp1.clear(); inc_sp1.clear(); raan_sp1.clear(); argper_sp1.clear(); manom_sp1.clear();
        xsp2.clear();    ysp2.clear();    zsp2.clear();    dist_sp2.clear();
        vxsp2.clear();   vysp2.clear();   vzsp2.clear();   vel_sp2.clear();
        sma_sp2.clear(); ecc_sp2.clear(); inc_sp2.clear(); raan_sp2.clear(); argper_sp2.clear(); manom_sp2.clear();
    }

    dvec2 get_energy_and_momentum_at_t0(const solution &sol)
    {
        dvec3 r0   = {sol.integr.orbit[0][1],  sol.integr.orbit[0][2],  sol.integr.orbit[0][3]};
        dvec3 v0   = {sol.integr.orbit[0][4],  sol.integr.orbit[0][5],  sol.integr.orbit[0][6]};
        dvec4 q10  = {sol.integr.orbit[0][7],  sol.integr.orbit[0][8],  sol.integr.orbit[0][9],  sol.integr.orbit[0][10]};
        dvec3 w1b0 = {sol.integr.orbit[0][11], sol.integr.orbit[0][12], sol.integr.orbit[0][13]};
        dvec4 q20  = {sol.integr.orbit[0][14], sol.integr.orbit[0][15], sol.integr.orbit[0][16], sol.integr.orbit[0][17]};
        dvec3 w2b0 = {sol.integr.orbit[0][18], sol.integr.orbit[0][19], sol.integr.orbit[0][20]};

        dmat3 A10 = quat2mat(q10);
        dmat3 A20 = quat2mat(q20);

        double energy0 = 0.5*sol.integr.m*dot(v0,v0) + 0.5*dot( dot(w1b0, sol.integr.I1), w1b0) + 0.5*dot( dot(w2b0, sol.integr.I2), w2b0);

        if (sol.integr.props.ord2_checkbox)
            energy0 += mut_pot_integrals_ord2(r0, sol.integr.props.M1, sol.integr.J1, A10,
                                                  sol.integr.props.M2, sol.integr.J2, A20);
        else if (sol.integr.props.ord3_checkbox)
            energy0 += mut_pot_integrals_ord3(r0, sol.integr.props.M1, sol.integr.J1, A10,
                                                  sol.integr.props.M2, sol.integr.J2, A20);
        else
            energy0 += mut_pot_integrals_ord4(r0, sol.integr.props.M1, sol.integr.J1, A10,
                                                  sol.integr.props.M2, sol.integr.J2, A20);

        double momentum0 = length( sol.integr.m*cross(r0,v0) + dot(A10, dot(sol.integr.I1, w1b0)) + dot(A20, dot(sol.integr.I2, w2b0)) );

        return {energy0, momentum0};
    }

    void construct(const solution &sol)
    {
        clear();

        const size_t N = sol.t.size();
        if (N == 0 || N_SOL2D == 0)
            return;

        size_t N2D = N_SOL2D;
        if (N < N_SOL2D)
            N2D = N;

        double step = 0.0;
        if (N2D != 1)
            step = (N - 1.0)/(double)(N2D - 1.0);

        t.resize(N2D);
        
        x.resize(N2D);       y.resize(N2D);    z.resize(N2D);    dist.resize(N2D);
        vx.resize(N2D);      vy.resize(N2D);   vz.resize(N2D);   vel.resize(N2D);
        sma.resize(N2D);     ecc.resize(N2D);  inc.resize(N2D);  raan.resize(N2D);  argper.resize(N2D);  manom.resize(N2D);
        denergy.resize(N2D); dmomentum.resize(N2D);
        
        roll1.resize(N2D);  pitch1.resize(N2D);  yaw1.resize(N2D);  relyaw1.resize(N2D);
        w1ix.resize(N2D);   w1iy.resize(N2D);    w1iz.resize(N2D);
        w1bx.resize(N2D);   w1by.resize(N2D);    w1bz.resize(N2D);

        roll2.resize(N2D);  pitch2.resize(N2D);  yaw2.resize(N2D);  relyaw2.resize(N2D);       
        w2ix.resize(N2D);   w2iy.resize(N2D);    w2iz.resize(N2D);
        w2bx.resize(N2D);   w2by.resize(N2D);    w2bz.resize(N2D);
        
        if (sol.integr.props.spacecraft_checkbox)
        {
            xsp.resize(N2D);    ysp.resize(N2D);    zsp.resize(N2D);    dist_sp.resize(N2D);
            vxsp.resize(N2D);   vysp.resize(N2D);   vzsp.resize(N2D);   vel_sp.resize(N2D);
            sma_sp.resize(N2D); ecc_sp.resize(N2D); inc_sp.resize(N2D); raan_sp.resize(N2D); argper_sp.resize(N2D); manom_sp.resize(N2D);

            xsp1.resize(N2D);    ysp1.resize(N2D);    zsp1.resize(N2D);    dist_sp1.resize(N2D);
            vxsp1.resize(N2D);   vysp1.resize(N2D);   vzsp1.resize(N2D);   vel_sp1.resize(N2D);
            sma_sp1.resize(N2D); ecc_sp1.resize(N2D); inc_sp1.resize(N2D); raan_sp1.resize(N2D); argper_sp1.resize(N2D); manom_sp1.resize(N2D);

            xsp2.resize(N2D);    ysp2.resize(N2D);    zsp2.resize(N2D);    dist_sp2.resize(N2D);
            vxsp2.resize(N2D);   vysp2.resize(N2D);   vzsp2.resize(N2D);   vel_sp2.resize(N2D);
            sma_sp2.resize(N2D); ecc_sp2.resize(N2D); inc_sp2.resize(N2D); raan_sp2.resize(N2D); argper_sp2.resize(N2D); manom_sp2.resize(N2D);
        }

        dvec2 ener_mom_at_t0 = get_energy_and_momentum_at_t0(sol);
        double energy0 = ener_mom_at_t0[0];
        double momentum0 = ener_mom_at_t0[1];

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
        for (i = 0; i < N2D; ++i)
        {
            //Compute the current reduced index j based on the current 'dense index' i.
            size_t j = (size_t)(i*step);
            if (j >= N)
                j = N-1;

            dvec3 r   = {sol.integr.orbit[j][1],  sol.integr.orbit[j][2],  sol.integr.orbit[j][3]};
            dvec3 v   = {sol.integr.orbit[j][4],  sol.integr.orbit[j][5],  sol.integr.orbit[j][6]};
            dvec4 q1  = {sol.integr.orbit[j][7],  sol.integr.orbit[j][8],  sol.integr.orbit[j][9],  sol.integr.orbit[j][10]};
            dvec3 w1b = {sol.integr.orbit[j][11], sol.integr.orbit[j][12], sol.integr.orbit[j][13]};
            dvec4 q2  = {sol.integr.orbit[j][14], sol.integr.orbit[j][15], sol.integr.orbit[j][16], sol.integr.orbit[j][17]};
            dvec3 w2b = {sol.integr.orbit[j][18], sol.integr.orbit[j][19], sol.integr.orbit[j][20]};

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

            dvec6 kep = cart2kep({r[0],r[1],r[2], v[0],v[1],v[2]}, G*(sol.integr.props.M1 + sol.integr.props.M2));
            
            //Kinetic energy part (evaluated in body frames - that's ok coz energy is scalar and scalars are preserved under rotations) :
            double energy = 0.5*sol.integr.m*dot(v,v) + 0.5*dot( dot(w1b, sol.integr.I1), w1b) + 0.5*dot( dot(w2b, sol.integr.I2), w2b);

            //Potential energy part :
            if (sol.integr.props.ord2_checkbox)
                energy += mut_pot_integrals_ord2(r, sol.integr.props.M1, sol.integr.J1, A1, sol.integr.props.M2, sol.integr.J2, A2);
            else if (sol.integr.props.ord3_checkbox)
                energy += mut_pot_integrals_ord3(r, sol.integr.props.M1, sol.integr.J1, A1, sol.integr.props.M2, sol.integr.J2, A2);
            else
                energy += mut_pot_integrals_ord4(r, sol.integr.props.M1, sol.integr.J1, A1, sol.integr.props.M2, sol.integr.J2, A2);
            
            //Momentum magnitude (now evaluated it in the C.O.M. frame because it is a vector) :
            //Note : All 3 components of the momentum vector are conserved in time, but I just choose to store and plot the magnitude only.
            double momentum = length( sol.integr.m*cross(r,v) + dot(A1, dot(sol.integr.I1, w1b)) + dot(A2, dot(sol.integr.I2, w2b)) );

            t[i] = sol.integr.orbit[j][0]/86400.0; //[days]

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
                denergy[0] = dmomentum[0] = 0.0; //No error initially by default...
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
            if (sol.integr.props.spacecraft_checkbox)
            {
                dvec3 rsp = {sol.integr.orbit[j][21], sol.integr.orbit[j][22], sol.integr.orbit[j][23]};
                dvec3 vsp = {sol.integr.orbit[j][24], sol.integr.orbit[j][25], sol.integr.orbit[j][26]};

                dvec3 rsp1 = rsp - sol.integr.m1*r, vsp1 = vsp - sol.integr.m1*v;
                dvec3 rsp2 = rsp - sol.integr.m2*r, vsp2 = vsp - sol.integr.m2*v;
                dvec6 kep_sp  = cart2kep({ rsp[0],  rsp[1],  rsp[2],   vsp[0],  vsp[1],  vsp[2]}, G*(sol.integr.props.M1 + sol.integr.props.M2));
                dvec6 kep_sp1 = cart2kep({rsp1[0], rsp1[1], rsp1[2],  vsp1[0], vsp1[1], vsp1[2]}, G*sol.integr.props.M1);
                dvec6 kep_sp2 = cart2kep({rsp2[0], rsp2[1], rsp2[2],  vsp2[0], vsp2[1], vsp2[2]}, G*sol.integr.props.M2);

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
    }
};

#endif
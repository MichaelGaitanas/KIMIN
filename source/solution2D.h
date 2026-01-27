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
#include"dynamics.h"
#include"properties.h"
#include"integrator.h"
#include"solution.h"

class solution2D
{
public:
    //The following vector members constitute the REDUCED version of the solution of the ODEs and are used only for GUI plotting.
    
    dvec t;

    //Mutual state of the binary :
    dvec xmut,  ymut,  zmut,  dist_mut;
    dvec vxmut, vymut, vzmut, vel_mut;
    dvec sma_mut, ecc_mut, inc_mut, raan_mut, argper_mut, manom_mut;
    dvec denergy, dmomentum; //Relative errors, i.e. |(E[i] - E[0])/E[0]| and |(L[i] - L[0])/L[0]| regarding the mutual dynamics.

    //Body 1 :
    dvec roll1, pitch1, yaw1, relyaw1;
    dvec w1ix, w1iy, w1iz;
    dvec w1bx, w1by, w1bz;

    //Body 2 :
    dvec roll2, pitch2, yaw2, relyaw2;
    dvec w2ix, w2iy, w2iz;
    dvec w2bx, w2by, w2bz;

    //COM in Heliocentric frame :
    dvec xcom_helio,  ycom_helio,  zcom_helio,  dist_com_helio;
    dvec vxcom_helio, vycom_helio, vzcom_helio, vel_com_helio;
    dvec sma_com_helio, ecc_com_helio, inc_com_helio, raan_com_helio, argper_com_helio, manom_com_helio;

    //Spacecraft :
    //In Heliocentric frame :
    dvec xsp_helio,  ysp_helio,  zsp_helio,  dist_sp_helio;
    dvec vxsp_helio, vysp_helio, vzsp_helio, vel_sp_helio;
    dvec sma_sp_helio, ecc_sp_helio, inc_sp_helio, raan_sp_helio, argper_sp_helio, manom_sp_helio;
    //In binary's COM frame :
    dvec xsp_com,  ysp_com,  zsp_com,  dist_sp_com;
    dvec vxsp_com, vysp_com, vzsp_com, vel_sp_com;
    dvec sma_sp_com, ecc_sp_com, inc_sp_com, raan_sp_com, argper_sp_com, manom_sp_com;
    //In body 1 frame :
    dvec xsp_com1,  ysp_com1,  zsp_com1,  dist_sp_com1;
    dvec vxsp_com1, vysp_com1, vzsp_com1, vel_sp_com1;
    dvec sma_sp_com1, ecc_sp_com1, inc_sp_com1, raan_sp_com1, argper_sp_com1, manom_sp_com1;
    //In body 2 frame :
    dvec xsp_com2,  ysp_com2,  zsp_com2,  dist_sp_com2;
    dvec vxsp_com2, vysp_com2, vzsp_com2, vel_sp_com2;
    dvec sma_sp_com2, ecc_sp_com2, inc_sp_com2, raan_sp_com2, argper_sp_com2, manom_sp_com2;
    //Rotation :
    dvec roll_sp, pitch_sp, yaw_sp;
    dvec wix_sp, wiy_sp, wiz_sp;
    dvec wbx_sp, wby_sp, wbz_sp;

    solution2D() { }

    void clear()
    {
        t.clear();

        xmut.clear();          ymut.clear();          zmut.clear();    dist_mut.clear();
        vxmut.clear();         vymut.clear();         vzmut.clear();   vel_mut.clear();
        sma_mut.clear();       ecc_mut.clear();       inc_mut.clear(); raan_mut.clear(); argper_mut.clear(); manom_mut.clear();
        denergy.clear();       dmomentum.clear();
        roll1.clear();         pitch1.clear();        yaw1.clear(); relyaw1.clear();
        w1ix.clear();          w1iy.clear();          w1iz.clear();
        w1bx.clear();          w1by.clear();          w1bz.clear();
        roll2.clear();         pitch2.clear();        yaw2.clear(); relyaw2.clear();
        w2ix.clear();          w2iy.clear();          w2iz.clear();
        w2bx.clear();          w2by.clear();          w2bz.clear();
        xcom_helio.clear();    ycom_helio.clear();    zcom_helio.clear();  dist_com_helio.clear();
        vxcom_helio.clear();   vycom_helio.clear();   vzcom_helio.clear(); vel_com_helio.clear();
        sma_com_helio.clear(); ecc_com_helio.clear(); inc_com_helio.clear(); raan_com_helio.clear(); argper_com_helio.clear(); manom_com_helio.clear();

        xsp_helio.clear();     ysp_helio.clear();     zsp_helio.clear();     dist_sp_helio.clear();
        vxsp_helio.clear();    vysp_helio.clear();    vzsp_helio.clear();    vel_sp_helio.clear();
        sma_sp_helio.clear();  ecc_sp_helio.clear();  inc_sp_helio.clear();  raan_sp_helio.clear(); argper_sp_helio.clear();  manom_sp_helio.clear();
        xsp_com.clear();       ysp_com.clear();       zsp_com.clear();       dist_sp_com.clear();
        vxsp_com.clear();      vysp_com.clear();      vzsp_com.clear();      vel_sp_com.clear();
        sma_sp_com.clear();    ecc_sp_com.clear();    inc_sp_com.clear();    raan_sp_com.clear();   argper_sp_com.clear();  manom_sp_com.clear();
        xsp_com1.clear();      ysp_com1.clear();      zsp_com1.clear();      dist_sp_com1.clear();
        vxsp_com1.clear();     vysp_com1.clear();     vzsp_com1.clear();     vel_sp_com1.clear();
        sma_sp_com1.clear();   ecc_sp_com1.clear();   inc_sp_com1.clear();   raan_sp_com1.clear();  argper_sp_com1.clear(); manom_sp_com1.clear();
        xsp_com2.clear();      ysp_com2.clear();      zsp_com2.clear();      dist_sp_com2.clear();
        vxsp_com2.clear();     vysp_com2.clear();     vzsp_com2.clear();     vel_sp_com2.clear();
        sma_sp_com2.clear();   ecc_sp_com2.clear();   inc_sp_com2.clear();   raan_sp_com2.clear();  argper_sp_com2.clear(); manom_sp_com2.clear();
        roll_sp.clear();       pitch_sp.clear();      yaw_sp.clear();
        wix_sp.clear();        wiy_sp.clear();        wiz_sp.clear();
        wbx_sp.clear();        wby_sp.clear();        wbz_sp.clear();
    }

    dvec2 get_energy_and_momentum_at_t0(const solution &sol)
    {
        const dvec3 r0   = {sol.integr.orbit[0][1],  sol.integr.orbit[0][2],  sol.integr.orbit[0][3]};
        const dvec3 v0   = {sol.integr.orbit[0][4],  sol.integr.orbit[0][5],  sol.integr.orbit[0][6]};
        const dvec4 q10  = {sol.integr.orbit[0][7],  sol.integr.orbit[0][8],  sol.integr.orbit[0][9],  sol.integr.orbit[0][10]};
        const dvec3 w1b0 = {sol.integr.orbit[0][11], sol.integr.orbit[0][12], sol.integr.orbit[0][13]};
        const dvec4 q20  = {sol.integr.orbit[0][14], sol.integr.orbit[0][15], sol.integr.orbit[0][16], sol.integr.orbit[0][17]};
        const dvec3 w2b0 = {sol.integr.orbit[0][18], sol.integr.orbit[0][19], sol.integr.orbit[0][20]};

        const dmat3 A10 = quat2mat(q10);
        const dmat3 A20 = quat2mat(q20);

        double E0 = 0.5*sol.integr.m*dot(v0,v0) + 0.5*dot( dot(w1b0, sol.integr.I1), w1b0) + 0.5*dot( dot(w2b0, sol.integr.I2), w2b0);
        if (sol.integr.props.ord2_checkbox)
            E0 += mut_pot_integrals_ord2(r0, sol.integr.props.M1, sol.integr.J1, A10,
                                             sol.integr.props.M2, sol.integr.J2, A20);
        else if (sol.integr.props.ord3_checkbox)
            E0 += mut_pot_integrals_ord3(r0, sol.integr.props.M1, sol.integr.J1, A10,
                                             sol.integr.props.M2, sol.integr.J2, A20);
        else
            E0 += mut_pot_integrals_ord4(r0, sol.integr.props.M1, sol.integr.J1, A10,
                                             sol.integr.props.M2, sol.integr.J2, A20);

        const double L0 = length( sol.integr.m*cross(r0,v0) + dot(A10, dot(sol.integr.I1, w1b0)) + dot(A20, dot(sol.integr.I2, w2b0)) );

        return {E0, L0};
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
        
        xmut.resize(N2D);    ymut.resize(N2D);    zmut.resize(N2D);    dist_mut.resize(N2D);
        vxmut.resize(N2D);   vymut.resize(N2D);   vzmut.resize(N2D);   vel_mut.resize(N2D);
        sma_mut.resize(N2D); ecc_mut.resize(N2D); inc_mut.resize(N2D); raan_mut.resize(N2D); argper_mut.resize(N2D); manom_mut.resize(N2D);
        denergy.resize(N2D); dmomentum.resize(N2D);
        
        roll1.resize(N2D);  pitch1.resize(N2D);  yaw1.resize(N2D);  relyaw1.resize(N2D);
        w1ix.resize(N2D);   w1iy.resize(N2D);    w1iz.resize(N2D);
        w1bx.resize(N2D);   w1by.resize(N2D);    w1bz.resize(N2D);

        roll2.resize(N2D);  pitch2.resize(N2D);  yaw2.resize(N2D);  relyaw2.resize(N2D);       
        w2ix.resize(N2D);   w2iy.resize(N2D);    w2iz.resize(N2D);
        w2bx.resize(N2D);   w2by.resize(N2D);    w2bz.resize(N2D);

        xcom_helio.resize(N2D);    ycom_helio.resize(N2D);    zcom_helio.resize(N2D);    dist_com_helio.resize(N2D);
        vxcom_helio.resize(N2D);   vycom_helio.resize(N2D);   vzcom_helio.resize(N2D);   vel_com_helio.resize(N2D);
        sma_com_helio.resize(N2D); ecc_com_helio.resize(N2D); inc_com_helio.resize(N2D); raan_com_helio.resize(N2D); argper_com_helio.resize(N2D); manom_com_helio.resize(N2D);
        
        if (sol.integr.props.spacecraft_checkbox)
        {
            xsp_helio.resize(N2D);      ysp_helio.resize(N2D);      zsp_helio.resize(N2D);      dist_sp_helio.resize(N2D);
            vxsp_helio.resize(N2D);     vysp_helio.resize(N2D);     vzsp_helio.resize(N2D);     vel_sp_helio.resize(N2D);
            sma_sp_helio.resize(N2D);   ecc_sp_helio.resize(N2D);   inc_sp_helio.resize(N2D);   raan_sp_helio.resize(N2D); argper_sp_helio.resize(N2D);  manom_sp_helio.resize(N2D);
            xsp_com.resize(N2D);        ysp_com.resize(N2D);        zsp_com.resize(N2D);        dist_sp_com.resize(N2D);
            vxsp_com.resize(N2D);       vysp_com.resize(N2D);       vzsp_com.resize(N2D);       vel_sp_com.resize(N2D);
            sma_sp_com.resize(N2D);     ecc_sp_com.resize(N2D);     inc_sp_com.resize(N2D);     raan_sp_com.resize(N2D);  argper_sp_com.resize(N2D);  manom_sp_com.resize(N2D);
            xsp_com1.resize(N2D);       ysp_com1.resize(N2D);       zsp_com1.resize(N2D);       dist_sp_com1.resize(N2D);
            vxsp_com1.resize(N2D);      vysp_com1.resize(N2D);      vzsp_com1.resize(N2D);      vel_sp_com1.resize(N2D);
            sma_sp_com1.resize(N2D);    ecc_sp_com1.resize(N2D);    inc_sp_com1.resize(N2D);    raan_sp_com1.resize(N2D); argper_sp_com1.resize(N2D); manom_sp_com1.resize(N2D);
            xsp_com2.resize(N2D);       ysp_com2.resize(N2D);       zsp_com2.resize(N2D);       dist_sp_com2.resize(N2D);
            vxsp_com2.resize(N2D);      vysp_com2.resize(N2D);      vzsp_com2.resize(N2D);      vel_sp_com2.resize(N2D);
            sma_sp_com2.resize(N2D);    ecc_sp_com2.resize(N2D);    inc_sp_com2.resize(N2D);    raan_sp_com2.resize(N2D); argper_sp_com2.resize(N2D); manom_sp_com2.resize(N2D);
            if (sol.integr.props.sp_is_rigidbody_checkbox)
            {
                roll_sp.resize(N2D); pitch_sp.resize(N2D); yaw_sp.resize(N2D);
                wix_sp.resize(N2D);  wiy_sp.resize(N2D);   wiz_sp.resize(N2D);
                wbx_sp.resize(N2D);  wby_sp.resize(N2D);   wbz_sp.resize(N2D);
            }
        }

        const dvec2 ener_mom_at_t0 = get_energy_and_momentum_at_t0(sol);
        const double energy0 = ener_mom_at_t0[0];
        const double momentum0 = ener_mom_at_t0[1];

        #ifdef _OPENMP
            const int total_threads = omp_get_max_threads();
            const int used_threads  = (total_threads > 1 ? total_threads/2 : 1);
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

            const dvec3 rmut       = {sol.integr.orbit[j][1],  sol.integr.orbit[j][2],  sol.integr.orbit[j][3]};
            const dvec3 vmut       = {sol.integr.orbit[j][4],  sol.integr.orbit[j][5],  sol.integr.orbit[j][6]};
            const dvec4 q1         = {sol.integr.orbit[j][7],  sol.integr.orbit[j][8],  sol.integr.orbit[j][9],  sol.integr.orbit[j][10]};
            const dvec3 w1b        = {sol.integr.orbit[j][11], sol.integr.orbit[j][12], sol.integr.orbit[j][13]};
            const dvec4 q2         = {sol.integr.orbit[j][14], sol.integr.orbit[j][15], sol.integr.orbit[j][16], sol.integr.orbit[j][17]};
            const dvec3 w2b        = {sol.integr.orbit[j][18], sol.integr.orbit[j][19], sol.integr.orbit[j][20]};
            const dvec3 rcom_helio = {sol.integr.orbit[j][21], sol.integr.orbit[j][22], sol.integr.orbit[j][23]};
            const dvec3 vcom_helio = {sol.integr.orbit[j][24], sol.integr.orbit[j][25], sol.integr.orbit[j][26]};

            const dmat3 A1   = quat2mat(q1);
            const dmat3 A2   = quat2mat(q2);
            const dvec3 w1i  = body2iner(w1b,A1);
            const dvec3 w2i  = body2iner(w2b,A2);
            const dvec3 rpy1 = quat2ang(q1);
            const dvec3 rpy2 = quat2ang(q2);

            const dvec3 rcyl = cart2cyl(rmut);
            
            //Calculate libration angle (relative yaw) of body 1 & 2 :
            double temp = rpy1[2] - rcyl[1]; //phi1 = thita1z - thita
            while (temp > PI)
                temp -= 2.0*PI;
            while (temp <= -PI)
                temp += 2.0*PI;
            const double libration1 = temp;
            temp = rpy2[2] - rcyl[1];  //phi2 = thita2z - thita
            while (temp > PI)
                temp -= 2.0*PI;
            while (temp <= -PI)
                temp += 2.0*PI;
            const double libration2 = temp;

            const dvec6 kep_mut = cart2kep({rmut[0],rmut[1],rmut[2], vmut[0],vmut[1],vmut[2]}, G*(sol.integr.props.M1 + sol.integr.props.M2));
            const dvec6 kep_com_helio = cart2kep({rcom_helio[0],rcom_helio[1],rcom_helio[2], vcom_helio[0],vcom_helio[1],vcom_helio[2]}, G*MSUN);
            
            //Kinetic energy part (evaluated in body frames - that's ok coz energy is scalar and scalars are preserved under rotations) :
            double energy = 0.5*sol.integr.m*dot(vmut,vmut) + 0.5*dot( dot(w1b, sol.integr.I1), w1b) + 0.5*dot( dot(w2b, sol.integr.I2), w2b);

            //Potential energy part :
            if (sol.integr.props.ord2_checkbox)
                energy += mut_pot_integrals_ord2(rmut, sol.integr.props.M1, sol.integr.J1, A1, sol.integr.props.M2, sol.integr.J2, A2);
            else if (sol.integr.props.ord3_checkbox)
                energy += mut_pot_integrals_ord3(rmut, sol.integr.props.M1, sol.integr.J1, A1, sol.integr.props.M2, sol.integr.J2, A2);
            else
                energy += mut_pot_integrals_ord4(rmut, sol.integr.props.M1, sol.integr.J1, A1, sol.integr.props.M2, sol.integr.J2, A2);
            
            //Momentum magnitude (now evaluated it in the inertial (Heliocentric) frame because it is a vector) :
            //Note : All 3 components of the momentum vector are conserved in time, but I just choose to store and plot the magnitude only.
            const double momentum = length( sol.integr.m*cross(rmut,vmut) + dot(A1, dot(sol.integr.I1, w1b)) + dot(A2, dot(sol.integr.I2, w2b)) );

            t[i] = (sol.integr.orbit[j][0] - sol.integr.t0)/DAY2SEC; //Back in [days].

            xmut[i]     = rmut[0];
            ymut[i]     = rmut[1];
            zmut[i]     = rmut[2];
            dist_mut[i] = length(rmut);

            vxmut[i]   = vmut[0];
            vymut[i]   = vmut[1];
            vzmut[i]   = vmut[2];
            vel_mut[i] = length(vmut);

            sma_mut[i]    = kep_mut[0];
            ecc_mut[i]    = kep_mut[1];
            inc_mut[i]    = kep_mut[2]*180.0/PI;
            raan_mut[i]   = kep_mut[3]*180.0/PI;
            argper_mut[i] = kep_mut[4]*180.0/PI;
            manom_mut[i]  = kep_mut[5]*180.0/PI;

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

            //We directly store them in [AU] for the gui plots.
            xcom_helio[i]     = rcom_helio[0]/AU2KM;
            ycom_helio[i]     = rcom_helio[1]/AU2KM;
            zcom_helio[i]     = rcom_helio[2]/AU2KM;
            dist_com_helio[i] = length(rcom_helio)/AU2KM;

            //Velocities remain in [km/sec].
            vxcom_helio[i]    = vcom_helio[0];
            vycom_helio[i]    = vcom_helio[1];
            vzcom_helio[i]    = vcom_helio[2];
            vel_com_helio[i]  = length(vcom_helio);

            sma_com_helio[i]    = kep_com_helio[0];
            ecc_com_helio[i]    = kep_com_helio[1];
            inc_com_helio[i]    = kep_com_helio[2]*180.0/PI;
            raan_com_helio[i]   = kep_com_helio[3]*180.0/PI;
            argper_com_helio[i] = kep_com_helio[4]*180.0/PI;
            manom_com_helio[i]  = kep_com_helio[5]*180.0/PI;

            //Spacecraft :
            if (sol.integr.props.spacecraft_checkbox)
            {
                //Spacecraft in Heliocentric frame.
                const dvec3 rsp_helio = {sol.integr.orbit[j][27], sol.integr.orbit[j][28], sol.integr.orbit[j][29]};
                const dvec3 vsp_helio = {sol.integr.orbit[j][30], sol.integr.orbit[j][31], sol.integr.orbit[j][32]};
                const dvec6 kep_sp_helio = cart2kep({rsp_helio[0],rsp_helio[1],rsp_helio[2], vsp_helio[0],vsp_helio[1],vsp_helio[2]}, G*MSUN);

                //Spacecraft in binary's COM frame.
                const dvec3 rsp_com = rsp_helio - rcom_helio;
                const dvec3 vsp_com = vsp_helio - vcom_helio;
                const dvec6 kep_sp_com = cart2kep({rsp_com[0],rsp_com[1],rsp_com[2], vsp_com[0],vsp_com[1],vsp_com[2]}, G*(sol.integr.props.M1 + sol.integr.props.M2));

                const dvec3 rsp_com1 = rsp_helio - rcom_helio - sol.integr.m1*rmut;
                const dvec3 vsp_com1 = vsp_helio - vcom_helio - sol.integr.m1*vmut;
                const dvec6 kep_sp_com1 = cart2kep({rsp_com1[0],rsp_com1[1],rsp_com1[2], vsp_com1[0],vsp_com1[1],vsp_com1[2]}, G*sol.integr.props.M1);
                
                const dvec3 rsp_com2 = rsp_helio - rcom_helio - sol.integr.m2*rmut;
                const dvec3 vsp_com2 = vsp_helio - vcom_helio - sol.integr.m2*vmut;
                const dvec6 kep_sp_com2 = cart2kep({rsp_com2[0],rsp_com2[1],rsp_com2[2], vsp_com2[0],vsp_com2[1],vsp_com2[2]}, G*sol.integr.props.M2);

                //Heliocentric is position plotted in [AU].
                xsp_helio[i]     = rsp_helio[0]/AU2KM;
                ysp_helio[i]     = rsp_helio[1]/AU2KM;
                zsp_helio[i]     = rsp_helio[2]/AU2KM;
                dist_sp_helio[i] = length(rsp_helio)/AU2KM;

                //These remain in [km/sec].
                vxsp_helio[i]   = vsp_helio[0];
                vysp_helio[i]   = vsp_helio[1];
                vzsp_helio[i]   = vsp_helio[2];
                vel_sp_helio[i] = length(vsp_helio);

                sma_sp_helio[i]    = kep_sp_helio[0]/AU2KM; //Also in [AU].
                ecc_sp_helio[i]    = kep_sp_helio[1];
                inc_sp_helio[i]    = kep_sp_helio[2]*180.0/PI;
                raan_sp_helio[i]   = kep_sp_helio[3]*180.0/PI;
                argper_sp_helio[i] = kep_sp_helio[4]*180.0/PI;
                manom_sp_helio[i]  = kep_sp_helio[5]*180.0/PI;

                ///////////////////////////////////////////////////////////
                
                //Those remain in [km].
                xsp_com[i]     = rsp_com[0];
                ysp_com[i]     = rsp_com[1];
                zsp_com[i]     = rsp_com[2];
                dist_sp_com[i] = length(rsp_com);

                //[km/sec]
                vxsp_com[i]   = vsp_com[0];
                vysp_com[i]   = vsp_com[1];
                vzsp_com[i]   = vsp_com[2];
                vel_sp_com[i] = length(vsp_com);

                sma_sp_com[i]    = kep_sp_com[0]; //[km]
                ecc_sp_com[i]    = kep_sp_com[1];
                inc_sp_com[i]    = kep_sp_com[2]*180.0/PI;
                raan_sp_com[i]   = kep_sp_com[3]*180.0/PI;
                argper_sp_com[i] = kep_sp_com[4]*180.0/PI;
                manom_sp_com[i]  = kep_sp_com[5]*180.0/PI;

                ///////////////////////////////////////////////////////////

                //Those remain in [km].
                xsp_com1[i]     = rsp_com1[0];
                ysp_com1[i]     = rsp_com1[1];
                zsp_com1[i]     = rsp_com1[2];
                dist_sp_com1[i] = length(rsp_com1);

                //[km/sec]
                vxsp_com1[i]   = vsp_com1[0];
                vysp_com1[i]   = vsp_com1[1];
                vzsp_com1[i]   = vsp_com1[2];
                vel_sp_com1[i] = length(vsp_com1);

                sma_sp_com1[i]    = kep_sp_com1[0]; //[km]
                ecc_sp_com1[i]    = kep_sp_com1[1];
                inc_sp_com1[i]    = kep_sp_com1[2]*180.0/PI;
                raan_sp_com1[i]   = kep_sp_com1[3]*180.0/PI;
                argper_sp_com1[i] = kep_sp_com1[4]*180.0/PI;
                manom_sp_com1[i]  = kep_sp_com1[5]*180.0/PI;

                ///////////////////////////////////////////////////////////

                //[km]
                xsp_com2[i]     = rsp_com2[0];
                ysp_com2[i]     = rsp_com2[1];
                zsp_com2[i]     = rsp_com2[2];
                dist_sp_com2[i] = length(rsp_com2);

                //[km/sec]
                vxsp_com2[i]   = vsp_com2[0];
                vysp_com2[i]   = vsp_com2[1];
                vzsp_com2[i]   = vsp_com2[2];
                vel_sp_com2[i] = length(vsp_com2);

                sma_sp_com2[i]    = kep_sp_com2[0]; //[km]
                ecc_sp_com2[i]    = kep_sp_com2[1];
                inc_sp_com2[i]    = kep_sp_com2[2]*180.0/PI;
                raan_sp_com2[i]   = kep_sp_com2[3]*180.0/PI;
                argper_sp_com2[i] = kep_sp_com2[4]*180.0/PI;
                manom_sp_com2[i]  = kep_sp_com2[5]*180.0/PI;

                if (sol.integr.props.sp_is_rigidbody_checkbox)
                {
                    const dvec4 qsp   = {sol.integr.orbit[j][33], sol.integr.orbit[j][34], sol.integr.orbit[j][35], sol.integr.orbit[j][36]};
                    const dvec3 wb_sp = {sol.integr.orbit[j][37], sol.integr.orbit[j][38], sol.integr.orbit[j][39]};

                    const dmat3 Asp    = quat2mat(qsp);
                    const dvec3 wi_sp  = body2iner(wb_sp, Asp);
                    const dvec3 rpy_sp = quat2ang(qsp);

                    wix_sp[i] = wi_sp[0];
                    wiy_sp[i] = wi_sp[1];
                    wiz_sp[i] = wi_sp[2];

                    wbx_sp[i] = wb_sp[0];
                    wby_sp[i] = wb_sp[1];
                    wbz_sp[i] = wb_sp[2];

                    roll_sp[i]  = rpy_sp[0]*180.0/PI;
                    pitch_sp[i] = rpy_sp[1]*180.0/PI;
                    yaw_sp[i]   = rpy_sp[2]*180.0/PI;
                }
            }
        }
    }
};

#endif
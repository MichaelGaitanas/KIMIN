#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

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
#include"solution.hpp"

#include<boost/numeric/odeint.hpp>

class Integrator : public Properties
{

public:

    dmatnx3 masc1, masc2; //mascon cloud coordinates
    dtens J1, J2; //inertial integrals
    dmat3 I1, I2; //moments of inertia

    bool collision; //binary collision flag
    dmat msol; //minimal solution matrix of the binary

    Integrator(const Properties &properties) : Properties(properties)
    {
        if (ell_checkbox)
        {
            if (ord2_checkbox)
            {
                J1 = ell_integrals(M1, semiaxes1, 2);
                J2 = ell_integrals(M2, semiaxes2, 2);
                I1 = ell_inertia(M1, semiaxes1);
                I2 = ell_inertia(M2, semiaxes2);
            }
            else if (ord3_checkbox)
            {
                J1 = ell_integrals(M1, semiaxes1, 3);
                J2 = ell_integrals(M2, semiaxes2, 3);
                I1 = ell_inertia(M1, semiaxes1);
                I2 = ell_inertia(M2, semiaxes2);
            }
            else if (ord4_checkbox)
            {
                J1 = ell_integrals(M1, semiaxes1, 4);
                J2 = ell_integrals(M2, semiaxes2, 4);
                I1 = ell_inertia(M1, semiaxes1);
                I2 = ell_inertia(M2, semiaxes2);
            }
            else
            {
                masc1 = fill_ell_with_masc(semiaxes1, grid_reso1);
                masc2 = fill_ell_with_masc(semiaxes2, grid_reso2);
                I1 = masc_inertia(M1, masc1);
                I2 = masc_inertia(M2, masc2);
            }
        }
        else //obj_checkbox
        {
            Obj poly1(obj_path1.c_str());
            masc1 = poly1.fill_with_masc(grid_reso1);
            correct_masc_com(masc1);
            correct_masc_inertia(M1, masc1);

            Obj poly2(obj_path2.c_str());
            masc2 = poly2.fill_with_masc(grid_reso2);
            correct_masc_com(masc2);
            correct_masc_inertia(M2, masc2);

            if (ord2_checkbox)
            {
                J1 = masc_integrals(M1, masc1, 2);
                J2 = masc_integrals(M2, masc2, 2);
                I1 = masc_inertia(M1, masc1);
                I2 = masc_inertia(M2, masc2);
            }
            else if (ord3_checkbox)
            {
                J1 = masc_integrals(M1, masc1, 3);
                J2 = masc_integrals(M2, masc2, 3);
                I1 = masc_inertia(M1, masc1);
                I2 = masc_inertia(M2, masc2);
            }
            else if (ord4_checkbox)
            {
                J1 = masc_integrals(M1, masc1, 4);
                J2 = masc_integrals(M2, masc2, 4);
                I1 = masc_inertia(M1, masc1);
                I2 = masc_inertia(M2, masc2);
            }
            //If pure mascons theory is selected, no additional parameter must be computed. masc1[][] and masc2[][] suffice. 
        }
        
        if (cart_kep_var_choice == 1) //the user chose Keplerian elements as ic
            cart = kep2cart(kep, G*(M1+M2)); //transform Keplerian to Cartesian because the f2bp odes is written in Cartesian elements

        if (orient_var_choice == 0) //orientation variables were chosen to be Euler angles
        {
            //transform Euler angles (roll, pitch, yaw) to quaternions because the f2bp odes is written in quaternionic structure
            q1 = ang2quat(rpy1);
            q2 = ang2quat(rpy2);
        }

        if (frame_type_choice == 0) //angular velocities were given in the "world" inertial frame
        {
            //transform inertial angular velocities to the corresponding body ones because the Euler equations of rotation are written in the body frames
            w1b = iner2body(w1i, quat2mat(q1));
            w2b = iner2body(w2i, quat2mat(q2));
        }
    }
    
    void build_rhs(const boost::array<double, 20> &state, boost::array<double, 20> &dstate, double t)
    {
        //split the state vector
        dvec3 r  =  { state[0],  state[1],  state[2] };
        dvec3 v  =  { state[3],  state[4],  state[5] };
        dvec4 q1 =  { state[6],  state[7],  state[8],  state[9] };
        dvec3 w1b = { state[10], state[11], state[12] };
        dvec4 q2 =  { state[13], state[14], state[15], state[16] };
        dvec3 w2b = { state[17], state[18], state[19] };

        //normalize the quaternions
        q1 = quat2unit(q1);
        q2 = quat2unit(q2);

        //construct the rotation matrices
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);

        //calculate the foce and the torques, depending on the user's choice of the theory
        dvec3 force, tau1i;
        if (ord2_checkbox)
        {
            force = mut_force_integrals_ord2(r, M1,J1,A1, M2,J2,A2);
            tau1i = mut_torque_integrals_ord2(r, J1,A1, M2);
        }
        else if (ord3_checkbox)
        {
            force = mut_force_integrals_ord3(r, M1,J1,A1, M2,J2,A2);
            tau1i = mut_torque_integrals_ord3(r, J1,A1, M2);
        } 
        else if (ord4_checkbox)
        {
            force = mut_force_integrals_ord4(r, M1,J1,A1, M2,J2,A2);
            tau1i = mut_torque_integrals_ord4(r, M1,J1,A1, M2,J2,A2);
        }
        else
        {
            force = mut_force_masc(r, M1,masc1,A1, M2,masc2,A2);
            tau1i = mut_torque_masc(r, M1,masc1,A1, M2,masc2,A2);
        }

        dvec3 tau2i = -tau1i - cross(r,force);

        dvec3 tau1b = iner2body(tau1i,A1);
        dvec3 tau2b = iner2body(tau2i,A2);

        dvec4 dq1 = quat_rhs(q1,w1b);
        dvec3 dw1b = euler_rhs(w1b,I1,tau1b);

        dvec4 dq2 = quat_rhs(q2,w2b);
        dvec3 dw2b = euler_rhs(w2b,I2,tau2b);

        //relative position rhs (x,y,z)
        dstate[0] = v[0];
        dstate[1] = v[1];
        dstate[2] = v[2];

        //relative velocity rhs (vx,vy,vz)
        dstate[3] = force[0]/(M1*M2/(M1+M2));
        dstate[4] = force[1]/(M1*M2/(M1+M2));
        dstate[5] = force[2]/(M1*M2/(M1+M2));

        //quaternion rhs of rigid body 1 (q10,q11,q12,q13)
        dstate[6] = dq1[0];
        dstate[7] = dq1[1];
        dstate[8] = dq1[2];
        dstate[9] = dq1[3];

        //Euler rhs of rigid body 1 (w11,w12,w13)
        dstate[10] = dw1b[0];
        dstate[11] = dw1b[1];
        dstate[12] = dw1b[2];

        //quaternion odes of rigid body 2 (q20,q21,q22,q23)
        dstate[13] = dq2[0];
        dstate[14] = dq2[1];
        dstate[15] = dq2[2];
        dstate[16] = dq2[3];

        //Euler rhs of rigid body 2 (w21,w22,w23)
        dstate[17] = dw2b[0];
        dstate[18] = dw2b[1];
        dstate[19] = dw2b[2];

        return;
    }

    Solution run()
    {
        //initial conditions (boost::array<> must be used to call the integration method)
        boost::array<double, 20> state = { cart[0], cart[1], cart[2],
                                           cart[3], cart[4], cart[5],
                                             q1[0],   q1[1],   q1[2], q1[3],
                                            w1b[0],  w1b[1],  w1b[2],
                                             q2[0],   q2[1],   q2[2], q2[3],
                                            w2b[0],  w2b[1],  w2b[2] };

        double t0 = epoch*86400.0; //[sec]
        double tmax = t0 + dur*86400.0; //[sec]
        double dt = step*86400.0; //[sec]

        collision = false; //no collision initially
        msol.clear();

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>> rkf78; //integration method

        //integration loop
        for (double t = t0; t <= tmax; t += dt)
        {
            //1) save current state
            msol.push_back({t,
                            state[0],
                            state[1],
                            state[2],
                            state[3],
                            state[4],
                            state[5],
                            state[6],
                            state[7],
                            state[8],
                            state[9],
                            state[10],
                            state[11],
                            state[12],
                            state[13],
                            state[14],
                            state[15],
                            state[16],
                            state[17],
                            state[18],
                            state[19]});

            //2) check for sphere-sphere collision detection
            if (sph_sph_collision(length(dvec3{state[0],state[1],state[2]}), ell_brillouin(semiaxes1), ell_brillouin(semiaxes2)))
            {
                collision = true;
                break; //stop the integration
            }

            //3) update the state vector
            rkf78.do_step(std::bind(&Integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
        }

        Solution solution;
        double ener0, mom0;
        for (int i = 0; i < msol.size(); ++i)
        {
            double t = msol[i][0];
            dvec3 r = {msol[i][1], msol[i][2], msol[i][3]};
            dvec3 v = {msol[i][4], msol[i][5], msol[i][6]};
            dvec4 q1 = {msol[i][7], msol[i][8], msol[i][9], msol[i][10]};
            dvec3 w1b = {msol[i][11], msol[i][12], msol[i][13]};
            dvec4 q2 = {msol[i][14], msol[i][15], msol[i][16], msol[i][17]};
            dvec3 w2b = {msol[i][18], msol[i][19], msol[i][20]};
            dmat3 A1 = quat2mat(q1);
            dmat3 A2 = quat2mat(q2);
            dvec3 w1i = body2iner(w1b,A1);
            dvec3 w2i = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);
            dvec6 kep = cart2kep({r[0],r[1],r[2], v[0],v[1],v[2]}, G*(M1+M2));
            
            double ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b);
            if (ord2_checkbox)
                ener += mut_pot_integrals_ord2(r, M1,J1,A1, M2,J2,A2);
            else if (ord3_checkbox)
                ener += mut_pot_integrals_ord3(r, M1,J1,A1, M2,J2,A2);
            else if (ord4_checkbox)
                ener += mut_pot_integrals_ord4(r, M1,J1,A1, M2,J2,A2);
            else
                ener += mut_pot_masc(r, M1,masc1,A1, M2,masc2,A2);
            
            double mom = length( (M1*M2/(M1+M2))*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b)) );
            
            if (i == 0)
            {
                ener0 = ener;
                mom0 = mom;
            }
            
            solution.t.push_back(t);
            solution.x.push_back(r[0]);
            solution.y.push_back(r[1]);
            solution.z.push_back(r[2]);
            solution.dist.push_back(length(r));
            solution.roll1.push_back(rpy1[0]);
            solution.pitch1.push_back(rpy1[1]);
            solution.yaw1.push_back(rpy1[2]);
            solution.roll2.push_back(rpy2[0]);
            solution.pitch2.push_back(rpy2[1]);
            solution.yaw2.push_back(rpy2[2]);
            solution.q10.push_back(q1[0]);
            solution.q11.push_back(q1[1]);
            solution.q12.push_back(q1[2]);
            solution.q13.push_back(q1[3]);
            solution.q20.push_back(q2[0]);
            solution.q21.push_back(q2[1]);
            solution.q22.push_back(q2[2]);
            solution.q23.push_back(q2[3]);
            solution.vx.push_back(v[0]);
            solution.vy.push_back(v[1]);
            solution.vz.push_back(v[2]);
            solution.vmag.push_back(length(v));
            solution.w1ix.push_back(w1i[0]);
            solution.w1iy.push_back(w1i[1]);
            solution.w1iz.push_back(w1i[2]);
            solution.w1bx.push_back(w1b[0]);
            solution.w1by.push_back(w1b[1]);
            solution.w1bz.push_back(w1b[2]);
            solution.w2ix.push_back(w2i[0]);
            solution.w2iy.push_back(w2i[1]);
            solution.w2iz.push_back(w2i[2]);
            solution.w2bx.push_back(w2b[0]);
            solution.w2by.push_back(w2b[1]);
            solution.w2bz.push_back(w2b[2]);
            solution.a.push_back(kep[0]);
            solution.e.push_back(kep[1]);
            solution.inc.push_back(kep[2]);
            solution.Om.push_back(kep[3]);
            solution.w.push_back(kep[4]);
            solution.M.push_back(kep[5]);
            solution.ener_rel_err.push_back(fabs((ener - ener0)/ener0));
            solution.mom_rel_err.push_back(fabs((mom - mom0)/mom0));
        }
        solution.collision = collision;

        return solution;
    }
};


#endif
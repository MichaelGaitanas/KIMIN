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
#include"console.hpp"

#include"properties.hpp"

#include<boost/numeric/odeint.hpp>

class Integrator : public Properties
{

public:

    dmatnx3 masc1, masc2; //mascon cloud coordinates
    dtens J1, J2; //inertial integrals
    dmat3 I1, I2; //moments of inertia

    bool collision; //binary collision flag
    dmat msol; //minimal solution matrix of the binary

    float progress;
    bool is_running;
    bool exists_solution;
    bool force_kill;

    Integrator() { 
        is_running = false;
        exists_solution = false;
        force_kill = false;
    }

    void update_properties(const Properties &properties)
    {
        strcpy(simname, properties.simname);
        ell_checkbox = properties.ell_checkbox;;
        clicked_ell_ok  = properties.clicked_ell_ok  ;
        semiaxes1 = properties.semiaxes1 ;
        semiaxes2 = properties.semiaxes2 ;
        obj_checkbox = properties.obj_checkbox ;
        obj_refer_to_body = properties.obj_refer_to_body ;
        clicked_poly1_index = properties.clicked_poly1_index ;
        clicked_poly2_index = properties.clicked_poly2_index ;
        clicked_poly1 = properties.clicked_poly1 ;
        clicked_poly2 = properties.clicked_poly2 ;
        path_to_poly_obj = properties.path_to_poly_obj ;
        obj_path1 = properties.obj_path1 ;
        obj_path2 = properties.obj_path2 ;
        vf1 = properties.vf1 ;
        vf2 = properties.vf2 ;
        grid_reso1 = properties.grid_reso1 ;
        grid_reso2 = properties.grid_reso2 ;
        grid_reso_inactive = properties.grid_reso_inactive ;
        clicked_obj_ok = properties.clicked_obj_ok ;
        ord2_checkbox = properties.ord2_checkbox ;
        ord3_checkbox = properties.ord3_checkbox ;
        ord4_checkbox = properties.ord4_checkbox ;
        mascons_checkbox = properties.mascons_checkbox ;
        M1 = properties.M1 ;
        M2 = properties.M2 ;
        v_impact = properties.v_impact ;
        M_impact = properties.M_impact ;
        beta = properties.beta ;
        epoch = properties.epoch ;
        dur = properties.dur ;
        step = properties.step ;
        cart_kep_var_choice = properties.cart_kep_var_choice ;
        cart = properties.cart ;
        kep = properties.kep ;
        orient_var_choice = properties.orient_var_choice ;
        rpy1 = properties.rpy1 ;
        rpy2 = properties.rpy2 ;
        q1 = properties.q1 ;
        q2 = properties.q2 ;
        frame_type_choice = properties.frame_type_choice ;
        w1i = properties.w1i ;
        w2i = properties.w2i ;
        w1b = properties.w1b ;
        w2b = properties.w2b ;
    
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
            q1 = ang2quat(rpy1*pi/180.0);
            q2 = ang2quat(rpy2*pi/180.0);
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

    void run(Console & cons)
    {
        char buffer[150];
        cons.timedlog("[Integrator] New simulation started.");
        is_running = true;
        //apply the β correction to the initial relative velocity
        cart[0] += (M1 + M2)*beta*M_impact*v_impact[0]/M2;
        cart[1] += (M1 + M2)*beta*M_impact*v_impact[1]/M2;
        cart[2] += (M1 + M2)*beta*M_impact*v_impact[2]/M2;

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
        int steps = 0;
        int maxsteps = (int)((tmax-t0)/dt + 1.0) ;

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>> rkf78; //integration method
        progress = 0;
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
                sprintf(buffer,"[Integrator] Colision detected at t = %5.2lf days.",t/86400.0);
                cons.timedlog(buffer);
                break; //stop the integration
            }

            //3) check for kill condition
            if(force_kill){
                sprintf(buffer,"[Integrator] Integration stoped at t = %5.2lf days.",t/86400.0);
                cons.timedlog(buffer);
                break; // stop the integration
            }   
            //4) update the state vector
            rkf78.do_step(std::bind(&Integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
            step ++;
            progress = (float)step/maxsteps;
                        
        }
        is_running = false;
        exists_solution = true;
        cons.timedlog("[Integrator] Simulation has been completed. Press Plot to process your results.");
        return;
    }
};

#endif
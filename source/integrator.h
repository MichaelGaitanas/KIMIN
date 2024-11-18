#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include<atomic>
#include<boost/numeric/odeint.hpp>

#include"constant.h"
#include"typedef.h"
#include"linalg.h"
#include"conversion.h"
#include"ellipsoid.h"
#include"rigidbody.h"
#include"gravity.h"
#include"properties_panel.h"
#include"console_panel.h"

class integrator
{
public:
    properties_panel properties; //The user's choice of inputs in the gui.

    double m; //Reduced binary mass ( m = M1*M2/(M1 + M2) ).
    dmat3 I1, I2; //Moments of inetia.
    dtens J1, J2; //Inertial integrals.
    double brillouin1, brillouin2; //Brillouin radii of the 2 bodies.

    bool collision; //Collision detection flag (Brillouin spheres intersection).

    double t0, tmax, dt; //Integration time.

    dmat orbit; //This is the solution matrix of the differential equations that will be propagated (time + state vector).

    integrator() { }

    //Copy properties.
    integrator(const properties_panel &properties)
    {
        this->properties = properties;
        //Note : In the following member functions, whatever change is made upon the 'properties' variable, has nothing to do with the gui's displayed properties.
        //We operate only on this class' member 'properties', which is only a copy.
    }

private:
    //This function builds the right hand sides of the differential equations of motion. It is executed at each step of the integration.
    void build_rhs(const boost::array<double, 20> &state, boost::array<double, 20> &dstate, double /*t*/)
    {
        //Extract the current state vector into individual variables (for readability mostly).
        dvec3 r  =  { state[0],  state[1],  state[2] };
        dvec3 v  =  { state[3],  state[4],  state[5] };
        dvec4 q1 =  { state[6],  state[7],  state[8],  state[9] };
        dvec3 w1b = { state[10], state[11], state[12] };
        dvec4 q2 =  { state[13], state[14], state[15], state[16] };
        dvec3 w2b = { state[17], state[18], state[19] };

        //Normalize the quaternions. We can probably skip doing this at every evaluation of the RHS and
        //perhaps do it (e.g.) per 10 or more evaluations to gain performance, but let's leave be strict for now.
        q1 = quat2unit(q1);
        q2 = quat2unit(q2);

        //Construct the rotation matrices from the (normalized) quaternions. Even though the quaternions do encode the rotational state
        //of each body, we need the matrices as well because the expressions of the potential, force and torque are written in matrix form.
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);

        //Calculate the force and the torques in the inertial frame, depending on the user's choice of the mutual potential order.
        dvec3 force, tau1i;
        if (properties.ord2_checkbox)
        {
            force = mut_force_integrals_ord2(r, properties.M1,J1,A1, properties.M2,J2,A2);
            tau1i = mut_torque_integrals_ord2(r, J1,A1, properties.M2);
        }
        else if (properties.ord3_checkbox)
        {
            force = mut_force_integrals_ord3(r, properties.M1,J1,A1, properties.M2,J2,A2);
            tau1i = mut_torque_integrals_ord3(r, J1,A1, properties.M2);
        } 
        else //Only 'ord4_checkbox' remains...
        {
            force = mut_force_integrals_ord4(r, properties.M1,J1,A1, properties.M2,J2,A2);
            tau1i = mut_torque_integrals_ord4(r,              J1,A1, properties.M2,J2,A2);
        }

        dvec3 tau2i = -tau1i - cross(r,force);

        //Convert the torques into the corresponding body frames because Euler's ODEs are written in the body frames.
        dvec3 tau1b = iner2body(tau1i,A1);
        dvec3 tau2b = iner2body(tau2i,A2);

        dvec4 dq1 = quat_rhs(q1,w1b);
        dvec3 dw1b = euler_rhs(w1b,I1,tau1b);

        dvec4 dq2 = quat_rhs(q2,w2b);
        dvec3 dw2b = euler_rhs(w2b,I2,tau2b);

        //Now here are the actual RHS :

        //Relative position RHS (x,y,z).
        dstate[0] = v[0];
        dstate[1] = v[1];
        dstate[2] = v[2];

        //Relative velocity RHS (vx,vy,vz).
        dstate[3] = force[0]/m;
        dstate[4] = force[1]/m;
        dstate[5] = force[2]/m;

        //Quaternion RHS of rigid body 1 (q10,q11,q12,q13).
        dstate[6] = dq1[0];
        dstate[7] = dq1[1];
        dstate[8] = dq1[2];
        dstate[9] = dq1[3];

        //Euler RHS of rigid body 1 (w11,w12,w13).
        dstate[10] = dw1b[0];
        dstate[11] = dw1b[1];
        dstate[12] = dw1b[2];

        //Quaternion RHS of rigid body 2 (q20,q21,q22,q23).
        dstate[13] = dq2[0];
        dstate[14] = dq2[1];
        dstate[15] = dq2[2];
        dstate[16] = dq2[3];

        //Euler RHS of rigid body 2 (w21,w22,w23).
        dstate[17] = dw2b[0];
        dstate[18] = dw2b[1];
        dstate[19] = dw2b[2];
    }

public:
    //Before the actual integration of the ODEs starts, we do some preparations.
    void prepare(std::atomic<bool> &abort_flag, std::atomic<float> &progress, console_panel &console)
    {
        console.add_time_and_then_text("[Info] : Integrator preparation started.");
        progress.store(0.0f);

        m = properties.M1*properties.M2/(properties.M1 + properties.M2);

        //Preparation 1 : If the user chose Keplerian elements as initial position/velocity, then, transform
        //them to Cartesian coords because the F2BP odes are written in Cartesian form.
        if (properties.cart_kep_var_choice == 1)
            properties.cart = kep2cart(dvec6{properties.kep[0],
                                             properties.kep[1],
                                             properties.kep[2]*pi/180.0,
                                             properties.kep[3]*pi/180.0,
                                             properties.kep[4]*pi/180.0,
                                             properties.kep[5]*pi/180.0}, G*(properties.M1 + properties.M2));
        
        //Preparation 2 : If the user chose Euler angles as initial orientations, then, transform them to
        //quaternions because the F2BP odes are written in quaternion form.
        if (properties.orient_var_choice == 0)
        {
            //Note : ang2quat() ensures that the returned quaternion is normalized, so we don't need to apply quat2unit().
            properties.q1 = ang2quat(properties.rpy1*pi/180.0);
            properties.q2 = ang2quat(properties.rpy2*pi/180.0);
        }

        //Preparation 3 : If the user chose to input the angular velocities in the inertial frame, then, transform them
        //to the corresponding body frames because the Euler equations of rotation are written in body frame form.
        if (properties.frame_type_choice == 0)
        {
            //In this case, the user chose angular velocities to be in the inertial/world frame, so we convert them to the body frames.
            properties.w1b = iner2body(properties.w1i, quat2mat(properties.q1));
            properties.w2b = iner2body(properties.w2i, quat2mat(properties.q2));
        }

        //Preparation 4 : Based on the user's choice 'ell_checkbox' or '.obj files', we need to evaluate :
        //1) The Brillouin radii, 2) The inertial inertial integrals of the corresponding chosen order.
        if (properties.ell_checkbox)
        {
            brillouin1 = ell_brillouin(properties.semiaxes1);
            brillouin2 = ell_brillouin(properties.semiaxes2);
            if (properties.ord2_checkbox)
            {
                J1 = ell_integrals(properties.M1, properties.semiaxes1, 2);
                J2 = ell_integrals(properties.M2, properties.semiaxes2, 2);
                I1 = ell_inertia(properties.M1, properties.semiaxes1);
                I2 = ell_inertia(properties.M2, properties.semiaxes2);
            }
            else if (properties.ord3_checkbox)
            {
                J1 = ell_integrals(properties.M1, properties.semiaxes1, 3);
                J2 = ell_integrals(properties.M2, properties.semiaxes2, 3);
                I1 = ell_inertia(properties.M1, properties.semiaxes1);
                I2 = ell_inertia(properties.M2, properties.semiaxes2);
            }
            else //Only 'ord4_checkbox' remains...
            {
                J1 = ell_integrals(properties.M1, properties.semiaxes1, 4);
                J2 = ell_integrals(properties.M2, properties.semiaxes2, 4);
                I1 = ell_inertia(properties.M1, properties.semiaxes1);
                I2 = ell_inertia(properties.M2, properties.semiaxes2);
            }
        }
        else
        {
            //Polyhedron logic. To be added...
        }

        //Preparation 5 : If the user assumed a kinetic impactor, then (based on theory) we apply a momentum (velocity)
        //enhancement (or reduction, depending on the sign of beta and the direction of v_impact[]) to the secondary body. 
        if (properties.impactor_checkbox)
        {
            //This in turn affects the mutual velocity, which is updated as :
            properties.cart[3] += properties.beta*properties.M_impact*properties.v_impact[0]/properties.M2;
            properties.cart[4] += properties.beta*properties.M_impact*properties.v_impact[1]/properties.M2;
            properties.cart[5] += properties.beta*properties.M_impact*properties.v_impact[2]/properties.M2;
        }

        collision = false; //Assuming no collision at t = t0.
        
        //Convert the time in [sec]
        t0 = properties.epoch*86400.0; //[sec]
        tmax = t0 + properties.dur*86400.0; //[sec]
        dt = properties.step*86400.0; //[sec]

        orbit.clear();

        if (!abort_flag.load())
        {
            progress.store(1.0f);
            console.add_time_and_then_text("[Info] : Integration ended.");
        }
    }

    void run(std::atomic<bool> &abort_flag, std::atomic<float> &progress, console_panel &console)
    {
        console.add_time_and_then_text("[Info] : New integration started.");
        progress.store(0.0f);

        //Initial conditions.
        boost::array<double, 20> state = { properties.cart[0], properties.cart[1], properties.cart[2],
                                           properties.cart[3], properties.cart[4], properties.cart[5],
                                             properties.q1[0],   properties.q1[1],   properties.q1[2], properties.q1[3],
                                            properties.w1b[0],  properties.w1b[1],  properties.w1b[2],
                                             properties.q2[0],   properties.q2[1],   properties.q2[2], properties.q2[3],
                                            properties.w2b[0],  properties.w2b[1],  properties.w2b[2] };

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>> rkf78;
        char formatted_text[128];
        for (double t = t0; t <= tmax; t += dt)
        {
            //Append the current state into the final 'orbit' matrix.
            orbit.push_back({t, state[0],  state[1],  state[2],
                                state[3],  state[4],  state[5],
                                state[6],  state[7],  state[8],  state[9],
                                state[10], state[11], state[12],
                                state[13], state[14], state[15], state[16],
                                state[17], state[18], state[19]});

            //Check for sphere-sphere collision detection between the 2 asteroids.
            if (sphere_sphere_collision(length(dvec3{state[0],state[1],state[2]}), brillouin1, brillouin2))
            {
                sprintf(formatted_text,"[Integrator] Collision detected at t = %5.2lf days.", t/86400.0);
                console.add_time_and_then_text(formatted_text);
                collision = true;
                break;
            }

            //Check the abort flag (the user might want to kill the integration by pressing the 'Abort' button in the gui).
            if (abort_flag.load())
            {
                sprintf(formatted_text, "[Info] : Integration was aborted at t = %5.2lf [days].", t/86400.0);
                console.add_time_and_then_text(formatted_text);
                break;
            }
                
            //Update the state vector by doing 1 step of the numerical method.
            rkf78.do_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);

            //Update the progressbar value in [0,1].
            progress.store((t-t0)/(tmax-t0));
        }

        if (!abort_flag.load())
        {
            progress.store(1.0f);
            console.add_time_and_then_text("[Info] : Integration ended.");
        }
    }
};

#endif
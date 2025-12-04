/* This class handles the numerical integration of the F2BP. */

#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include<atomic>
#include<boost/numeric/odeint.hpp>

#include"constants.h"
#include"typedef.h"
#include"linalg.h"
#include"conversion.h"
#include"ellipsoid.h"
#include"polyhedron.h"
#include"rigidbody.h"
#include"gravity.h"
#include"properties_panel.h"
#include"console_panel.h"

class integrator
{
public:
    properties_panel properties; //This is meant to be copy of the user's choice of inputs in the gui.

    double m; //Reduced binary mass ( m = M1*M2/(M1 + M2) ).
    double m1, m2; //Coefficients that when multiplied with the relative position, yield each body's aboslute position in the COM frame, i.e. m1 = -M2/(M1+M2), m2 = M1/(M1+M2).
    dmat3 I1, I2; //Moments of inertia.
    dtens J1, J2; //Inertial integrals.
    double brillouin1, brillouin2; //Brillouin radii of the 2 bodies.
    bool collision, collision_sp; //Collision detection flag for the binary and the spacecraft (respectively).
    bool maneuver1, maneuver2; //Whether or not a beta-kick (equivalent maneuver) has been applied to the corresponding body.

    dvec3 rsun; //Constant sun's position relative to the COM of the binary in Cartesian coords.

    double t0, tmax, dt, init_guess_time_step; //Integration time.

    dmat orbit; //This is the solution matrix of the differential equations that will be propagated (time + state vector).

    integrator() { } //Needed to instantiate an integrator object in the solution class.

    integrator(const properties_panel &properties) //And this is needed to instantiate the integrator in the gui class.
    {
        this->properties = properties;
        //Note : In the following member functions, whatever change is made upon the 'properties' variable, has nothing to do with the gui's displayed properties.
        //We operate only on THIS class member 'properties', which is a deep copy of the gui's input.
    }

private:
    //This function builds the right hand sides of the differential equations of motion. It is executed at each step of the integration.
    void build_rhs(const boost::array<double, N_ODES> &state, boost::array<double, N_ODES> &dstate, double /*t*/)
    {
        //Extract the current state vector of the binary (for readability mostly).
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
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);

        //Calculate the force and the torques in the inertial frame, depending on the user's choice of the mutual potential order.
        dvec6 force_and_tau1i;
        if (properties.ord2_checkbox)
            force_and_tau1i = mut_force_tau1i_integrals_ord2(r, properties.M1,J1,A1, properties.M2,J2,A2);
        else if (properties.ord3_checkbox)
            force_and_tau1i = mut_force_tau1i_integrals_ord3(r, properties.M1,J1,A1, properties.M2,J2,A2);
        else //Only 'ord4_checkbox' remains...
            force_and_tau1i = mut_force_tau1i_integrals_ord4(r, properties.M1,J1,A1, properties.M2,J2,A2);
        
        dvec3 force = {force_and_tau1i[0], force_and_tau1i[1], force_and_tau1i[2]};

        dvec3 tau1i = {force_and_tau1i[3], force_and_tau1i[4], force_and_tau1i[5]};
        dvec3 tau2i = -tau1i - cross(r,force);

        //Convert the torques into the corresponding body frames because Euler's ODEs are written in the body frames.
        dvec3 tau1b = iner2body(tau1i,A1);
        dvec3 tau2b = iner2body(tau2i,A2);

        dvec4 dq1 = quat_rhs(q1,w1b);
        dvec3 dw1b = euler_rhs(w1b,I1,tau1b);

        dvec4 dq2 = quat_rhs(q2,w2b);
        dvec3 dw2b = euler_rhs(w2b,I2,tau2b);

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

        if (properties.spacecraft_checkbox)
        {
            //Spacecraft state in binary's COM frame.
            dvec3 rsp = { state[20], state[21], state[22] };
            dvec3 vsp = { state[23], state[24], state[25] };
            //Individual bodies' COM positions (i.e. COM1 and COM2) in binary's COM frame.
            dvec3 r1 = m1*r;
            dvec3 r2 = m2*r;
            //Corresponding body to spacecraft vector.
            dvec3 rho1 = rsp - r1;
            dvec3 rho2 = rsp - r2;

            dvec3 asp; //Spacecraft's acceleration due to the combined presence of the 2 rigid bodies + SRP.
            if (properties.ord2_checkbox)
                asp = accel_integrals_ord2(rho1, properties.M1, J1, A1) + accel_integrals_ord2(rho2, properties.M2, J2, A2);
            else if (properties.ord3_checkbox)
                asp = accel_integrals_ord3(rho1, properties.M1, J1, A1) + accel_integrals_ord3(rho2, properties.M2, J2, A2);
            else
                asp = accel_integrals_ord4(rho1, properties.M1, J1, A1) + accel_integrals_ord4(rho2, properties.M2, J2, A2);

            if (properties.srp_checkbox)
            {
                bool sp_in_shadow = false;
                if (properties.srp_shadow_checkbox)
                    sp_in_shadow = line_sphere_intersection(rsp, rsun, r1, brillouin1) || line_sphere_intersection(rsp, rsun, r2, brillouin2);
                if (!sp_in_shadow)
                    asp = asp + accel_srp(properties.sp_refl, properties.sp_area, properties.sp_mass, rsp, rsun);
            }

            //Spacecraft's position and velocity rhs.
            dstate[20] = vsp[0];
            dstate[21] = vsp[1];
            dstate[22] = vsp[2];
            dstate[23] = asp[0];
            dstate[24] = asp[1];
            dstate[25] = asp[2];
        }
        else
            dstate[20] = dstate[21] = dstate[22] = dstate[23] = dstate[24] = dstate[25] = 0.0;
    }

public:
    //Before the actual integration of the ODEs starts, we do some preparations.
    void prepare(console_panel &console)
    {
        console.add_timed_text("[Polyhedron] : Computing inertial integrals... ");

        m  =  properties.M1*properties.M2/(properties.M1 + properties.M2);
        m1 = -properties.M2/(properties.M1 + properties.M2);
        m2 =  properties.M1/(properties.M1 + properties.M2);

        //Preparation 1 : If the user chose Keplerian elements as initial position/velocity, then, transform
        //them to Cartesian coords because the F2BP odes are written in Cartesian form.
        if (properties.pos_vel_var == properties_panel::KEPLERIAN)
            properties.cart = kep2cart(dvec6{properties.kep[0],
                                             properties.kep[1],
                                             properties.kep[2]*PI/180.0,
                                             properties.kep[3]*PI/180.0,
                                             properties.kep[4]*PI/180.0,
                                             properties.kep[5]*PI/180.0}, G*(properties.M1 + properties.M2));
        
        //Preparation 2 : If the user chose Euler angles as initial orientations, then, transform them to
        //quaternions because the F2BP odes are written in quaternion form.
        if (properties.orient_var == properties_panel::EULER_XYZ)
        {
            //Note : ang2quat() ensures that the returned quaternion is normalized, so we don't need to apply quat2unit().
            properties.q1 = ang2quat(properties.rpy1*PI/180.0);
            properties.q2 = ang2quat(properties.rpy2*PI/180.0);
        }

        //Preparation 3 : If the user chose to input the angular velocities in the inertial frame, then, transform them
        //to the corresponding body frames because the Euler equations of rotation are written in body frame form.
        if (properties.angvel_frame == properties_panel::INERTIAL)
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
            I1 = ell_inertia(properties.M1, properties.semiaxes1);
            I2 = ell_inertia(properties.M2, properties.semiaxes2);
            if (properties.ord2_checkbox)
            {
                J1 = ell_integrals(properties.M1, properties.semiaxes1, 2);
                J2 = ell_integrals(properties.M2, properties.semiaxes2, 2);
            }
            else if (properties.ord3_checkbox)
            {
                J1 = ell_integrals(properties.M1, properties.semiaxes1, 3);
                J2 = ell_integrals(properties.M2, properties.semiaxes2, 3);
            }
            else //Only 'ord4_checkbox' remains...
            {
                J1 = ell_integrals(properties.M1, properties.semiaxes1, 4);
                J2 = ell_integrals(properties.M2, properties.semiaxes2, 4);
            }
        }
        else //.obj file
        {
            properties.poly1.set_com_zero();
            properties.poly1.set_inertia_diagonal();
            brillouin1 = properties.poly1.get_farthest_vertex_distance();
            
            properties.poly2.set_com_zero();
            properties.poly2.set_inertia_diagonal();
            brillouin2 = properties.poly2.get_farthest_vertex_distance();

            I1 = properties.poly1.get_inertia(properties.M1);
            I2 = properties.poly2.get_inertia(properties.M2);
            if (properties.ord2_checkbox)
            {
                J1 = properties.poly1.get_inertial_integrals_ord2(properties.M1);
                J2 = properties.poly2.get_inertial_integrals_ord2(properties.M2);
            }
            else if (properties.ord3_checkbox)
            {
                J1 = properties.poly1.get_inertial_integrals_ord3(properties.M1);
                J2 = properties.poly2.get_inertial_integrals_ord3(properties.M2);
            }
            else //Only 'ord4_checkbox' remains...
            {
                J1 = properties.poly1.get_inertial_integrals_ord4(properties.M1);
                J2 = properties.poly2.get_inertial_integrals_ord4(properties.M2);
            }
        }
        
        //Preparation 5 : Convert the time in [sec]
        t0 = properties.epoch*86400.0; //[sec]
        tmax = t0 + properties.dur*86400.0; //[sec]
        if (properties.integration_method == properties_panel::RKF78_FIXED || properties.integration_method == properties_panel::ABM5_FIXED)
            dt = properties.step*86400.0; //[sec]
        else
            init_guess_time_step = ODES_INIT_GUESS_TIME_STEP; //[sec]

        //Preparation 6 : Convert impact times in [sec]. Then, apply maneuvers BEFORE the while integration loop, only if the impact times are chosen to be at t = t0.
        maneuver1 = maneuver2 = false;
        if (properties.impactors_checkbox)
        {
            properties.tD1 *= 86400.0;
            properties.tD2 *= 86400.0;
            if (fabs(t0 - properties.tD1) < 1e-15)
            {
                properties.cart[3] -= properties.beta1*properties.mD1*properties.vD1[0]/properties.M1;
                properties.cart[4] -= properties.beta1*properties.mD1*properties.vD1[1]/properties.M1;
                properties.cart[5] -= properties.beta1*properties.mD1*properties.vD1[2]/properties.M1;
                if (properties.spacecraft_checkbox)
                {
                    properties.vsp = properties.vsp - properties.beta1*properties.mD1*properties.vD1/(properties.M1 + properties.M2);
                }
                maneuver1 = true;
            }
            if (fabs(t0 - properties.tD2) < 1e-15)
            {
                properties.cart[3] += properties.beta2*properties.mD2*properties.vD2[0]/properties.M2;
                properties.cart[4] += properties.beta2*properties.mD2*properties.vD2[1]/properties.M2;
                properties.cart[5] += properties.beta2*properties.mD2*properties.vD2[2]/properties.M2;
                if (properties.spacecraft_checkbox)
                {
                    properties.vsp = properties.vsp - properties.beta2*properties.mD2*properties.vD2/(properties.M1 + properties.M2);
                }
                maneuver2 = true;
            }
        }

        //Preparation 7 : Spacecraft and SRP assumption.
        if (!properties.spacecraft_checkbox)
        {
            properties.rsp[0] = properties.rsp[1] = properties.rsp[2] = 0.0;
            properties.vsp[0] = properties.vsp[1] = properties.vsp[2] = 0.0;
        }
        else if (properties.spacecraft_checkbox && properties.srp_checkbox)
        {
            rsun = spher2cart({properties.sun_dist, properties.sun_lon*PI/180.0, properties.sun_lat*PI/180.0}); //{[AU], [AU], [AU]}
            rsun = rsun*AU2KM; //{[km], [km], [km]}
        }

        collision = collision_sp = false; //Assuming no collision when the simulation starts.
        
        orbit.clear();

        console.add_text("Done.\n");
    }

    void run(std::atomic<bool> &abort_flag, std::atomic<float> &progress, console_panel &console)
    {
        console.add_timed_text("[Integrator] : Propagating orbit... ");
        progress.store(0.0f);

        //Initial conditions.
        boost::array<double, N_ODES> state = {  properties.cart[0], properties.cart[1], properties.cart[2],
                                                properties.cart[3], properties.cart[4], properties.cart[5],
                                                  properties.q1[0],   properties.q1[1],   properties.q1[2], properties.q1[3],
                                                 properties.w1b[0],  properties.w1b[1],  properties.w1b[2],
                                                  properties.q2[0],   properties.q2[1],   properties.q2[2], properties.q2[3],
                                                 properties.w2b[0],  properties.w2b[1],  properties.w2b[2],
                                                 properties.rsp[0],  properties.rsp[1],  properties.rsp[2],
                                                 properties.vsp[0],  properties.vsp[1],  properties.vsp[2] };
        
        double t = t0; //Initialize time.

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, N_ODES>> rkf78_fixed;
        auto rkf78_adaptive = boost::numeric::odeint::make_controlled(properties.target_error, properties.target_error, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, N_ODES>>());
        boost::numeric::odeint::bulirsch_stoer<boost::array<double, N_ODES>> bstoer_adaptive(properties.target_error, properties.target_error);
        boost::numeric::odeint::adams_bashforth_moulton<5, boost::array<double, N_ODES>> abm_fixed;

        if (properties.integration_method == properties_panel::ABM5_FIXED) //Seed Adams-Bashforth-Moulton only if this is the requested method of integration.
            abm_fixed.initialize(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
        
        char formatted_text[128];
        
        //Shoot it!!!
        while (t <= tmax)
        {
            //1) Orbits : append the current state into the final 'orbit' matrix. No matter what, at least the initial condition is gonna be taken into account (provided that the user input properties are correct).
            orbit.push_back({t, state[0],  state[1],  state[2],
                                state[3],  state[4],  state[5],
                                state[6],  state[7],  state[8],  state[9],
                                state[10], state[11], state[12],
                                state[13], state[14], state[15], state[16],
                                state[17], state[18], state[19],
                                state[20], state[21], state[22] });

            //2) Kinetic impacts : apply corresponding maneuvers in case that kinetic impactors were assumed in the gui.
            if (properties.impactors_checkbox)
            {
                if (!maneuver1 && t >= properties.tD1)
                {
                    state[3] -= properties.beta1*properties.mD1*properties.vD1[0]/properties.M1;
                    state[4] -= properties.beta1*properties.mD1*properties.vD1[1]/properties.M1;
                    state[5] -= properties.beta1*properties.mD1*properties.vD1[2]/properties.M1;
                    if (properties.spacecraft_checkbox)
                    {
                        state[23] -= properties.beta1*properties.mD1*properties.vD1[0]/(properties.M1 + properties.M2);
                        state[24] -= properties.beta1*properties.mD1*properties.vD1[1]/(properties.M1 + properties.M2);
                        state[25] -= properties.beta1*properties.mD1*properties.vD1[2]/(properties.M1 + properties.M2);
                    }
                    maneuver1 = true;
                }
                if (!maneuver2 && t >= properties.tD2)
                {
                    state[3] += properties.beta2*properties.mD2*properties.vD2[0]/properties.M2;
                    state[4] += properties.beta2*properties.mD2*properties.vD2[1]/properties.M2;
                    state[5] += properties.beta2*properties.mD2*properties.vD2[2]/properties.M2;
                    if (properties.spacecraft_checkbox)
                    {
                        state[23] -= properties.beta2*properties.mD2*properties.vD2[0]/(properties.M1 + properties.M2);
                        state[24] -= properties.beta2*properties.mD2*properties.vD2[1]/(properties.M1 + properties.M2);
                        state[25] -= properties.beta2*properties.mD2*properties.vD2[2]/(properties.M1 + properties.M2);
                    }
                    maneuver2 = true;
                }
            }

            //3) Collisions : check for asteroid-asteroid and spacecraft-asteroid collisions.
            if (properties.collision_spheres) //Spheres checkbox.
            {
                //Sphere asteroid-asteroid collision.
                if (sphere_sphere_collision(length(dvec3{state[0],state[1],state[2]}), brillouin1, brillouin2))
                {
                    sprintf(formatted_text,"< Collision (asteroid - asteroid) detected at t = %5.2lf [days]. >\n", t/86400.0);
                    console.add_text(formatted_text);
                    collision = true;
                    break;
                }
                
                //Sphere spacecraft-asteroid (sphere-point test).
                if (properties.spacecraft_checkbox)
                {
                    const dvec3 r = dvec3{state[0],state[1],state[2]};
                    const dvec3 r1 = m1*r;
                    const dvec3 r2 = m2*r;
                    const dvec3 rsp = dvec3{state[20],state[21],state[22]};
                    if (sphere_point_collision(length(rsp - r1), brillouin1))
                    {
                        sprintf(formatted_text,"< Collision (asteroid - spacecraft) at t = %5.2lf [days]. >\n", t/86400.0);
                        console.add_text(formatted_text);
                        collision_sp = true;
                        break;
                    }
                    if (sphere_point_collision(length(rsp - r2), brillouin2))
                    {
                        sprintf(formatted_text,"< Collision (asteroid - spacecraft) at t = %5.2lf [days]. >\n", t/86400.0);
                        console.add_text(formatted_text);
                        collision_sp = true;
                        break;
                    }
                }
            }
            else if (properties.collision_polyhedra) //Polyhedra checkbox, but with sphere gates.
            {
                const dvec3 r = dvec3{state[0],state[1],state[2]};
                const dvec3 r1 = m1*r;
                const dvec3 r2 = m2*r;
                if (sphere_sphere_collision(length(r), brillouin1, brillouin2))
                {
                    const dmat3 A1 = quat2mat(dvec4{state[6],state[7],state[8],state[9]});
                    const dmat3 A2 = quat2mat(dvec4{state[13],state[14],state[15],state[16]});
                    if (polyhedron_polyhedron_collision(properties.poly1, A1, r1, properties.poly2, A2, r2))
                    {
                        sprintf(formatted_text,"< Collision (asteroid - asteroid) at t = %5.2lf [days]. >\n", t/86400.0);
                        console.add_text(formatted_text);
                        collision = true;
                        break;
                    }
                }

                //Polyhedron spacecraft-asteroid (sphere-point gate, then polyhedron-point).
                if (properties.spacecraft_checkbox)
                {
                    const dvec3 rsp = dvec3{state[20],state[21],state[22]};
                    if (sphere_point_collision(length(rsp - r1), brillouin1))
                    {
                        const dmat3 A1 = quat2mat(dvec4{state[6],state[7],state[8],state[9]});
                        if (polyhedron_point_collision(properties.poly1, A1, r1, rsp))
                        {
                            sprintf(formatted_text,"< Collision (spacecraft - asteroid) at t = %5.2lf [days]. >\n", t/86400.0);
                            console.add_text(formatted_text);
                            collision_sp = true;
                            break;
                        }
                    }
                    if (sphere_point_collision(length(rsp - r2), brillouin2))
                    {
                        const dmat3 A2 = quat2mat(dvec4{state[13],state[14],state[15],state[16]});
                        if (polyhedron_point_collision(properties.poly2, A2, r2, rsp))
                        {
                            sprintf(formatted_text,"< Collision (spacecraft - asteroid, polyhedron) at t = %5.2lf [days]. >\n", t/86400.0);
                            console.add_text(formatted_text);
                            collision_sp = true;
                            break;
                        }
                    }
                }
            }

            //Check the abort flag (the user might want to kill the integration by pressing the 'Abort' button in the gui).
            if (abort_flag.load())
            {
                sprintf(formatted_text, "< Aborted at t = %5.2lf [days]. > \n", t/86400.0);
                console.add_text(formatted_text);
                break;
            }
            
            //Update the state vector by doing 1 step of the numerical method.
            if (properties.integration_method == properties_panel::RKF78_FIXED)
            {
                rkf78_fixed.do_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
                t += dt;
            }
            else if (properties.integration_method == properties_panel::RKF78_ADAPTIVE)
                rkf78_adaptive.try_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, init_guess_time_step);
            else if (properties.integration_method == properties_panel::BSTOER_ADAPTIVE)
                bstoer_adaptive.try_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, init_guess_time_step);
            else //properties_panel::ABM5_FIXED
            {
                abm_fixed.do_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
                t += dt;
            }
            //Note : Boost's do_step() does NOT update internally t, hence we have to do it ourselves. But try_step() DOES update internally t, hence we do not touch it in this case.

            //Update the progressbar value in [0,1].
            progress.store((t-t0)/(tmax-t0));
        }

        if (!abort_flag.load() && !collision && !collision_sp)
        {
            progress.store(1.0f);
            console.add_text("Done.\n");
        }
    }
};

#endif
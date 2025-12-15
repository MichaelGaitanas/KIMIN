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
#include"properties.h"
#include"console.h"

class integrator
{
public:
    properties props; //User's choice of properties inputs in the gui (copy).

    double m; //Reduced binary mass ( m = M1*M2/(M1 + M2) ).
    double m1, m2; //Coefficients that when multiplied with the mutual position, yield each body's aboslute position in the COM frame, i.e. m1 = -M2/(M1+M2), m2 = M1/(M1+M2).
    dmat3 I1, I2; //Moments of inertia.
    dtens J1, J2; //Inertial integrals.
    double brillouin1, brillouin2; //Brillouin radii of the 2 bodies.
    bool collision_mut, collision_sp; //Collision flags for the binary and the spacecraft.
    bool maneuver1, maneuver2; //Whether or not a beta-kick (equivalent maneuver) has been applied to the corresponding body.

    double t0, tmax, dt, init_guess_time_step; //Integration time.
    dmat orbit; //This is the solution matrix of the differential equations that will be propagated (time + state vector).


    integrator() { } //Needed to instantiate an integrator object in the solution class.

    integrator(const properties &props) //Needed to instantiate the integrator in the gui class.
    {
        this->props = props; //Deep copy of the gui's properties. So whatever change happens here to any property, it happens on the copy.
    }

private:
    //In case Sun's gravity is off, this function evaluates analytically the equation of motion of the COM of the binary in the Heliocentric frame.
    dvec3 get_analytic_rcom(const dvec6 &cart_com0, const double t)
    {
        const dvec3 rcom0 = {cart_com0[0],cart_com0[1],cart_com0[2]};
        const dvec3 vcom0 = {cart_com0[3],cart_com0[4],cart_com0[5]};
        dvec3 r = rcom0 + vcom0*(t - t0); //Due to initial state setting (straight line until we add Sun's gravity).
        if (props.impactors_checkbox)
        {
            if ((t + 1e-15) >= props.tD1) //Impact 1 contribution.
            {
                const dvec3 delta_vcom1 = props.beta1*props.mD1*props.vD1/(props.M1 + props.M2);
                r = r + delta_vcom1*(t - props.tD1);
            }
            if ((t + 1e-15) >= props.tD2) //Impact 2 contribution.
            {
                const dvec3 delta_vcom2 = props.beta2*props.mD2*props.vD2/(props.M1 + props.M2);
                r = r + delta_vcom2*(t - props.tD2);
            }
        }
        return r;
    }

    //This function builds the right hand sides of the differential equations of motion. It is executed at each step of the integration.
    void build_rhs(const boost::array<double, N_ODES> &state, boost::array<double, N_ODES> &dstate, double t)
    {
        //Extract binary's mutual state (position & velocity) and absolute orientations (orientations & angular velocities).
        dvec3 rmut = {state[0],  state[1],  state[2]};
        dvec3 vmut = {state[3],  state[4],  state[5]};
        dvec4 q1   = {state[6],  state[7],  state[8],  state[9]};
        dvec3 w1b  = {state[10], state[11], state[12]};
        dvec4 q2   = {state[13], state[14], state[15], state[16]};
        dvec3 w2b  = {state[17], state[18], state[19]};
        //Extract COM's state.
        dvec3 rcom = {state[20], state[21], state[22]};
        dvec3 vcom = {state[23], state[24], state[25]};

        //Compute rotation matrices from normalized quaternions.
        q1 = quat2unit(q1);
        q2 = quat2unit(q2);
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);

        //Calculate the mutual force and the torque acted on body 1 in the inertial (Heliocentric) frame.
        dvec6 force_and_torque1i;
        if (props.ord2_checkbox)
            force_and_torque1i = mut_force_torque1i_integrals_ord2(rmut, props.M1,J1,A1, props.M2,J2,A2);
        else if (props.ord3_checkbox)
            force_and_torque1i = mut_force_torque1i_integrals_ord3(rmut, props.M1,J1,A1, props.M2,J2,A2);
        else
            force_and_torque1i = mut_force_torque1i_integrals_ord4(rmut, props.M1,J1,A1, props.M2,J2,A2);
        
        //Mutual and COM acceleration due to binary's gravity (no Sun yet).
        dvec3 amut = {force_and_torque1i[0], force_and_torque1i[1], force_and_torque1i[2]}/m;
        dvec3 acom = {0.0,0.0,0.0};
        if (props.sun_gravity) //Now add Sun's contribution to both the amut and acom.
        {
            //Individual bodies' inertial (Heliocentric) positions, i.e. COM1 and COM2.
            dvec3 r1 = rcom + m1*rmut;
            dvec3 r2 = rcom + m2*rmut;
            double r1len = length(r1);
            double r2len = length(r2);
            double rcomlen = length(rcom);
            amut = amut - G*MSUN*(r2/(r2len*r2len*r2len) - r1/(r1len*r1len*r1len));
            acom = -G*MSUN*rcom/(rcomlen*rcomlen*rcomlen);
        }
        //Else the mutual state is governed only by the binary's mutual gravity and the COM shall move at a straight line in space. Bye bye Solar system!

        dvec3 torque1i = {force_and_torque1i[3], force_and_torque1i[4], force_and_torque1i[5]};
        dvec3 torque2i = -torque1i - cross(r,force);

        //Convert the torques into the corresponding body frames because Euler's ODEs are written in the body frame.
        dvec3 torque1b = iner2body(torque1i,A1);
        dvec3 torque2b = iner2body(torque2i,A2);

        dvec4 dq1 = quat_rhs(q1,w1b);
        dvec3 dw1b = euler_rhs(w1b,I1,torque1b);

        dvec4 dq2 = quat_rhs(q2,w2b);
        dvec3 dw2b = euler_rhs(w2b,I2,torque2b);

        //Mutual position rhs (xmut,ymut,zmut).
        dstate[0] = vmut[0];
        dstate[1] = vmut[1];
        dstate[2] = vmut[2];

        //Mutual velocity rhs (vxmut,vymut,vzmut).
        dstate[3] = amut[0];
        dstate[4] = amut[1];
        dstate[5] = amut[2];

        //Quaternion rhs of rigid body 1 (q10,q11,q12,q13).
        dstate[6] = dq1[0];
        dstate[7] = dq1[1];
        dstate[8] = dq1[2];
        dstate[9] = dq1[3];

        //Euler rhs of rigid body 1 (w11,w12,w13).
        dstate[10] = dw1b[0];
        dstate[11] = dw1b[1];
        dstate[12] = dw1b[2];

        //Quaternion rhs of rigid body 2 (q20,q21,q22,q23).
        dstate[13] = dq2[0];
        dstate[14] = dq2[1];
        dstate[15] = dq2[2];
        dstate[16] = dq2[3];

        //Euler rhs of rigid body 2 (w21,w22,w23).
        dstate[17] = dw2b[0];
        dstate[18] = dw2b[1];
        dstate[19] = dw2b[2];

        //COM position rhs (xcom,ycom,zcom).
        dstate[20] = vcom[0];
        dstate[21] = vcom[1];
        dstate[22] = vcom[2];

        //COM velocity rhs (vxcom,vycom,vzcom).
        dstate[23] = acom[0];
        dstate[24] = acom[1];
        dstate[25] = acom[2];

        //Now handle the spacecraft's ODEs :
        if (props.spacecraft_checkbox)
        {
            //Spacecraft's state in the inertial (Heliocentric) frame.
            dvec3 rsp = {state[26], state[27], state[28]};
            dvec3 vsp = {state[29], state[30], state[31]};
            //Individual bodies' inertial (Heliocentric) positions, i.e. COM1 and COM2.
            dvec3 r1 = rcom + m1*rmut;
            dvec3 r2 = rcom + m2*rmut;
            //Corresponding body-to-spacecraft vector.
            dvec3 rho1 = rsp - r1;
            dvec3 rho2 = rsp - r2;

            //Spacecraft's acceleration due to the combined presence of the 2 rigid bodies (no Sun, no SRP yet).
            dvec3 asp;
            if (props.ord2_checkbox)
                asp = accel_integrals_ord2(rho1, props.M1, J1, A1) + accel_integrals_ord2(rho2, props.M2, J2, A2);
            else if (props.ord3_checkbox)
                asp = accel_integrals_ord3(rho1, props.M1, J1, A1) + accel_integrals_ord3(rho2, props.M2, J2, A2);
            else
                asp = accel_integrals_ord4(rho1, props.M1, J1, A1) + accel_integrals_ord4(rho2, props.M2, J2, A2);

            if (props.sun_gravity) //Add Sun's gravity.
            {
                double rsplen = length(rsp);
                asp = asp - G*MSUN*rsp/(rsplen*rsplen*rsplen);
            }

            if (props.srp_checkbox) //Add SRP.
            {
                bool sp_in_shadow = false;
                dvec3 rsun = 0.0; //Sun is now at the origin.
                if (props.srp_shadow_checkbox)
                    sp_in_shadow = line_sphere_intersection(rsp, rsun, r1, brillouin1) || line_sphere_intersection(rsp, rsun, r2, brillouin2);
                if (!sp_in_shadow)
                    asp = asp + accel_srp(props.sp_refl, props.sp_area, props.sp_mass, rsp, rsun);
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
    void prepare(console &cons)
    {
        cons.print("[Polyhedron] : Computing inertial integrals... ");

        m  =  props.M1*props.M2/(props.M1 + props.M2);
        m1 = -props.M2/(props.M1 + props.M2);
        m2 =  props.M1/(props.M1 + props.M2);

        //Preparation : If the user chose Keplerian elements as initial position/velocity, then, transform
        //them to Cartesian coords because the F2BP odes are written in Cartesian form.
        if (props.pos_vel_mut_var == properties::KEPLERIAN_MUT)
            props.cart_mut = kep2cart({props.kep_mut[0],
                                       props.kep_mut[1],
                                       props.kep_mut[2]*PI/180.0,
                                       props.kep_mut[3]*PI/180.0,
                                       props.kep_mut[4]*PI/180.0,
                                       props.kep_mut[5]*PI/180.0}, G*(props.M1 + props.M2));
        
        //Preparation : If the user chose Euler angles as initial orientations, then, transform them to
        //quaternions because the F2BP odes are written in quaternion form.
        if (props.orient_var == properties::EULER_XYZ)
        {
            props.q1 = ang2quat(props.rpy1*PI/180.0);
            props.q2 = ang2quat(props.rpy2*PI/180.0);
        }

        //Preparation : If the user chose to input the angular velocities in the inertial frame, then, transform them
        //to the corresponding body frames because the Euler equations of rotation are written in body frame form.
        if (props.angvel_frame == properties::INERTIAL_ANGVEL)
        {
            props.w1b = iner2body(props.w1i, quat2mat(props.q1));
            props.w2b = iner2body(props.w2i, quat2mat(props.q2));
        }

        //Preparation : If the user chose Keplerian elements as initial Heliocentric position/velocity of the COM, then, transform them to Cartesian.
        if (props.pos_vel_com_var == properties::KEPLERIAN_COM)
            props.cart_com = kep2cart({props.kep_com[0]*AU2KM,
                                       props.kep_com[1],
                                       props.kep_com[2]*PI/180.0,
                                       props.kep_com[3]*PI/180.0,
                                       props.kep_com[4]*PI/180.0,
                                       props.kep_com[5]*PI/180.0}, G*MSUN); //[km], [km/sec]
        else
        {
            props.cart_com[0] *= AU2KM;
            props.cart_com[1] *= AU2KM;
            props.cart_com[2] *= AU2KM;
            //[km], [km/sec]
        }

        //Preparation : Evaluate Brillouin radii and inertial integrals, based on the user's choice of shape model (ellipsoids or obj files).
        if (props.ell_checkbox)
        {
            brillouin1 = ell_brillouin(props.semiaxes1);
            brillouin2 = ell_brillouin(props.semiaxes2);
            I1 = ell_inertia(props.M1, props.semiaxes1);
            I2 = ell_inertia(props.M2, props.semiaxes2);
            if (props.ord2_checkbox)
            {
                J1 = ell_integrals(props.M1, props.semiaxes1, 2);
                J2 = ell_integrals(props.M2, props.semiaxes2, 2);
            }
            else if (props.ord3_checkbox)
            {
                J1 = ell_integrals(props.M1, props.semiaxes1, 3);
                J2 = ell_integrals(props.M2, props.semiaxes2, 3);
            }
            else
            {
                J1 = ell_integrals(props.M1, props.semiaxes1, 4);
                J2 = ell_integrals(props.M2, props.semiaxes2, 4);
            }
        }
        else //.obj files
        {
            props.poly1.set_com_zero();
            props.poly1.set_inertia_diagonal();
            brillouin1 = props.poly1.get_farthest_vertex_distance();
            
            props.poly2.set_com_zero();
            props.poly2.set_inertia_diagonal();
            brillouin2 = props.poly2.get_farthest_vertex_distance();

            I1 = props.poly1.get_inertia(props.M1);
            I2 = props.poly2.get_inertia(props.M2);
            if (props.ord2_checkbox)
            {
                J1 = props.poly1.get_inertial_integrals_ord2(props.M1);
                J2 = props.poly2.get_inertial_integrals_ord2(props.M2);
            }
            else if (props.ord3_checkbox)
            {
                J1 = props.poly1.get_inertial_integrals_ord3(props.M1);
                J2 = props.poly2.get_inertial_integrals_ord3(props.M2);
            }
            else
            {
                J1 = props.poly1.get_inertial_integrals_ord4(props.M1);
                J2 = props.poly2.get_inertial_integrals_ord4(props.M2);
            }
        }
        
        //Preparation : Set time parameters in [sec] for integration.
        t0 = props.epoch*86400.0; //[sec]
        tmax = t0 + props.dur*86400.0; //[sec]
        if (props.integration_method == properties::RKF78_FIXED || props.integration_method == properties::ABM5_FIXED)
            dt = props.step*86400.0; //[sec]
        else
            init_guess_time_step = ODES_INIT_GUESS_TIME_STEP; //[sec]

        if (props.impactors_checkbox)
        {
            props.tD1 *= 86400.0;
            props.tD2 *= 86400.0;
        }

        maneuver1 = maneuver2 = false;

        //Preparation : Spacecraft i.c. and SRP assumptions.
        if (!props.spacecraft_checkbox)
        {
            for (int i = 0; i < 6; ++i)
                props.cart_sp[i] = 0.0;
        }
        else
        {
            //First case is that the user set the i.c. in CARTESIAN_SP, so we do no check for that.
            //In any other case, we must evaluate the position and velocity in Cartesian coords relative to the binary's COM.
            if (props.pos_vel_sp_var == properties::CARTESIAN_SP1)
                props.cart_sp = m1*props.cart_mut + props.cart_sp1;
            else if (props.pos_vel_sp_var == properties::CARTESIAN_SP2)
                props.cart_sp = m2*props.cart_mut + props.cart_sp2;
            else if (props.pos_vel_sp_var == properties::KEPLERIAN_SP)
                 props.cart_sp = kep2cart({props.kep_sp[0],
                                           props.kep_sp[1],
                                           props.kep_sp[2]*PI/180.0,
                                           props.kep_sp[3]*PI/180.0,
                                           props.kep_sp[4]*PI/180.0,
                                           props.kep_sp[5]*PI/180.0}, G*(props.M1 + props.M2));
            else if (props.pos_vel_sp_var == properties::KEPLERIAN_SP1)
            {
                props.cart_sp1 = kep2cart({props.kep_sp1[0],
                                           props.kep_sp1[1],
                                           props.kep_sp1[2]*PI/180.0,
                                           props.kep_sp1[3]*PI/180.0,
                                           props.kep_sp1[4]*PI/180.0,
                                           props.kep_sp1[5]*PI/180.0}, G*props.M1);
                props.cart_sp = m1*props.cart_mut + props.cart_sp1;
            }
            else if (props.pos_vel_sp_var == properties::KEPLERIAN_SP2)
            {
                props.cart_sp2 = kep2cart({props.kep_sp2[0],
                                           props.kep_sp2[1],
                                           props.kep_sp2[2]*PI/180.0,
                                           props.kep_sp2[3]*PI/180.0,
                                           props.kep_sp2[4]*PI/180.0,
                                           props.kep_sp2[5]*PI/180.0}, G*props.M2);
                props.cart_sp = m2*props.cart_mut + props.cart_sp2;
            }
        }

        collision_mut = collision_sp = false;
        
        orbit.clear();

        cons.print("Done.\n");
    }

    void run(std::atomic<bool> &abort_flag, std::atomic<float> &progress, console &cons)
    {
        cons.print("[Integrator] : Propagating orbit... ");
        progress.store(0.0f);

        //Initial conditions.
        boost::array<double, N_ODES> state = {    props.cart_mut[0],     props.cart_mut[1],     props.cart_mut[2],
                                                  props.cart_mut[3],     props.cart_mut[4],     props.cart_mut[5],
                                                        props.q1[0],           props.q1[1],           props.q1[2], props.q1[3],
                                                       props.w1b[0],          props.w1b[1],          props.w1b[2],
                                                        props.q2[0],           props.q2[1],           props.q2[2], props.q2[3],
                                                       props.w2b[0],          props.w2b[1],          props.w2b[2],
                                                   props.cart_sp[0],      props.cart_sp[1],      props.cart_sp[2],
                                                   props.cart_sp[3],      props.cart_sp[4],      props.cart_sp[5] };
        
        double t = t0; //Initialize time.

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, N_ODES>> rkf78_fixed;
        auto rkf78_adaptive = boost::numeric::odeint::make_controlled(props.target_error, props.target_error, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, N_ODES>>());
        boost::numeric::odeint::bulirsch_stoer<boost::array<double, N_ODES>> bstoer_adaptive(props.target_error, props.target_error);
        boost::numeric::odeint::adams_bashforth_moulton<5, boost::array<double, N_ODES>> abm5_fixed;

        if (props.integration_method == properties::ABM5_FIXED) //Seed Adams-Bashforth-Moulton only if this is the requested method of integration.
            abm5_fixed.initialize(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
        
        //Shoot it!!!
        while (t <= tmax)
        {
            //Solution storage : append the current state into the 'orbit' matrix. No matter what, at least the initial condition is gonna be stored.
            if (props.spacecraft_checkbox)
                orbit.push_back({t, state[0],  state[1],  state[2],
                                    state[3],  state[4],  state[5],
                                    state[6],  state[7],  state[8],  state[9],
                                    state[10], state[11], state[12],
                                    state[13], state[14], state[15], state[16],
                                    state[17], state[18], state[19],
                                    state[20], state[21], state[22],
                                    state[23], state[24], state[25] });
            else
                orbit.push_back({t, state[0],  state[1],  state[2],
                                    state[3],  state[4],  state[5],
                                    state[6],  state[7],  state[8],  state[9],
                                    state[10], state[11], state[12],
                                    state[13], state[14], state[15], state[16],
                                    state[17], state[18], state[19] });

            //Kinetic impacts : apply corresponding maneuvers if impactors are chosen.
            if (props.impactors_checkbox)
            {
                if (!maneuver1 && (t + 1e-15) >= props.tD1)
                {
                    state[3] -= props.beta1*props.mD1*props.vD1[0]/props.M1;
                    state[4] -= props.beta1*props.mD1*props.vD1[1]/props.M1;
                    state[5] -= props.beta1*props.mD1*props.vD1[2]/props.M1;
                    if (props.spacecraft_checkbox)
                    {
                        state[23] -= props.beta1*props.mD1*props.vD1[0]/(props.M1 + props.M2);
                        state[24] -= props.beta1*props.mD1*props.vD1[1]/(props.M1 + props.M2);
                        state[25] -= props.beta1*props.mD1*props.vD1[2]/(props.M1 + props.M2);
                    }
                    maneuver1 = true;
                }
                if (!maneuver2 && (t + 1e-15) >= props.tD2)
                {
                    state[3] += props.beta2*props.mD2*props.vD2[0]/props.M2;
                    state[4] += props.beta2*props.mD2*props.vD2[1]/props.M2;
                    state[5] += props.beta2*props.mD2*props.vD2[2]/props.M2;
                    if (props.spacecraft_checkbox)
                    {
                        state[23] -= props.beta2*props.mD2*props.vD2[0]/(props.M1 + props.M2);
                        state[24] -= props.beta2*props.mD2*props.vD2[1]/(props.M1 + props.M2);
                        state[25] -= props.beta2*props.mD2*props.vD2[2]/(props.M1 + props.M2);
                    }
                    maneuver2 = true;
                }
            }

            //Collisions : check for asteroid-asteroid collision, spacecraft-asteroid collision or Sun-COM close approach.
            char buffer[128];
            if (props.collision_spheres)
            {
                //Asteroid-asteroid.
                if (sphere_sphere_collision(length(dvec3{state[0],state[1],state[2]}), brillouin1, brillouin2))
                {
                    sprintf(buffer,"< Collision (asteroid - asteroid) detected at t = %5.2lf [days]. >\n", t/86400.0);
                    cons.print(buffer);
                    collision_mut = true;
                    break;
                }
                //Spacecraft-asteroid.
                if (props.spacecraft_checkbox)
                {
                    const dvec3 r = {state[0],state[1],state[2]};
                    const dvec3 r1 = m1*r;
                    const dvec3 r2 = m2*r;
                    const dvec3 rsp = {state[20],state[21],state[22]};
                    if (sphere_point_collision(length(rsp - r1), brillouin1))
                    {
                        sprintf(buffer,"< Collision (asteroid - spacecraft) at t = %5.2lf [days]. >\n", t/86400.0);
                        cons.print(buffer);
                        collision_sp = true;
                        break;
                    }
                    if (sphere_point_collision(length(rsp - r2), brillouin2))
                    {
                        sprintf(buffer,"< Collision (asteroid - spacecraft) at t = %5.2lf [days]. >\n", t/86400.0);
                        cons.print(buffer);
                        collision_sp = true;
                        break;
                    }
                }
            }
            else if (props.collision_polyhedra)
            {
                const dvec3 r = {state[0],state[1],state[2]};
                const dvec3 r1 = m1*r;
                const dvec3 r2 = m2*r;
                //We apply sphere-sphere gate first, because polyhedron-polyhedron collision requires first the Brillouin spheres to collide (which is a lot faster to test).
                if (sphere_sphere_collision(length(r), brillouin1, brillouin2))
                {
                    const dmat3 A1 = quat2mat({state[6],  state[7],  state[8],  state[9]});
                    const dmat3 A2 = quat2mat({state[13], state[14], state[15], state[16]});
                    if (polyhedron_polyhedron_collision(props.poly1, A1, r1, props.poly2, A2, r2))
                    {
                        sprintf(buffer,"< Collision (asteroid - asteroid) at t = %5.2lf [days]. >\n", t/86400.0);
                        cons.print(buffer);
                        collision_mut = true;
                        break;
                    }
                }

                //Spacecraft-asteroid (again, sphere-point gate, then polyhedron-point).
                if (props.spacecraft_checkbox)
                {
                    const dvec3 rsp = {state[20],state[21],state[22]};
                    if (sphere_point_collision(length(rsp - r1), brillouin1))
                    {
                        const dmat3 A1 = quat2mat({state[6],state[7],state[8],state[9]});
                        if (polyhedron_point_collision(props.poly1, A1, r1, rsp))
                        {
                            sprintf(buffer,"< Collision (spacecraft - asteroid) at t = %5.2lf [days]. >\n", t/86400.0);
                            cons.print(buffer);
                            collision_sp = true;
                            break;
                        }
                    }
                    if (sphere_point_collision(length(rsp - r2), brillouin2))
                    {
                        const dmat3 A2 = quat2mat({state[13],state[14],state[15],state[16]});
                        if (polyhedron_point_collision(props.poly2, A2, r2, rsp))
                        {
                            sprintf(buffer,"< Collision (spacecraft - asteroid, polyhedron) at t = %5.2lf [days]. >\n", t/86400.0);
                            cons.print(buffer);
                            collision_sp = true;
                            break;
                        }
                    }
                }
            }
            
            //Sun-COM close approach :
            if (length(get_analytic_rcom(props.cart_com, t)) < MIN_SUN_BODY_DIST*AU2KM)
            {
                sprintf(buffer,"< Binary COM too close to Sun at t = %5.2lf [days]. >\n", t/86400.0);
                cons.print(buffer);
                break;
            }

            //Abort flag : the user might want to kill the integration via the 'Abort' button in the gui.
            if (abort_flag.load())
            {
                sprintf(buffer, "< Aborted at t = %5.2lf [days]. > \n", t/86400.0);
                cons.print(buffer);
                break;
            }
            
            //State update : do 1 step of whatever numerical method is chosen.
            if (props.integration_method == properties::RKF78_FIXED)
            {
                rkf78_fixed.do_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
                t += dt;
            }
            else if (props.integration_method == properties::RKF78_ADAPTIVE)
                rkf78_adaptive.try_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, init_guess_time_step);
            else if (props.integration_method == properties::BSTOER_ADAPTIVE)
                bstoer_adaptive.try_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, init_guess_time_step);
            else //properties::ABM5_FIXED
            {
                abm5_fixed.do_step(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
                t += dt;
            }
            //Note : Boost's do_step() does NOT update internally t, hence we have to do it ourselves. But try_step() DOES update internally t, hence we do not touch it in this case.

            //Progressbar : set the progress value of the integrator in [0,1].
            progress.store((t-t0)/(tmax-t0));
        }

        //If everything went right, give the 'Done.' message.
        if (!abort_flag.load() && !collision_mut && !collision_sp)
        {
            progress.store(1.0f);
            cons.print("Done.\n");
        }
    }
};

#endif
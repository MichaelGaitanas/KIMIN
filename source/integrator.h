/* This class handles the numerical integration of the F2RBP. */

#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include<atomic>
#include<boost/numeric/odeint.hpp>

#include"constants.h"
#include"typedef.h"
#include"linalg.h"
#include"geometry.h"
#include"conversion.h"
#include"ellipsoid.h"
#include"polyhedron.h"
#include"rigidbody.h"
#include"dynamics.h"
#include"properties.h"
#include"console.h"

class integrator
{
public:
    properties props; //COPY of user's choice of properties in the gui.
    dvec6 cart_sp_helio; //Heliocentric Cartesian state of the spacecraft (arcitectural convenience to have it as member).

    double m, m1, m2; //Reduced mass and reduced mass coefficients : m = M1*M2/(M1+M2), m1 = -M2/(M1+M2), m2 = M1/(M1+M2).
    dmat3 I1, I2, Isp; //Moments of inertia.
    dtens J1, J2, Jsp; //Inertial integrals.
    double brillouin1, brillouin2, brillouin_sp; //Brillouin radii of the 2 rigid bodies + spacecraft.
    bool collision_mut, collision_sp1, collision_sp2, collision_sun; //Collision flags.
    bool maneuver1, maneuver2; //Whether or not a beta-kick (equivalent maneuver) has been applied to the corresponding body.

    double t0, tmax, dt, init_guess_time_step; //Integration time parameters.
    
    dmat orbit; //This is the final solution matrix of the differential equations that will be solved (time + state vector).

    integrator() { } //Needed to instantiate an integrator object in the solution class.

    integrator(const properties &props) //Needed to instantiate the integrator in the gui class.
    {
        this->props = props; //Deep copy of the gui's properties. So from now on, whatever change happens here to any property, it happens on the copy.
    }

private:
    //This function builds the right hand sides of the differential equations of motion.
    void build_rhs(const boost::array<double, N_ODES> &state, boost::array<double, N_ODES> &dstate, double /*t*/)
    {
        //Extract binary's mutual position & velocity and absolute orientations & angular velocities.
        const dvec3 rmut = {state[0],  state[1],  state[2]};
        const dvec3 vmut = {state[3],  state[4],  state[5]};
        const dvec4 q1   = {state[6],  state[7],  state[8],  state[9]};
        const dvec3 w1b  = {state[10], state[11], state[12]};
        const dvec4 q2   = {state[13], state[14], state[15], state[16]};
        const dvec3 w2b  = {state[17], state[18], state[19]};
        //Extract COM's state.
        const dvec3 rcom_helio = {state[20], state[21], state[22]};
        const dvec3 vcom_helio = {state[23], state[24], state[25]};

        //Compute rotation matrices from normalized quaternions.
        const dmat3 A1 = quat2mat(q1), A2 = quat2mat(q2);

        //Calculate the mutual force and the torque acted on body 1 in the inertial (Heliocentric) frame.
        dvec6 force_torque1i;
        if (props.ord2_checkbox)
            force_torque1i = mut_force_torque1_integrals_ord2(rmut, props.M1,J1,A1, props.M2,J2,A2);
        else if (props.ord3_checkbox)
            force_torque1i = mut_force_torque1_integrals_ord3(rmut, props.M1,J1,A1, props.M2,J2,A2);
        else
            force_torque1i = mut_force_torque1_integrals_ord4(rmut, props.M1,J1,A1, props.M2,J2,A2);
        
        const dvec3 force = {force_torque1i[0], force_torque1i[1], force_torque1i[2]};
        //Mutual and COM acceleration due to binary's gravity.
        dvec3 amut = force/m;
        dvec3 acom_helio   = {0.0,0.0,0.0};
        dvec3 torque1b_sun = {0.0,0.0,0.0};
        dvec3 torque2b_sun = {0.0,0.0,0.0};
        if (props.sun_gravity) //Add Sun's contribution to both the amut and acom_helio.
        {
            //Individual bodies' inertial (Heliocentric) positions, i.e. COM1 and COM2.
            const dvec3 r1_helio = rcom_helio + m1*rmut;
            const dvec3 r2_helio = rcom_helio + m2*rmut;
            const double d1 = length(r1_helio);
            const double d2 = length(r2_helio);
            const dvec3 r1b_unit = iner2body(r1_helio, A1)/d1;
            const dvec3 r2b_unit = iner2body(r2_helio, A2)/d2;
            const double dcom = length(rcom_helio);
            amut = amut - G*MSUN*(r2_helio/(d2*d2*d2) - r1_helio/(d1*d1*d1));
            acom_helio = -G*MSUN*rcom_helio/(dcom*dcom*dcom);
            torque1b_sun = 3.0*G*MSUN*cross(r1b_unit, dot(I1,r1b_unit))/(d1*d1*d1);
            torque2b_sun = 3.0*G*MSUN*cross(r2b_unit, dot(I2,r2b_unit))/(d2*d2*d2);
        }
        //Else the mutual state is governed only by the binary's mutual gravity and the COM shall move at a straight line in space. Bye bye Solar system!

        const dvec3 torque1i = {force_torque1i[3], force_torque1i[4], force_torque1i[5]};
        const dvec3 torque2i = -torque1i - cross(rmut, force);

        //Convert torques into the corresponding body frames because Euler's ODEs are written in the body frame.
        const dvec3 torque1b = iner2body(torque1i,A1) + torque1b_sun;
        const dvec3 torque2b = iner2body(torque2i,A2) + torque2b_sun;

        const dvec4 dq1 = quat_rhs(q1,w1b);
        const dvec3 dw1b = euler_rhs(w1b,I1,torque1b);

        const dvec4 dq2 = quat_rhs(q2,w2b);
        const dvec3 dw2b = euler_rhs(w2b,I2,torque2b);

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
        dstate[20] = vcom_helio[0];
        dstate[21] = vcom_helio[1];
        dstate[22] = vcom_helio[2];

        //COM velocity rhs (vxcom,vycom,vzcom).
        dstate[23] = acom_helio[0];
        dstate[24] = acom_helio[1];
        dstate[25] = acom_helio[2];

        //Now handle the spacecraft's ODEs :
        if (props.spacecraft_checkbox)
        {
            //Precompute essential quantities :

            //Spacecraft's state in the inertial (Heliocentric) frame.
            const dvec3 rsp_helio = {state[26], state[27], state[28]};
            const dvec3 vsp_helio = {state[29], state[30], state[31]};
            //Individual bodies' inertial (Heliocentric) positions, i.e. COM1 and COM2.
            const dvec3 r1_helio = rcom_helio + m1*rmut;
            const dvec3 r2_helio = rcom_helio + m2*rmut;
            //Corresponding body-to-spacecraft vector.
            const dvec3 rho1 = rsp_helio - r1_helio;
            const dvec3 rho2 = rsp_helio - r2_helio;

            //Spacecraft's acceleration due to the combined presence of the 2 rigid bodies.
            dvec3 asp_helio = {0.0,0.0,0.0};

            //1) Add Sun's contribution to spacecraft's acceleration if chosen :
            if (props.sun_gravity)
            {
                const double dsp = length(rsp_helio);
                asp_helio = asp_helio - G*MSUN*rsp_helio/(dsp*dsp*dsp);
            }

            //2) Add SRP contribution to spacecraft's acceleration if chosen :
            if (props.srp_checkbox)
            {
                bool sp_in_shadow = false;
                const dvec3 rsun_helio = {0.0,0.0,0.0}; //Sun is at the origin.
                if (props.srp_shadow_checkbox)
                    sp_in_shadow = line_sphere_intersection(rsp_helio, rsun_helio, r1_helio, brillouin1) || line_sphere_intersection(rsp_helio, rsun_helio, r2_helio, brillouin2);
                if (!sp_in_shadow)
                    asp_helio = asp_helio + accel_srp(props.sp_refl, props.sp_area, props.sp_mass, rsp_helio, rsun_helio);
            }
            
            if (!props.sp_is_rigidbody_checkbox)
            {
                if (props.ord2_checkbox)
                    asp_helio = asp_helio + accel_integrals_ord2(rho1, props.M1, J1, A1) + accel_integrals_ord2(rho2, props.M2, J2, A2);
                else if (props.ord3_checkbox)
                    asp_helio = asp_helio + accel_integrals_ord3(rho1, props.M1, J1, A1) + accel_integrals_ord3(rho2, props.M2, J2, A2);
                else
                    asp_helio = asp_helio + accel_integrals_ord4(rho1, props.M1, J1, A1) + accel_integrals_ord4(rho2, props.M2, J2, A2);

                //Spacecraft's rotational rhs :
                for (int i = 32; i <= 38; ++i)
                    dstate[i] = 0.0;
            }
            else
            {
                const dvec4 qsp  = {state[32], state[33], state[34], state[35]};
                const dvec3 wspb = {state[36], state[37], state[38]};
                const dmat3 Asp  = quat2mat(qsp);

                dvec6 ft1, ft2; //Force and torque applied to the spacecraft.
                if (props.ord2_checkbox)
                {
                    ft1 = mut_force_torque1_integrals_ord2(rho1, props.sp_mass, Jsp, Asp, props.M1, J1, A1);
                    ft2 = mut_force_torque1_integrals_ord2(rho2, props.sp_mass, Jsp, Asp, props.M2, J2, A2);
                }
                else if (props.ord3_checkbox)
                {
                    ft1 = mut_force_torque1_integrals_ord3(rho1, props.sp_mass, Jsp, Asp, props.M1, J1, A1);
                    ft2 = mut_force_torque1_integrals_ord3(rho2, props.sp_mass, Jsp, Asp, props.M2, J2, A2);
                }
                else
                {
                    ft1 = mut_force_torque1_integrals_ord4(rho1, props.sp_mass, Jsp, Asp, props.M1, J1, A1);
                    ft2 = mut_force_torque1_integrals_ord4(rho2, props.sp_mass, Jsp, Asp, props.M2, J2, A2);
                }

                const dvec3 Fsp     = dvec3{ft1[0],ft1[1],ft1[2]} + dvec3{ft2[0],ft2[1],ft2[2]};
                const dvec3 tau_spi = dvec3{ft1[3],ft1[4],ft1[5]} + dvec3{ft2[3],ft2[4],ft2[5]};
                asp_helio = asp_helio + Fsp/props.sp_mass;

                dvec3 torque_spb_sun = {0.0,0.0,0.0};
                if (props.sun_gravity)
                {
                    const double dsp = length(rsp_helio);
                    const dvec3 rspb_unit = iner2body(rsp_helio, Asp)/dsp;
                    torque_spb_sun = 3.0*G*MSUN*cross(rspb_unit, dot(Isp, rspb_unit))/(dsp*dsp*dsp);
                }

                const dvec3 torque_spb = iner2body(tau_spi, Asp) + torque_spb_sun;

                const dvec4 dqsp  = quat_rhs(qsp, wspb);
                const dvec3 dwspb = euler_rhs(wspb, Isp, torque_spb);

                //Spacecraft's rotational rhs :
                dstate[32] = dqsp[0];
                dstate[33] = dqsp[1];
                dstate[34] = dqsp[2];
                dstate[35] = dqsp[3];
                dstate[36] = dwspb[0];
                dstate[37] = dwspb[1];
                dstate[38] = dwspb[2];
            }

            //Spacecraft's position and velocity rhs.
            dstate[26] = vsp_helio[0];
            dstate[27] = vsp_helio[1];
            dstate[28] = vsp_helio[2];
            dstate[29] = asp_helio[0];
            dstate[30] = asp_helio[1];
            dstate[31] = asp_helio[2];
        }
        else //no spacecraft was chosen, so set everything to zero that concerns the spacecraft.
            for (int i = 26; i <= 38; ++i)
                dstate[i] = 0.0;
    }

public:
    //Before the actual integration of the ODEs, we do the following preparations :
    void prepare(console &cons)
    {
        cons.print("[Integrator] : Preparing integrator... ");

        m  =  props.M1*props.M2/(props.M1 + props.M2);
        m1 = -props.M2/(props.M1 + props.M2);
        m2 =  props.M1/(props.M1 + props.M2);

        //Preparation : If the user chose Keplerian elements as initial position/velocity, then, transform them to Cartesian coords.
        if (props.pos_vel_mut_var == properties::KEPLERIAN_MUT)
            props.cart_mut = kep2cart({props.kep_mut[0],
                                       props.kep_mut[1],
                                       props.kep_mut[2]*PI/180.0,
                                       props.kep_mut[3]*PI/180.0,
                                       props.kep_mut[4]*PI/180.0,
                                       props.kep_mut[5]*PI/180.0}, G*(props.M1 + props.M2));
        
        //Preparation : If the user chose Euler angles as initial orientations, then, transform them to quaternions.
        if (props.orient_var == properties::EULER_XYZ)
        {
            props.q1 = ang2quat(props.rpy1*PI/180.0);
            props.q2 = ang2quat(props.rpy2*PI/180.0);
        }

        //Preparation : If the user chose to input the angular velocities in the inertial frame, then, transform them to the corresponding body frames.
        if (props.angvel_frame == properties::ANGVEL_HELIO)
        {
            props.w1b = iner2body(props.w1i, quat2mat(props.q1));
            props.w2b = iner2body(props.w2i, quat2mat(props.q2));
        }

        //Preparation : If the user chose Keplerian elements as initial Heliocentric position/velocity of the COM, then, transform them to Cartesian.
        if (props.pos_vel_com_var == properties::KEPLERIAN_COM_HELIO)
            props.cart_com_helio = kep2cart({props.kep_com_helio[0]*AU2KM,
                                             props.kep_com_helio[1],
                                             props.kep_com_helio[2]*PI/180.0,
                                             props.kep_com_helio[3]*PI/180.0,
                                             props.kep_com_helio[4]*PI/180.0,
                                             props.kep_com_helio[5]*PI/180.0}, G*MSUN); //[km], [km/sec]
        else
        {
            props.cart_com_helio[0] *= AU2KM;
            props.cart_com_helio[1] *= AU2KM;
            props.cart_com_helio[2] *= AU2KM;
            //[km]
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
        else //obj files
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

        //Preparation : In the app's GUI, the spacecraft's i.c. is in either COM frame or body 1 frame (COM1) or body 2 frame (COM2).
        //But the build_rhs() is in Heliocentric frame. Therefore we convert position/velocity in Heliocentric Cartesian coords.
        cart_sp_helio = {0.0,0.0,0.0,0.0,0.0,0.0};
        if (props.spacecraft_checkbox)
        {
            if (props.pos_vel_sp_var == properties::CARTESIAN_SP_COM)
                cart_sp_helio = props.cart_com_helio + props.cart_sp_com;
            else if (props.pos_vel_sp_var == properties::CARTESIAN_SP_COM1)
                cart_sp_helio = props.cart_com_helio + m1*props.cart_mut + props.cart_sp_com1;
            else if (props.pos_vel_sp_var == properties::CARTESIAN_SP_COM2)
                cart_sp_helio = props.cart_com_helio + m2*props.cart_mut + props.cart_sp_com2;
            else if (props.pos_vel_sp_var == properties::KEPLERIAN_SP_COM)
            {
                props.cart_sp_com = kep2cart({props.kep_sp_com[0],
                                              props.kep_sp_com[1],
                                              props.kep_sp_com[2]*PI/180.0,
                                              props.kep_sp_com[3]*PI/180.0,
                                              props.kep_sp_com[4]*PI/180.0,
                                              props.kep_sp_com[5]*PI/180.0}, G*(props.M1 + props.M2));
                cart_sp_helio = props.cart_com_helio + props.cart_sp_com;
            }
            else if (props.pos_vel_sp_var == properties::KEPLERIAN_SP_COM1)
            {
                props.cart_sp_com1 = kep2cart({props.kep_sp_com1[0],
                                               props.kep_sp_com1[1],
                                               props.kep_sp_com1[2]*PI/180.0,
                                               props.kep_sp_com1[3]*PI/180.0,
                                               props.kep_sp_com1[4]*PI/180.0,
                                               props.kep_sp_com1[5]*PI/180.0}, G*props.M1);
                cart_sp_helio = props.cart_com_helio + m1*props.cart_mut + props.cart_sp_com1;
            }
            else if (props.pos_vel_sp_var == properties::KEPLERIAN_SP_COM2)
            {
                props.cart_sp_com2 = kep2cart({props.kep_sp_com2[0],
                                               props.kep_sp_com2[1],
                                               props.kep_sp_com2[2]*PI/180.0,
                                               props.kep_sp_com2[3]*PI/180.0,
                                               props.kep_sp_com2[4]*PI/180.0,
                                               props.kep_sp_com2[5]*PI/180.0}, G*props.M2);
                cart_sp_helio = props.cart_com_helio + m2*props.cart_mut + props.cart_sp_com2;
            }

            if (props.sp_is_rigidbody_checkbox)
            {
                //Preparation : If the user chose Euler angles as initial orientation, then, transform them to quaternion.
                if (props.sp_orient_var == properties::EULER_XYZ_SP)
                    props.qsp = ang2quat(props.rpy_sp*PI/180.0);

                //Preparation : If the user chose to input the angular velocity in the inertial frame, then, transform iy to the body frame.
                if (props.sp_angvel_frame == properties::ANGVEL_SP_HELIO)
                    props.wb_sp = iner2body(props.wi_sp, quat2mat(props.qsp));

                //Preparation : Evaluate Brillouin radius and inertial integrals of the spacecraft, based on the user's choice of shape model (ellipsoid or obj file).
                if (props.sp_ell_checkbox)
                {
                    brillouin_sp = ell_brillouin(props.sp_semiaxes);
                    Isp = ell_inertia(props.sp_mass, props.sp_semiaxes);
                    if (props.ord2_checkbox)
                        Jsp = ell_integrals(props.sp_mass, props.sp_semiaxes, 2);
                    else if (props.ord3_checkbox)
                        Jsp = ell_integrals(props.sp_mass, props.sp_semiaxes, 3);
                    else
                        Jsp = ell_integrals(props.sp_mass, props.sp_semiaxes, 4);
                }
                else //obj file
                {
                    props.poly_sp.set_com_zero();
                    props.poly_sp.set_inertia_diagonal();
                    brillouin_sp = props.poly_sp.get_farthest_vertex_distance();
                    Isp = props.poly_sp.get_inertia(props.sp_mass);
                    if (props.ord2_checkbox)
                        Jsp = props.poly_sp.get_inertial_integrals_ord2(props.sp_mass);
                    else if (props.ord3_checkbox)
                        Jsp = props.poly_sp.get_inertial_integrals_ord3(props.sp_mass);
                    else
                        Jsp = props.poly_sp.get_inertial_integrals_ord4(props.sp_mass);
                }
            }
        }
        
        //Preparation : Set time parameters in [sec] for integration.
        t0 = (props.epoch_jd - JD_J2000)*DAY2SEC; //[sec]
        tmax = t0 + props.dur*DAY2SEC; //[sec]
        if (props.integration_method == properties::RKF78_FIXED || props.integration_method == properties::ABM5_FIXED)
            dt = props.step*DAY2SEC; //[sec]
        else
            init_guess_time_step = ODES_INIT_GUESS_TIME_STEP; //[sec]

        if (props.impactors_checkbox)
        {
            props.tD1 = t0 + props.tD1*DAY2SEC; //[sec]
            props.tD2 = t0 + props.tD2*DAY2SEC; //[sec]
        }

        maneuver1 = maneuver2 = false;
        collision_mut = collision_sp1 = collision_sp2 = collision_sun = false;
        
        orbit.clear();

        cons.print("Done.\n");
    }

    void run(std::atomic<bool> &abort_flag, std::atomic<float> &progress, console &cons)
    {
        cons.print("[Integrator] : Propagating orbit... ");
        progress.store(0.0f);

        //Initial conditions : everything must be in the Heliocentric frame.
        double t = t0;
        boost::array<double, N_ODES> state = {       props.cart_mut[0],        props.cart_mut[1],        props.cart_mut[2],
                                                     props.cart_mut[3],        props.cart_mut[4],        props.cart_mut[5],
                                                           props.q1[0],              props.q1[1],              props.q1[2], props.q1[3],
                                                          props.w1b[0],             props.w1b[1],             props.w1b[2],
                                                           props.q2[0],              props.q2[1],              props.q2[2], props.q2[3],
                                                          props.w2b[0],             props.w2b[1],             props.w2b[2],
                                               props.cart_com_helio[0],  props.cart_com_helio[1],  props.cart_com_helio[2],
                                               props.cart_com_helio[3],  props.cart_com_helio[4],  props.cart_com_helio[5],
                                                      cart_sp_helio[0],         cart_sp_helio[1],         cart_sp_helio[2],
                                                      cart_sp_helio[3],         cart_sp_helio[4],         cart_sp_helio[5],
                                                          props.qsp[0],             props.qsp[1],             props.qsp[2], props.qsp[3],
                                                        props.wb_sp[0],           props.wb_sp[1],           props.wb_sp[2] };

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, N_ODES>> rkf78_fixed;
        auto rkf78_adaptive = boost::numeric::odeint::make_controlled(props.target_error, props.target_error, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, N_ODES>>());
        boost::numeric::odeint::bulirsch_stoer<boost::array<double, N_ODES>> bstoer_adaptive(props.target_error, props.target_error);
        boost::numeric::odeint::adams_bashforth_moulton<5, boost::array<double, N_ODES>> abm5_fixed;

        if (props.integration_method == properties::ABM5_FIXED) //Seed Adams-Bashforth-Moulton only if this is the requested method of integration.
            abm5_fixed.initialize(std::bind(&integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
        
        //Shoot it!!!
        while (t <= tmax)
        {
            //Solution storage : append the current state into the 'orbit' matrix. No matter what, at least the initial condition is gonna be stored if the execution reaches run().
            if (props.spacecraft_checkbox && !props.sp_is_rigidbody_checkbox)
                orbit.push_back({t, state[0],  state[1],  state[2],
                                    state[3],  state[4],  state[5],
                                    state[6],  state[7],  state[8],  state[9],
                                    state[10], state[11], state[12],
                                    state[13], state[14], state[15], state[16],
                                    state[17], state[18], state[19],
                                    state[20], state[21], state[22],
                                    state[23], state[24], state[25],
                                    state[26], state[27], state[28],
                                    state[29], state[30], state[31] });
            else if (props.spacecraft_checkbox && props.sp_is_rigidbody_checkbox)
                orbit.push_back({t, state[0],  state[1],  state[2],
                                    state[3],  state[4],  state[5],
                                    state[6],  state[7],  state[8],  state[9],
                                    state[10], state[11], state[12],
                                    state[13], state[14], state[15], state[16],
                                    state[17], state[18], state[19],
                                    state[20], state[21], state[22],
                                    state[23], state[24], state[25],
                                    state[26], state[27], state[28],
                                    state[29], state[30], state[31],
                                    state[32], state[33], state[34], state[35],
                                    state[36], state[37], state[38] });
            else
                orbit.push_back({t, state[0],  state[1],  state[2],
                                    state[3],  state[4],  state[5],
                                    state[6],  state[7],  state[8],  state[9],
                                    state[10], state[11], state[12],
                                    state[13], state[14], state[15], state[16],
                                    state[17], state[18], state[19],
                                    state[20], state[21], state[22],
                                    state[23], state[24], state[25] });

            //Kinetic impacts : apply corresponding maneuvers if impactors are chosen.
            if (props.impactors_checkbox)
            {
                if (!maneuver1 && (t + 1e-15) >= props.tD1)
                {
                    state[3]  -= props.beta1*props.mD1*props.vD1[0]/props.M1;
                    state[4]  -= props.beta1*props.mD1*props.vD1[1]/props.M1;
                    state[5]  -= props.beta1*props.mD1*props.vD1[2]/props.M1;
                    state[23] += props.beta1*props.mD1*props.vD1[0]/(props.M1 + props.M2);
                    state[24] += props.beta1*props.mD1*props.vD1[1]/(props.M1 + props.M2);
                    state[25] += props.beta1*props.mD1*props.vD1[2]/(props.M1 + props.M2);
                    maneuver1 = true;
                }
                if (!maneuver2 && (t + 1e-15) >= props.tD2)
                {
                    state[3]  += props.beta2*props.mD2*props.vD2[0]/props.M2;
                    state[4]  += props.beta2*props.mD2*props.vD2[1]/props.M2;
                    state[5]  += props.beta2*props.mD2*props.vD2[2]/props.M2;
                    state[23] += props.beta2*props.mD2*props.vD2[0]/(props.M1 + props.M2);
                    state[24] += props.beta2*props.mD2*props.vD2[1]/(props.M1 + props.M2);
                    state[25] += props.beta2*props.mD2*props.vD2[2]/(props.M1 + props.M2);
                    maneuver2 = true;
                }
            }

            //Collisions : check for asteroid-asteroid collision, spacecraft-asteroid collision or Sun-COM close approach.
            char buffer[128];
            if (props.collision_spheres)
            {
                //Asteroid-asteroid :
                if (sphere_sphere_collision(length(dvec3{state[0],state[1],state[2]}), brillouin1, brillouin2))
                {
                    sprintf(buffer,"< Collision (asteroid - asteroid) detected after t = %5.2lf [days]. >\n", (t-t0)/DAY2SEC);
                    cons.print(buffer);
                    collision_mut = true;
                    break;
                }
                //Spacecraft-asteroid :
                if (props.spacecraft_checkbox)
                {
                    const dvec3 rmut = {state[0], state[1], state[2]};
                    const dvec3 rcom_helio = {state[20], state[21], state[22]};
                    const dvec3 r1_helio = rcom_helio + m1*rmut;
                    const dvec3 r2_helio = rcom_helio + m2*rmut;
                    const dvec3 rsp_helio = {state[26], state[27], state[28]};
                    if (!props.sp_is_rigidbody_checkbox)
                    {
                        if (sphere_point_collision(length(rsp_helio - r1_helio), brillouin1))
                        {
                            sprintf(buffer,"< Collision (spacecraft - asteroid 1) after t = %5.2lf [days]. >\n", (t-t0)/DAY2SEC);
                            cons.print(buffer);
                            collision_sp1 = true;
                            break;
                        }
                        if (sphere_point_collision(length(rsp_helio - r2_helio), brillouin2))
                        {
                            sprintf(buffer,"< Collision (spacecraft - asteroid 2) after t = %5.2lf [days]. >\n", (t-t0)/DAY2SEC);
                            cons.print(buffer);
                            collision_sp2 = true;
                            break;
                        }
                    }
                    else
                    {
                        if (sphere_sphere_collision(length(rsp_helio - r1_helio), brillouin1, brillouin_sp))
                        {
                            sprintf(buffer,"< Collision (spacecraft - asteroid 1) detected after t = %5.2lf [days]. >\n", (t-t0)/DAY2SEC);
                            cons.print(buffer);
                            collision_sp1 = true;
                            break;
                        }
                        if (sphere_sphere_collision(length(rsp_helio - r2_helio), brillouin2, brillouin_sp))
                        {
                            sprintf(buffer,"< Collision (spacecraft - asteroid 2) detected after t = %5.2lf [days]. >\n", (t-t0)/DAY2SEC);
                            cons.print(buffer);
                            collision_sp2 = true;
                            break;
                        }
                    }
                }
            }
            
            //Sun-COM close approach :
            if (length(dvec3{state[20], state[21], state[22]}) < MIN_SUN_BODY_DIST*AU2KM)
            {
                sprintf(buffer,"< Binary COM too close to Sun after t = %5.2lf [days]. >\n", (t-t0)/DAY2SEC);
                cons.print(buffer);
                collision_sun = true;
                break;
            }

            //Abort flag : if the user presses the 'Abort' button in the gui, break the loop.
            if (abort_flag.load())
            {
                sprintf(buffer, "< Aborted after t = %5.2lf [days]. > \n", (t-t0)/DAY2SEC);
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
            //Note : Boost's do_step() does NOT update internally t, hence we have to do it ourselves. But try_step() DOES update t internally, hence we do not touch it in this case.

            //Quaternion normalization :
            double norm = length(dvec4{state[6], state[7], state[8], state[9]}); //q1
            for (int i = 6; i <= 9; ++i)
                state[i] /= norm;
            norm = length(dvec4{state[13], state[14], state[15], state[16]}); //q2
            for (int i = 13; i <= 16; ++i)
                state[i] /= norm;
            
            if (props.spacecraft_checkbox && props.sp_is_rigidbody_checkbox)
            {
                norm = length(dvec4{state[32], state[33], state[34], state[35]}); //qsp
                for (int i = 32; i <= 35; ++i)
                    state[i] /= norm;
            }
                
            //Progressbar : set the progress value of the integrator in [0,1]. The properties then convert it to [0,100]
            progress.store((t-t0)/(tmax-t0));
        }

        //If everything went right, give the 'Done.' message.
        if (!abort_flag.load() && !collision_mut && !collision_sp1 && !collision_sp2 && !collision_sun)
        {
            progress.store(1.0f);
            cons.print("Done.\n");
        }
    }
};

#endif
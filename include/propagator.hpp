#ifndef PROPAGATOR_HPP
#define PROPAGATOR_HPP

#include<cstdio>
#include<cmath>
#include<filesystem>
#include<vector>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"task.hpp"

#include"conversion.hpp"
#include"polyhedron.hpp"
#include"mascon.hpp"
#include"ellipsoid.hpp"
#include"rigidbody.hpp"
#include"force.hpp"
#include"torque.hpp"

#include<boost/numeric/odeint.hpp>

//#include<json/json.h>

class Propagator : public Properties
{
public:

    dmatnx3 masc1, masc2; //mascon cloud coordinates
    dtens J1, J2; //inertial integrals
    dmat3 I1, I2; //moments of inertia
    bool collision; //binary collision flag
    dmat sol; //final solution matrix of the binary

    //////////////////////////////////////////////////////////////

    Propagator(Properties &props)
    {
        if (props.ell_checkbox)
        {
            if (props.ord2_checkbox)
            {
                J1 = ell_integrals(props.M1, props.semiaxes1, 2);
                J2 = ell_integrals(props.M2, props.semiaxes2, 2);
                I1 = ell_inertia(props.M1, props.semiaxes1);
                I2 = ell_inertia(props.M2, props.semiaxes2);
            }
            else if (props.ord3_checkbox)
            {
                J1 = ell_integrals(props.M1, props.semiaxes1, 3);
                J2 = ell_integrals(props.M2, props.semiaxes2, 3);
                I1 = ell_inertia(props.M1, props.semiaxes1);
                I2 = ell_inertia(props.M2, props.semiaxes2);
            }
            else if (props.ord4_checkbox)
            {
                J1 = ell_integrals(props.M1, props.semiaxes1, 4);
                J2 = ell_integrals(props.M2, props.semiaxes2, 4);
                I1 = ell_inertia(props.M1, props.semiaxes1);
                I2 = ell_inertia(props.M2, props.semiaxes2);
            }
            else
            {
                masc1 = fill_ell_with_masc(props.semiaxes1, props.grid_reso1);
                masc2 = fill_ell_with_masc(props.semiaxes2, props.grid_reso2);
                I1 = masc_inertia(props.M1, masc1);
                I2 = masc_inertia(props.M2, masc2);
            }
        }
        else //obj_checkbox
        {
            //temporary variables to load essential stuff from the .obj files
            dmatnx3 verts;
            dmatnx3 norms;
            dmatnx2 texs;
            imatnx3 facesnx3; //for the vf cases
            imatnx6 facesnx6; //for the vfn or vft cases
            imatnx9 facesnx9; //for the vfnt cases

            //body 1
            if (props.clicked_masc1_index != -1) //then the user clicked a mascon model from the database for body 1
                masc1 = loadobjv(props.obj_path1.c_str());
            else //the user clicked polyhedron model from the database for body 1
            {
                if (!props.vfnt1[2] && !props.vfnt1[3]) //vertices and faces
                    loadobjvf(props.obj_path1.c_str(), verts,facesnx3);
                else if (props.vfnt1[2] && !props.vfnt1[3]) //vertices, faces, norms
                {
                    loadobjvfn(props.obj_path1.c_str(), verts,facesnx6,norms);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (!props.vfnt1[2] && props.vfnt1[3]) //vertices, faces, textures
                {
                    loadobjvft(props.obj_path1.c_str(), verts,facesnx6,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (props.vfnt1[2] && props.vfnt1[3]) //vertices, faces, norms, textures
                {
                    loadobjvfnt(props.obj_path1.c_str(), verts,facesnx9,norms,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx9.size(); ++i)
                        facesnx3.push_back({facesnx9[i][0], facesnx9[i][3], facesnx9[i][6]});
                }

                masc1 = fill_poly_with_masc(verts,facesnx3, props.grid_reso1);
                //Don't forget to correct the mascon distribution.
                //Pull the center of mass towards 0 and rotate the axes to the principal ones.
            }

            //body 2
            if (props.clicked_masc2_index != -1) //then the user clicked a mascon model from the database for body 2
                masc2 = loadobjv(props.obj_path2.c_str());
            else //the user clicked polyhedron model from the database for body 2
            {
                if (!props.vfnt2[2] && !props.vfnt2[3]) //vertices and faces
                    loadobjvf(props.obj_path2.c_str(), verts,facesnx3);
                else if (props.vfnt2[2] && !props.vfnt2[3]) //vertices, faces, norms
                {
                    loadobjvfn(props.obj_path2.c_str(), verts,facesnx6,norms);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (!props.vfnt2[2] && props.vfnt2[3]) //vertices, faces, textures
                {
                    loadobjvft(props.obj_path2.c_str(), verts,facesnx6,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (props.vfnt2[2] && props.vfnt2[3]) //vertices, faces, norms, textures
                {
                    loadobjvfnt(props.obj_path2.c_str(), verts,facesnx9,norms,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx9.size(); ++i)
                        facesnx3.push_back({facesnx9[i][0], facesnx9[i][3], facesnx9[i][6]});
                }

                masc2 = fill_poly_with_masc(verts,facesnx3, props.grid_reso2);
                //Don't forget to correct the mascon distribution.
                //Pull the center of mass towards 0 and rotate the axes to the principal ones.
            }

            if (props.ord2_checkbox)
            {
                J1 = masc_integrals(props.M1, masc1, 2);
                J2 = masc_integrals(props.M2, masc2, 2);
                I1 = masc_inertia(props.M1, masc1);
                I2 = masc_inertia(props.M2, masc2);
            }
            else if (props.ord3_checkbox)
            {
                J1 = masc_integrals(props.M1, masc1, 3);
                J2 = masc_integrals(props.M2, masc2, 3);
                I1 = masc_inertia(props.M1, masc1);
                I2 = masc_inertia(props.M2, masc2);
            }
            else if (props.ord4_checkbox)
            {
                J1 = masc_integrals(props.M1, masc1, 4);
                J2 = masc_integrals(props.M2, masc2, 4);
                I1 = masc_inertia(props.M1, masc1);
                I2 = masc_inertia(props.M2, masc2);
            }
            //If pure mascons theory is selected, no additional parameter must be computed. masc1 and masc2 suffice. 
        }
        
        if (props.cart_kep_var_choice == 1) //the user chose Keplerian elements as ic
            props.relcart = kep2cart(props.relkep, G*(props.M1+props.M2)); //transform Keplerian to Cartesian because the f2bp propagator is written in Cartesian elements

        if (props.orient_var_choice == 0) //orientation variables were chosen to be Euler angles
        {
            //transform Euler angles (roll, pitch, yaw) to quaternions because the f2bp propagator is written in quaternionic structure
            props.q1 = ang2quat(props.rpy1);
            props.q2 = ang2quat(props.rpy2);
        }

        if (props.frame_type_choice == 0) //angular velocities were given in the "world" inertial frame
        {
            //transform inertial angular velocities to the corresponding body ones because the Euler equations of rotation are written in the body frames
            props.w1b = iner2body(props.w1i, quat2mat(props.q1));
            props.w2b = iner2body(props.w2i, quat2mat(props.q2));
        }

        temp_ord2_checkbox = props.ord2_checkbox;
        temp_ord3_checkbox = props.ord3_checkbox;
        temp_ord4_checkbox = props.ord4_checkbox;
        temp_M1 = props.M1;
        temp_M2 = props.M2;
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
        if (temp_ord2_checkbox)
        {
            force = mut_force_integrals_ord2(r, temp_M1,J1,A1, temp_M2,J2,A2);
            tau1i = mut_torque_integrals_ord2(r, J1,A1, temp_M2);
        }
        else if (temp_ord3_checkbox)
        {
            force = mut_force_integrals_ord3(r, temp_M1,J1,A1, temp_M2,J2,A2);
            tau1i = mut_torque_integrals_ord3(r, J1,A1, temp_M2);
        } 
        else if (temp_ord4_checkbox)
        {
            force = mut_force_integrals_ord4(r, temp_M1,J1,A1, temp_M2,J2,A2);
            tau1i = mut_torque_integrals_ord4(r, temp_M1,J1,A1, temp_M2,J2,A2);
        }
        else
        {
            force = mut_force_masc(r, temp_M1,masc1,A1, temp_M2,masc2,A2);
            tau1i = mut_torque_masc(r, temp_M1,masc1,A1, temp_M2,masc2,A2);
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
        dstate[3] = force[0]/(temp_M1*temp_M2/(temp_M1+temp_M2));
        dstate[4] = force[1]/(temp_M1*temp_M2/(temp_M1+temp_M2));
        dstate[5] = force[2]/(temp_M1*temp_M2/(temp_M1+temp_M2));

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

    void run()
    {
        //initial conditions (boost::array<> must be used to call the integration method)
        boost::array<double, 20> state = { relcart[0], relcart[1], relcart[2],
                                           relcart[3], relcart[4], relcart[5],
                                                q1[0],      q1[1],      q1[2], q1[3],
                                               w1b[0],     w1b[1],     w1b[2],
                                                q2[0],      q2[1],      q2[2], q2[3],
                                               w2b[0],     w2b[1],     w2b[2] };

        double t0 = epoch*86400.0; //[sec]
        double tmax = t0 + dur*86400.0; //[sec]
        double dt = step*86400.0; //[sec]

        collision = false; //no collision initially

        sol.clear();

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>> rkf78; //integration method

        //propagation loop
        for (double t = t0; t <= tmax; t += dt)
        {
            //1) save current state
            sol.push_back({t,
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
                break; //kill the propagation
            }

            //3) update the state vector
            rkf78.do_step(std::bind(&properties::odes, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
        }

        return;
    }

    
};


#endif
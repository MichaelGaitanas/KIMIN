#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include<cstdio>
#include<cmath>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"obj.hpp"
#include"conversion.hpp"
#include"polyhedron.hpp"
#include"mascon.hpp"
#include"ellipsoid.hpp"
#include"rigidbody.hpp"
#include"potential.hpp"
#include"force.hpp"
#include"torque.hpp"
#include"gui.hpp"

#include<boost/numeric/odeint.hpp>


/*
class Solution
{
public:

    dvec t;

    dvec x,y,z,d;
    dvec vx,vy,vz,v;
    dvec a,e,i,raan,w,M;

    dvec roll1,pitch1,yaw1;
    dvec w1ix,w1iy,w1iz;
    dvec w1bx,w1by,w1bz;

    dvec roll2,pitch2,yaw2;
    dvec w2ix,w2iy,w2iz;
    dvec w2bx,w2by,w2bz;

    dvec ener, ener_rel_err;
    dvec mom, mom_rel_err;
};
*/

class Integrator : public Properties
{
public:

    //bool ell_checkbox;

    dmatnx3 masc1, masc2; //mascon cloud coordinates
    dtens J1, J2; //inertial integrals
    dmat3 I1, I2; //moments of inertia

    bool collision; //binary collision flag
    dmat minimal_sol; //final solution matrix of the binary

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
            //temporary variables to load essential stuff from the .obj files
            dmatnx3 verts;
            dmatnx3 norms;
            dmatnx2 texs;
            imatnx3 facesnx3; //for the vf cases
            imatnx6 facesnx6; //for the vfn or vft cases
            imatnx9 facesnx9; //for the vfnt cases

            //body 1
            if (clicked_masc1_index != -1) //then the user clicked a mascon model from the database for body 1
                masc1 = loadobjv(obj_path1.c_str());
            else //the user clicked polyhedron model from the database for body 1
            {
                if (!vfnt1[2] && !vfnt1[3]) //vertices and faces
                    loadobjvf(obj_path1.c_str(), verts,facesnx3);
                else if (vfnt1[2] && !vfnt1[3]) //vertices, faces, norms
                {
                    loadobjvfn(obj_path1.c_str(), verts,facesnx6,norms);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (!vfnt1[2] && vfnt1[3]) //vertices, faces, textures
                {
                    loadobjvft(obj_path1.c_str(), verts,facesnx6,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (vfnt1[2] && vfnt1[3]) //vertices, faces, norms, textures
                {
                    loadobjvfnt(obj_path1.c_str(), verts,facesnx9,norms,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx9.size(); ++i)
                        facesnx3.push_back({facesnx9[i][0], facesnx9[i][3], facesnx9[i][6]});
                }

                masc1 = fill_poly_with_masc(verts,facesnx3, grid_reso1);
                //Don't forget to correct the mascon distribution.
                //Pull the center of mass towards 0 and rotate the axes to the principal ones.
            }

            //body 2
            if (clicked_masc2_index != -1) //then the user clicked a mascon model from the database for body 2
                masc2 = loadobjv(obj_path2.c_str());
            else //the user clicked polyhedron model from the database for body 2
            {
                if (!vfnt2[2] && !vfnt2[3]) //vertices and faces
                    loadobjvf(obj_path2.c_str(), verts,facesnx3);
                else if (vfnt2[2] && !vfnt2[3]) //vertices, faces, norms
                {
                    loadobjvfn(obj_path2.c_str(), verts,facesnx6,norms);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (!vfnt2[2] && vfnt2[3]) //vertices, faces, textures
                {
                    loadobjvft(obj_path2.c_str(), verts,facesnx6,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx6.size(); ++i)
                        facesnx3.push_back({facesnx6[i][0], facesnx6[i][2], facesnx6[i][4]});
                }
                else if (vfnt2[2] && vfnt2[3]) //vertices, faces, norms, textures
                {
                    loadobjvfnt(obj_path2.c_str(), verts,facesnx9,norms,texs);
                    //preserve only face indices
                    for (int i = 0; i < facesnx9.size(); ++i)
                        facesnx3.push_back({facesnx9[i][0], facesnx9[i][3], facesnx9[i][6]});
                }

                masc2 = fill_poly_with_masc(verts,facesnx3, grid_reso2);
                //Don't forget to correct the mascon distribution.
                //Pull the center of mass towards 0 and rotate the axes to the principal ones.
            }

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
            //If pure mascons theory is selected, no additional parameter must be computed. masc1 and masc2 suffice. 
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

    bool run()
    //Solution run()
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
        minimal_sol.clear();

        boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>> rkf78; //integration method

        //integration loop
        for (double t = t0; t <= tmax; t += dt)
        {
            //1) save current state
            minimal_sol.push_back({t,
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
                break; //kill the integration
            }

            //3) update the state vector
            rkf78.do_step(std::bind(&Integrator::build_rhs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), state, t, dt);
        }

        /*
        Solution solution;
        for (int i = 0; i < minimal_sol.size(); ++i)
        {
            double t = minimal_sol[i][0];
            dvec3 r = {minimal_sol[i][1], minimal_sol[i][2], minimal_sol[i][3]};
            dvec3 v = {minimal_sol[i][4], minimal_sol[i][5], minimal_sol[i][6]};
            dvec4 q1 = {minimal_sol[i][7], minimal_sol[i][8], minimal_sol[i][9], minimal_sol[i][10]};
            dvec3 w1b = {minimal_sol[i][11], minimal_sol[i][12], minimal_sol[i][13]};
            dvec4 q2 = {minimal_sol[i][14], minimal_sol[i][15], minimal_sol[i][16], minimal_sol[i][17]};
            dvec3 w2b = {minimal_sol[i][18], minimal_sol[i][19], minimal_sol[i][20]};
            dmat3 A1 = quat2mat(q1);
            dmat3 A2 = quat2mat(q2);
            dvec3 w1i = body2iner(w1b,A1);
            dvec3 w2i = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);
            
            double ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b);
            if (ord2_checkbox)
                ener += mut_pot_integrals_ord2(r, M1,J1,A1, M2,J2,A2);
            else if (ord3_checkbox)
                ener += mut_pot_integrals_ord3(r, M1,J1,A1, M2,J2,A2);
            else if (ord4_checkbox)
                ener += mut_pot_integrals_ord4(r, M1,J1,A1, M2,J2,A2);
            else
                ener += mut_pot_masc(r, M1,masc1,A1, M2,masc2,A2);

            dvec3 mom = (M1*M2/(M1+M2))*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b));
            
            solution.t.push_back(t);
            solution.x.push_back(r[0]);
            solution.y.push_back(r[1]);
            solution.z.push_back(r[2]);

            solution.ener.push_back(ener);
            solution.mom.push_back(length(mom));
        }
        */

        //return solution;
        return true;
    }
};


#endif
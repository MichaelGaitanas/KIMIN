#include<iostream>
#include<fstream>
#include<cmath>
#include<ctime>
#include<cstdlib>
#include<filesystem>
#include<boost/numeric/odeint.hpp>

#include"../../source/constant.h"
#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/conversion.h"
#include"../../source/rigidbody.h"
#include"../../source/ellipsoid.h"
#include"../../source/gravity.h"

double M1,M2,m; //m = M1*M2/(M1+M2)
dtens J1,J2; //Inertial integral tensors.
dmat3 I1,I2; //Moments of inertia.

//Build the rhs of the odes.
void odes(const boost::array<double, 20> &state, boost::array<double, 20> &dstate, double /*t*/)
{
    //Extract the state into variables for readability.
    dvec3 r  =  { state[0],  state[1],  state[2] };
    dvec3 v  =  { state[3],  state[4],  state[5] };
    dvec4 q1 =  { state[6],  state[7],  state[8],  state[9] };
    dvec3 w1b = { state[10], state[11], state[12] };
    dvec4 q2 =  { state[13], state[14], state[15], state[16] };
    dvec3 w2b = { state[17], state[18], state[19] };

    //Normalize the quaternions and convert to rotation matrices.
    q1 = quat2unit(q1);
    q2 = quat2unit(q2);
    dmat3 A1 = quat2mat(q1);
    dmat3 A2 = quat2mat(q2);

    dvec3 force = mut_force_integrals_ord2(r, M1,J1,A1, M2,J2,A2);

    dvec3 tau1i = mut_torque_integrals_ord2(r, J1,A1, M2);
    dvec3 tau2i = -tau1i - cross(r,force);

    dvec3 tau1b = iner2body(tau1i,A1);
    dvec3 tau2b = iner2body(tau2i,A2);

    dvec4 dq1 = quat_rhs(q1,w1b);
    dvec3 dw1b = euler_rhs(w1b,I1,tau1b);

    dvec4 dq2 = quat_rhs(q2,w2b);
    dvec3 dw2b = euler_rhs(w2b,I2,tau2b);

    //Relative position rhs (x,y,z).
    dstate[0] = v[0];
    dstate[1] = v[1];
    dstate[2] = v[2];

    //Relative velocity rhs (vx,vy,vz).
    dstate[3] = force[0]/m;
    dstate[4] = force[1]/m;
    dstate[5] = force[2]/m;

    //Quaternion rhs of rigid body 1 (q10,q11,q12,q13).
    dstate[6] = dq1[0];
    dstate[7] = dq1[1];
    dstate[8] = dq1[2];
    dstate[9] = dq1[3];

    //Euler rhs of rigid body 1 (w11,w12,w13).
    dstate[10] = dw1b[0];
    dstate[11] = dw1b[1];
    dstate[12] = dw1b[2];

    //Quaternion odes of rigid body 2 (q20,q21,q22,q23).
    dstate[13] = dq2[0];
    dstate[14] = dq2[1];
    dstate[15] = dq2[2];
    dstate[16] = dq2[3];

    //Euler rhs of rigid body 2 (w21,w22,w23).
    dstate[17] = dw2b[0];
    dstate[18] = dw2b[1];
    dstate[19] = dw2b[2];
}

int main()
{
    M1 = 5.32e11; //[kg]
    M2 = 4.94e9; //[kg]
    dvec3 semiaxes1 = {0.418765, 0.416194, 0.39309}; //[km]
    dvec3 semiaxes2 = {0.104, 0.080, 0.066}; //[km]

    m = M1*M2/(M1 + M2);
    J1 = ell_integrals(M1, semiaxes1, 2);
    J2 = ell_integrals(M2, semiaxes2, 2);
    I1 = ell_inertia(M1, semiaxes1);
    I2 = ell_inertia(M2, semiaxes2);

    //Collision parameters.
    double brillouin_radius1 = ell_brillouin(semiaxes1);
    double brillouin_radius2 = ell_brillouin(semiaxes2);

    //Time parameters.
    double t, t0 = 0.0; //[sec]
    double tmax = 30*86400.0; //[sec]
    double dt_guess = 1.0; //[sec]

    dvec3 r   = {1.19, 0.0, 0.0}; //[km]
    dvec3 v   = {0.0, 0.00017, 0.0}; //[km/sec]
    dvec4 q1  = {1.0, 0.0, 0.0, 0.0}; //[ ]
    dvec3 w1i = {0.0, 0.0, 0.00077}; //[rad/sec]
    dvec4 q2  = {1.0, 0.0, 0.0, 0.0}; // [ ]
    dvec3 w2i = {0.0, 0.0, 0.00014}; //[rad/sec]

    q1 = quat2unit(q1);
    q2 = quat2unit(q2);

    dvec3 w1b = iner2body(w1i, quat2mat(q1));
    dvec3 w2b = iner2body(w2i, quat2mat(q2));

    bool collision = false;

    boost::array<double, 20> state = {   r[0],    r[1],    r[2],
                                         v[0],    v[1],    v[2],
                                        q1[0],   q1[1],   q1[2], q1[3],
                                       w1b[0],  w1b[1],  w1b[2],
                                        q2[0],   q2[1],   q2[2], q2[3],
                                       w2b[0],  w2b[1],  w2b[2] };
    auto method = boost::numeric::odeint::make_controlled(1e-15, 1e-15, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>>());
    
    dmat orbit; //Output matrix, containing the state in time.

    t = t0;
    while (t <= tmax)
    {
        //Append the current state in the solution matrix.
        orbit.push_back({t,
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

        //Check for sphere-sphere collision (event detection).
        if (sphere_sphere_collision(length(dvec3{state[0],state[1],state[2]}), brillouin_radius1, brillouin_radius2))
        {
            collision = true;
            break;
        }
        //Update state.
        method.try_step(odes, state, t, dt_guess);
    }
    
    //Write 'orbit' data into files.
    std::filesystem::create_directory("io");
    FILE *file_t    = fopen("io/time.txt","w");
    FILE *file_pos  = fopen("io/rel_pos.txt","w");
    FILE *file_vel  = fopen("io/rel_vel.txt","w");
    FILE *file_q1   = fopen("io/quaternion1.txt","w");
    FILE *file_w1b  = fopen("io/ang_vel_w1b.txt","w");
    FILE *file_q2   = fopen("io/quaternion2.txt","w");
    FILE *file_w2b  = fopen("io/ang_vel_w2b.txt","w");
    FILE *file_rpy1 = fopen("io/euler_rpy1.txt","w");
    FILE *file_w1i  = fopen("io/ang_vel_w1i.txt","w");
    FILE *file_rpy2 = fopen("io/euler_rpy2.txt","w");
    FILE *file_w2i  = fopen("io/ang_vel_w2i.txt","w");
    FILE *file_EL   = fopen("io/ener_mom_rel_error.txt","w");
    for (size_t i = 0; i < orbit.size(); ++i)
    {
        dvec3 r = {orbit[i][1], orbit[i][2], orbit[i][3]};
        dvec3 v = {orbit[i][4], orbit[i][5], orbit[i][6]};
        dvec4 q1 = {orbit[i][7], orbit[i][8], orbit[i][9], orbit[i][10]};
        dvec3 w1b = {orbit[i][11], orbit[i][12], orbit[i][13]};
        dvec4 q2 = {orbit[i][14], orbit[i][15], orbit[i][16], orbit[i][17]};
        dvec3 w2b = {orbit[i][18], orbit[i][19], orbit[i][20]};
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);
        dvec3 w1i = body2iner(w1b,A1);
        dvec3 w2i = body2iner(w2b,A2);
        dvec3 rpy1 = quat2ang(q1)*180.0/pi;
        dvec3 rpy2 = quat2ang(q2)*180.0/pi;

        double energy = 0.5*m*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_integrals_ord2(r, M1,J1,A1, M2,J2,A2);
        double momentum = length(m*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b)));

        double energy_at_t0, momentum_at_t0;
        double ener_rel_err, mom_rel_err;
        if (i == 0)
        {
            energy_at_t0 = energy;
            momentum_at_t0 = momentum;
            ener_rel_err = 0.0;
            mom_rel_err  = 0.0;
        }
        else
        {
            if (fabs(energy_at_t0) > 1e-16)
                ener_rel_err = fabs((energy - energy_at_t0)/energy_at_t0);
            else //Fallback to absolute error to avoid division by zero.
                ener_rel_err = fabs(energy - energy_at_t0);
        
            if (fabs(momentum_at_t0) > 1e-16)
                mom_rel_err = fabs((momentum - momentum_at_t0)/momentum_at_t0);
            else //The same...
                mom_rel_err = fabs(momentum - momentum_at_t0);
        }

        fprintf(file_t,"%.16lf\n", orbit[i][0]/86400.0);
        fprintf(file_pos,"%.16lf %.16lf %.16lf %.16lf\n",r[0],r[1],r[2], length(r));
        fprintf(file_vel,"%.16lf %.16lf %.16lf %.16lf\n",v[0],v[1],v[2], length(v));
        fprintf(file_q1,"%.16lf %.16lf %.16lf %.16lf\n",q1[0],q1[1],q1[2],q1[3]);
        fprintf(file_w1b,"%.16lf %.16lf %.16lf\n",w1b[0],w1b[1],w1b[2]);
        fprintf(file_q2,"%.16lf %.16lf %.16lf %.16lf\n",q2[0],q2[1],q2[2],q2[3]);
        fprintf(file_w2b,"%.16lf %.16lf %.16lf\n",w2b[0],w2b[1],w2b[2]);

        fprintf(file_rpy1,"%.16lf %.16lf %.16lf\n",rpy1[0],rpy1[1],rpy1[2]);
        fprintf(file_w1i,"%.16lf %.16lf %.16lf\n",w1i[0],w1i[1],w1i[2]);
        fprintf(file_rpy2,"%.16lf %.16lf %.16lf\n",rpy2[0],rpy2[1],rpy2[2]);
        fprintf(file_w2i,"%.16lf %.16lf %.16lf\n",w2i[0],w2i[1],w2i[2]); 
        fprintf(file_EL,"%.16lf %.16lf\n", ener_rel_err, mom_rel_err);
    }
    fclose(file_t);
    fclose(file_pos);
    fclose(file_q1);
    fclose(file_rpy1);
    fclose(file_w1b);
    fclose(file_w1i);
    fclose(file_q2);
    fclose(file_rpy2);
    fclose(file_w2b);
    fclose(file_w2i);
    fclose(file_EL);

    FILE *fpsteps = fopen("io/steps.txt","w");
    fprintf(fpsteps,"%llu\n", orbit.size());
    fclose(fpsteps);

    FILE *fpcollision = fopen("io/collision.txt","w");
    fprintf(fpcollision, "Collision detected : %s", collision ? "Yes" : "No");
    fclose(fpcollision);

    return 0;
}
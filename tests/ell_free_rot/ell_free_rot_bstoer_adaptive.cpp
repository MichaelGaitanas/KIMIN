#include<iostream>
#include<fstream>
#include<cmath>
#include<ctime>
#include<cstdlib>
#include<filesystem>
#include<limits>
#include<boost/numeric/odeint.hpp>

#include"../../source/constant.h"
#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/conversion.h"
#include"../../source/rigidbody.h"
#include"../../source/ellipsoid.h"

dmat3 inertia; //Ellispoid inertia matrix. This must be global due to the call in boost's odes().

void odes(const boost::array<double, 7> &state, boost::array<double, 7> &dstate, double t)
{
    //Extract the state into variables for readability.
    dvec4 q =  { state[0],  state[1],  state[2], state[3] };
    dvec3 wb = { state[4], state[5], state[6] };

    //Normalize the quaternion and convert to rotation matrix.
    q = quat2unit(q);
    dmat3 A = quat2mat(q);

    //Zero torque applied to the ellipsoid, hence free rotation.
    //Note : Setting the torque to nonzero, would still work, but it would angular - accelerate/decelerate the ellipsoid (no energy/momentum conservation).
    dvec3 taui = dvec3{0.0,0.0,0.0};

    dvec4 dq = quat_rhs(q,wb);
    dvec3 dwb = euler_rhs(wb, inertia, iner2body(taui, A)); 

    dstate[0] = dq[0];
    dstate[1] = dq[1];
    dstate[2] = dq[2];
    dstate[3] = dq[3];

    dstate[4] = dwb[0];
    dstate[5] = dwb[1];
    dstate[6] = dwb[2];
}

int main()
{
    //Mass and semi axes.
    double M = 4.940814359692687e9; //[kg]
    dvec3 semiaxes = dvec3{0.104, 0.080, 0.066}; //[km]
    inertia = ell_inertia(M, semiaxes);

    //Time parameters.
    double t, t0 = 0.0; //[sec]
    double tmax = 86400.0; //[sec]
    double dt_guess = 1.0; //[sec]

    //Initial conditions (orientation and angular velocity).
    dvec4 q  = ang2quat(dvec3{0.0,0.0,0.0}); //[ ]
    dvec3 wi = dvec3{0.0001, 0.004, 0.0}; //[rad/sec]

    q = quat2unit(q);
    dvec3 wb = iner2body(wi, quat2mat(q));

    boost::array<double, 7> state = { q[0],q[1],q[2],q[3],  wb[0],wb[1],wb[2] };

    double acc = 1.0e-18;
    const double machine_eps = std::numeric_limits<double>::epsilon();
    if (acc <= machine_eps)
        acc = 10.0*machine_eps;
    boost::numeric::odeint::bulirsch_stoer<boost::array<double, 7>> bstoer_adaptive(acc, acc);

    dmat orbit; //Output matrix, containing the state in time.
    
    t = t0;
    while (t <= tmax) //Propagation loop.
    {
        orbit.push_back({t, state[0], state[1], state[2], state[3], state[4], state[5], state[6]}); // {t, q0, q1, q2, q3, wbx, wby, wbz}
        bstoer_adaptive.try_step(odes, state, t, dt_guess);
    }
 
    //Write 'orbit' data into files.
    std::filesystem::create_directory("io");
    FILE *fpt = fopen("io/time.txt","w");
    FILE *fpq = fopen("io/quaternion.txt","w");
    FILE *fpwi = fopen("io/w_inertial.txt","w");
    FILE *fpwb = fopen("io/w_body.txt","w");
    FILE *fprpy = fopen("io/roll_pitch_yaw.txt","w");
    FILE *fpEL = fopen("io/energy_momentum.txt","w");
    for (size_t i = 0; i < orbit.size(); ++i)
    {
        dvec4 q = dvec4{orbit[i][1], orbit[i][2], orbit[i][3], orbit[i][4]};
        dvec3 wb = dvec3{orbit[i][5], orbit[i][6], orbit[i][7]};
        dmat3 A = quat2mat(q);
        dvec3 wi = body2iner(wb,A);
        dvec3 rpy = quat2ang(q);
        double energy = 0.5*dot( dot(wb,inertia), wb);
        dvec3 momentum = dot(A, dot(inertia,wb));

        fprintf(fpt,"%.16lf\n", orbit[i][0]); //t
        fprintf(fpq,"%.16lf %.16lf %.16lf %.16lf\n",q[0],q[1],q[2],q[3]);
        fprintf(fprpy,"%.16lf %.16lf %.16lf\n",rpy[0],rpy[1],rpy[2]);
        fprintf(fpwb,"%.16lf %.16lf %.16lf\n",wb[0],wb[1],wb[2]);
        fprintf(fpwi,"%.16lf %.16lf %.16lf\n",wi[0],wi[1],wi[2]);
        fprintf(fpEL,"%.16lf %.16lf %.16lf %.16lf\n", energy, momentum[0],momentum[1],momentum[2]);
    }
    fclose(fpt);
    fclose(fpq);
    fclose(fprpy);
    fclose(fpwb);
    fclose(fpwi);
    fclose(fpEL);

    FILE *fpsteps = fopen("io/integrator_steps.txt","w");
    fprintf(fpsteps,"%u\n", orbit.size());
    fclose(fpsteps);
    
    return 0;
}
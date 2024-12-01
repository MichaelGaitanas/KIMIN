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

typedef boost::array<double, 7> bvec7;
typedef boost::numeric::odeint::runge_kutta_fehlberg78<bvec7> rkf78;

dmat3 inertia;
double t0, tmax, print_step;

dmat sol;

void observe(const bvec7 &state, const double t)
{
    sol.push_back({t, state[0], state[1], state[2], state[3], state[4], state[5], state[6]});
    return;
}

void odes(const bvec7 &state, bvec7 &dstate, double t)
{
    dvec4 q =  { state[0],  state[1],  state[2], state[3] };
    dvec3 wb = { state[4], state[5], state[6] };

    q = quat2unit(q);
    dmat3 A = quat2mat(q);
    dvec3 taui = dvec3{0.0,0.0,0.0}; //zero external torque, hence free rotation

    dvec4 dq = quat_rhs(q,wb);
    dvec3 dwb = euler_rhs( wb,inertia, iner2body(taui,A) ); 

    dstate[0] = dq[0];
    dstate[1] = dq[1];
    dstate[2] = dq[2];
    dstate[3] = dq[3];

    dstate[4] = dwb[0];
    dstate[5] = dwb[1];
    dstate[6] = dwb[2];

    return;
}

int main()
{
    double M = 4.940814359692687e9; //[kg]
    dvec3 semiaxes = {0.104, 0.080, 0.066}; //[km]
    t0 = 0.0; //[sec]
    tmax = 86400.0; //[sec]
    print_step = 60.0; //[sec]
    dvec4 q  = ang2quat(dvec3{0.0,0.0,0.0}); //[ ]
    dvec3 wi = {0.0, 0.004, 0.001}; //[rad/sec]

    inertia = ell_inertia(M, semiaxes);

    q = quat2unit(q);
    dvec3 wb = iner2body(wi, quat2mat(q));

    //initial conditions
    bvec7 state = { q[0],q[1],q[2],q[3],  wb[0],wb[1],wb[2] };
    
    //solve the odes
    unsigned steps = integrate_adaptive(make_controlled(1e-15, 1e-15, rkf78()), odes, state, t0, tmax, print_step, observe);
    
    //write solution 'sol' into files
    bool madedir = std::filesystem::create_directory("io");
    FILE *fpt = fopen("io/time.txt","w");
    FILE *fpq = fopen("io/quaternion.txt","w");
    FILE *fpwi = fopen("io/w_inertial.txt","w");
    FILE *fpwb = fopen("io/w_body.txt","w");
    FILE *fprpy = fopen("io/roll_pitch_yaw.txt","w");
    FILE *fpmat = fopen("io/rotation_matrix.txt","w");
    FILE *fpEL = fopen("io/energy_momentum.txt","w");
    for (size_t i = 0; i < sol.size(); ++i)
    {
        double t = sol[i][0];
        dvec4 q = {sol[i][1], sol[i][2], sol[i][3], sol[i][4]};
        dvec3 wb = {sol[i][5], sol[i][6], sol[i][7]};
        dmat3 A = quat2mat(q);
        dvec3 wi = body2iner(wb,A);
        dvec3 rpy = quat2ang(q);
        double ener = 0.5*dot( dot(wb,inertia), wb);
        dvec3 mom = dot(A, dot(inertia,wb));

        fprintf(fpt,"%.16lf\n",t);
        fprintf(fpq,"%.16lf %.16lf %.16lf %.16lf\n",q[0],q[1],q[2],q[3]);
        fprintf(fprpy,"%.16lf %.16lf %.16lf\n",rpy[0],rpy[1],rpy[2]);
        fprintf(fpmat,"%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",A[0][0],A[0][1],A[0][2], A[1][0],A[1][1],A[1][2], A[2][0],A[2][1],A[2][2]);
        fprintf(fpwb,"%.16lf %.16lf %.16lf\n",wb[0],wb[1],wb[2]);
        fprintf(fpwi,"%.16lf %.16lf %.16lf\n",wi[0],wi[1],wi[2]);
        fprintf(fpEL,"%.16lf %.16lf %.16lf %.16lf\n", ener, mom[0],mom[1],mom[2]);
    }
    fclose(fpt);
    fclose(fpq);
    fclose(fprpy);
    fclose(fpmat);
    fclose(fpwb);
    fclose(fpwi);
    fclose(fpEL);

    FILE *fpsteps = fopen("io/steps.txt","w");
    fprintf(fpsteps,"%u\n",steps);
    fclose(fpsteps);
    
    return 0;
}
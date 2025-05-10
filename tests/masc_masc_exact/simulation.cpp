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
#include"../../source/gravity.h"
#include"../../source/mascons.h"
#include"../../source/polyhedron.h"

double M1,M2,m; //m = M1*M2/(M1+M2)
mascons masc1, masc2;
dmat3 I1,I2; //Moments of inertia.

//Build the rhs of the odes.
void odes(const boost::array<double, 20> &state, boost::array<double, 20> &dstate, double t)
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

    dvec3 force = mut_force_masc(r, M1,masc1.get_points(),A1, M2,masc2.get_points(),A2);

    dvec3 tau1i = mut_torque_masc(r, M1,masc1.get_points(),A1, M2,masc2.get_points(),A2);
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
    M1 = 5.320591856403073e11; //[kg]
    M2 = 4.940814359692687e9; //[kg]
    m = M1*M2/(M1 + M2);

    std::filesystem::create_directory("io");

    polyhedron poly1,poly2;
    
    printf("Generating mascons 1... ");
    poly1.load_obj_file("../../obj/polyhedra/didymain2019_R04km.obj");
    masc1.generate_from_polyhedron(poly1, uvec3{20,20,20});
    masc1.export_obj_file("io/masc1.obj");
    masc1.set_com_zero();
    masc1.set_inertia_diagonal();
    masc1.export_obj_file("io/masc1_fixed.obj");
    I1 = masc1.get_inertia(M1);
    double brillouin_radius1 = masc1.get_farthest_point_distance();
    printf("Done.\n");

    printf("Generating mascons 2... ");
    poly2.load_obj_file("../../obj/polyhedra/dimorphos_ellipsoid_R01km.obj");
    masc2.generate_from_polyhedron(poly2, uvec3{20,20,20});
    masc2.export_obj_file("io/masc2.obj");
    masc2.set_com_zero();
    masc2.set_inertia_diagonal();
    masc2.export_obj_file("io/masc2_fixed.obj");
    I2 = masc2.get_inertia(M2);    
    double brillouin_radius2 = masc2.get_farthest_point_distance();
    printf("Done.\n");

    //Time parameters.
    double t, t0 = 0.0; //[sec]
    double tmax = 1*86400.0; //[sec]
    double dt_guess = 1.0; //[sec]

    dvec3 r   = {1.19, 0.0, 1.0}; //[km]
    dvec3 v   = {0.0, 0.00017421523858789, 0.0}; //[km/sec]
    dvec4 q1  = {1.0, 0.0, 0.0, 0.0}; //[ ]
    dvec3 w1i = {0.0, 0.0, 0.000772269580528465}; //[rad/sec]
    dvec4 q2  = {1.0, 0.0, 0.0, 0.0}; // [ ]
    dvec3 w2i = {0.0, 0.0, 0.000146399360157891}; //[rad/sec]

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
    auto method = boost::numeric::odeint::make_controlled(1.0e-15, 1.0e-15, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 20>>());
    
    dmat orbit; //Output matrix, containing the state in time.

    t = t0;
    while (t <= tmax)
    {

        double progress = 100.0*(t-t0)/(tmax-t0);
        printf("\rProgress: %.1f%%", progress);
        fflush(stdout);
    
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
    FILE *fpt = fopen("io/time.txt","w");
    FILE *fprv = fopen("io/pos_vel.txt","w");
    FILE *fpq1 = fopen("io/q1.txt","w");
    FILE *fpw1i = fopen("io/w1_inertial.txt","w");
    FILE *fpw1b = fopen("io/w1_body.txt","w");
    FILE *fprpy1 = fopen("io/roll_pitch_yaw_1.txt","w");
    FILE *fpq2 = fopen("io/q2.txt","w");
    FILE *fpw2i = fopen("io/w2_inertial.txt","w");
    FILE *fpw2b = fopen("io/w2_body.txt","w");
    FILE *fprpy2 = fopen("io/roll_pitch_yaw_2.txt","w");
    FILE *fpEL = fopen("io/energy_momentum.txt","w");
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
        dvec3 rpy1 = quat2ang(q1);
        dvec3 rpy2 = quat2ang(q2);

        double energy = 0.5*m*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_masc(r, M1,masc1.get_points(),A1, M2,masc2.get_points(),A2);
        dvec3 momentum = m*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b));

        fprintf(fpt,"%.16lf\n", orbit[i][0]);
        fprintf(fprv,"%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",r[0],r[1],r[2], v[0],v[1],v[2]);
        fprintf(fpq1,"%.16lf %.16lf %.16lf %.16lf\n",q1[0],q1[1],q1[2],q1[3]);
        fprintf(fprpy1,"%.16lf %.16lf %.16lf\n",rpy1[0],rpy1[1],rpy1[2]);
        fprintf(fpw1b,"%.16lf %.16lf %.16lf\n",w1b[0],w1b[1],w1b[2]);
        fprintf(fpw1i,"%.16lf %.16lf %.16lf\n",w1i[0],w1i[1],w1i[2]);
        fprintf(fpq2,"%.16lf %.16lf %.16lf %.16lf\n",q2[0],q2[1],q2[2],q2[3]);
        fprintf(fprpy2,"%.16lf %.16lf %.16lf\n",rpy2[0],rpy2[1],rpy2[2]);
        fprintf(fpw2b,"%.16lf %.16lf %.16lf\n",w2b[0],w2b[1],w2b[2]);
        fprintf(fpw2i,"%.16lf %.16lf %.16lf\n",w2i[0],w2i[1],w2i[2]); 
        fprintf(fpEL,"%.16lf %.16lf %.16lf %.16lf\n", energy, momentum[0],momentum[1],momentum[2]);
    }
    fclose(fpt);
    fclose(fprv);
    fclose(fpq1);
    fclose(fprpy1);
    fclose(fpw1b);
    fclose(fpw1i);
    fclose(fpq2);
    fclose(fprpy2);
    fclose(fpw2b);
    fclose(fpw2i);
    fclose(fpEL);

    FILE *fpsteps = fopen("io/steps.txt","w");
    fprintf(fpsteps,"%u\n", orbit.size());
    fclose(fpsteps);

    FILE *fpcollision = fopen("io/collision.txt","w");
    fprintf(fpcollision, "Collision detected : %s", collision ? "Yes" : "No");
    fclose(fpcollision);

    printf("\nDone.\n");

    return 0;
}
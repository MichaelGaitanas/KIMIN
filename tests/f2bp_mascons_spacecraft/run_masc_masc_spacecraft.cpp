#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<filesystem>
#include<boost/numeric/odeint.hpp>

#include"../../source/constant.h"
#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/file.h"
#include"../../source/conversion.h"
#include"../../source/rigidbody.h"
#include"../../source/gravity.h"
#include"../../source/mascons.h"
#include"../../source/polyhedron.h"

//Simulation parameters that enter in Boost's odes() function.
struct params
{
    double M1,M2,m; //m = M1*M2/(M1+M2)
    mascons masc1, masc2; //Mascons point clouds.
    dmat3 I1,I2; //Moments of inertia.
};
params pars;

//Simulation inputs (same order as in inputs_common.txt).
struct inputs
{
    double M1,M2; //Will be assigned to par.M1, par.M2
    char filename1[256], filename2[256];
    uvec3 reso1, reso2;
    double t0, tmax, atol, rtol;
    double x,y,z;
    double vx,vy,vz;
    double q10,q11,q12,q13;
    double q20,q21,q22,q23;
    double w1x,w1y,w1z;
    double w2x,w2y,w2z;
    double xs,ys,zs;
    double vxs,vys,vzs;

    void read()
    {
        FILE *fpins = fopen("inputs_common.txt","r");
        if (!fpins)
        {
            fprintf(stderr, "Error : File 'inputs_common.txt' was not found. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        //Read one value at a time, after finding the ':=' operator (and with the same order as in the inputs_common.txt).
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &M1);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &M2);
        if (find_assignment_operator(fpins)) fscanf(fpins, " \"%[^\"]\"", filename1);
        if (find_assignment_operator(fpins)) fscanf(fpins, " \"%[^\"]\"", filename2);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%u", &reso1[0]);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%u", &reso1[1]);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%u", &reso1[2]);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%u", &reso2[0]);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%u", &reso2[1]);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%u", &reso2[2]);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &t0);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &tmax);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &atol);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &rtol);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &x);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &y);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &z);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vx);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vy);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vz);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q10);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q11);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q12);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q13);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q20);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q21);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q22);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &q23);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w1x);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w1y);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w1z);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w2x);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w2y);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w2z);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &xs);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &ys);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &zs);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vxs);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vys);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vzs);
        fclose(fpins);
    }
};
inputs ins;

//Build the rhs of the odes.
void odes(const boost::array<double, 26> &state, boost::array<double, 26> &dstate, double /*t*/)
{
    //Binary's state.
    dvec3 r  =  { state[0],  state[1],  state[2] };
    dvec3 v  =  { state[3],  state[4],  state[5] };
    dvec4 q1 =  { state[6],  state[7],  state[8],  state[9] };
    dvec3 w1b = { state[10], state[11], state[12] };
    dvec4 q2 =  { state[13], state[14], state[15], state[16] };
    dvec3 w2b = { state[17], state[18], state[19] };

    //Spacecraft's state.
    dvec3 rs = { state[20], state[21], state[22] };
    dvec3 vs = { state[23], state[24], state[25] };

    //Normalize the quaternions and convert to rotation matrices.
    q1 = quat2unit(q1);
    q2 = quat2unit(q2);
    dmat3 A1 = quat2mat(q1);
    dmat3 A2 = quat2mat(q2);

    dvec6 force_and_tau1i = mut_force_tau1i_masc(r, pars.M1,pars.masc1.get_points(),A1,
                                                    pars.M2,pars.masc2.get_points(),A2);

    dvec3 force = {force_and_tau1i[0], force_and_tau1i[1], force_and_tau1i[2]};

    dvec3 tau1i = {force_and_tau1i[3], force_and_tau1i[4], force_and_tau1i[5]};
    dvec3 tau2i = -tau1i - cross(r,force);

    dvec3 tau1b = iner2body(tau1i,A1);
    dvec3 tau2b = iner2body(tau2i,A2);

    dvec4 dq1 = quat_rhs(q1,w1b);
    dvec3 dw1b = euler_rhs(w1b,pars.I1,tau1b);

    dvec4 dq2 = quat_rhs(q2,w2b);
    dvec3 dw2b = euler_rhs(w2b,pars.I2,tau2b);

    //Spacecraft's acceleration due to the presence of the 2 rigid bodies.
    dvec3 force_spacecraft = force_masc(rs + pars.M2*r/(pars.M1+pars.M2), pars.M1,pars.masc1.get_points(),A1) +
                             force_masc(rs - pars.M1*r/(pars.M1+pars.M2), pars.M2,pars.masc2.get_points(),A2);

    //Relative position rhs (x,y,z).
    dstate[0] = v[0];
    dstate[1] = v[1];
    dstate[2] = v[2];

    //Relative velocity rhs (vx,vy,vz).
    dstate[3] = force[0]/pars.m;
    dstate[4] = force[1]/pars.m;
    dstate[5] = force[2]/pars.m;

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

    //Spacecraft's position rhs (xs,ys,zs).
    dstate[20] = vs[0];
    dstate[21] = vs[1];
    dstate[22] = vs[2];

    //Spacecraft's velocity rhs (vxs,vys,vzs).
    dstate[23] = force_spacecraft[0];
    dstate[24] = force_spacecraft[1];
    dstate[25] = force_spacecraft[2];
}

int main()
{
    std::filesystem::create_directory("io");

    ins.read();
    pars.M1 = ins.M1;
    pars.M2 = ins.M2;
    pars.m = pars.M1*pars.M2/(pars.M1 + pars.M2);

    polyhedron poly1,poly2;
    
    printf("Generating mascons 1... ");
    poly1.load_obj_file(ins.filename1);
    pars.masc1.generate_from_polyhedron(poly1, uvec3{ins.reso1[0],ins.reso1[1],ins.reso1[2]});
    pars.masc1.export_obj_file("io/masc1.obj");
    pars.masc1.set_com_zero();
    pars.masc1.set_inertia_diagonal();
    pars.masc1.export_obj_file("io/masc1_fixed.obj");
    pars.I1 = pars.masc1.get_inertia(pars.M1);
    double brillouin_radius1 = pars.masc1.get_farthest_point_distance();
    printf("Done.\n");

    printf("Generating mascons 2... ");
    poly2.load_obj_file(ins.filename2);
    pars.masc2.generate_from_polyhedron(poly2, uvec3{ins.reso2[0],ins.reso2[1],ins.reso2[2]});
    pars.masc2.export_obj_file("io/masc2.obj");
    pars.masc2.set_com_zero();
    pars.masc2.set_inertia_diagonal();
    pars.masc2.export_obj_file("io/masc2_fixed.obj");
    pars.I2 = pars.masc2.get_inertia(pars.M2);    
    double brillouin_radius2 = pars.masc2.get_farthest_point_distance();
    printf("Done.\n");

    //Time parameters.
    double t, t0 = ins.t0;  //[sec]
    double tmax = ins.tmax; //[sec]
    double dt_guess = 1.0;  //[sec]

    dvec3 r   = {ins.x,   ins.y,   ins.z};             //[km]
    dvec3 v   = {ins.vx,  ins.vy,  ins.vz};            //[km/sec]
    dvec4 q1  = {ins.q10, ins.q11, ins.q12, ins.q13};  //[ ]
    dvec3 w1i = {ins.w1x, ins.w1y, ins.w1z};           //[rad/sec]
    dvec4 q2  = {ins.q20, ins.q21, ins.q22, ins.q23};  //[ ]
    dvec3 w2i = {ins.w2x, ins.w2y, ins.w2z};           //[rad/sec]

    dvec3 rs  = {ins.xs,   ins.ys,   ins.zs};          //[km]
    dvec3 vs  = {ins.vxs,  ins.vys,  ins.vzs};         //[km/sec]

    q1 = quat2unit(q1);
    q2 = quat2unit(q2);

    dvec3 w1b = iner2body(w1i, quat2mat(q1));
    dvec3 w2b = iner2body(w2i, quat2mat(q2));

    bool collision = false;

    boost::array<double, 26> state = {   r[0],    r[1],    r[2],
                                         v[0],    v[1],    v[2],
                                        q1[0],   q1[1],   q1[2], q1[3],
                                       w1b[0],  w1b[1],  w1b[2],
                                        q2[0],   q2[1],   q2[2], q2[3],
                                       w2b[0],  w2b[1],  w2b[2],
                                        rs[0],   rs[1],   rs[2],
                                        vs[0],   vs[1],   vs[2] };
    auto method = boost::numeric::odeint::make_controlled(ins.atol, ins.rtol, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 26>>());
    
    dmat orbit; //Output matrix, containing the state in time.

    t = t0;
    while (t <= tmax)
    {
        double progress = 100.0*(t-t0)/(tmax-t0);
        printf("\rProgress: %.1f%%", progress);
        fflush(stdout);
    
        //Append the current state in the solution matrix.
        orbit.push_back({t, state[0],  state[1],  state[2],
                            state[3],  state[4],  state[5],
                            state[6],  state[7],  state[8],  state[9],
                            state[10], state[11], state[12],
                            state[13], state[14], state[15], state[16],
                            state[17], state[18], state[19],
                            state[20], state[21], state[22],
                            state[23], state[24], state[25]});

        //Check for sphere-sphere collision (event detection).
        dvec3 r = dvec3{state[0],state[1],state[2]};
        dvec3 rs = dvec3{state[20],state[21],state[22]};
        if (sphere_sphere_collision(length(r), brillouin_radius1, brillouin_radius2) ||
            length(rs + pars.M2*r/(pars.M1+pars.M2)) <= brillouin_radius1 ||
            length(rs - pars.M1*r/(pars.M1+pars.M2)) <= brillouin_radius2 )
        {
            collision = true;
            break;
        }
        //Update state.
        method.try_step(odes, state, t, dt_guess);
    }
    
    //Write 'orbit' data into files.
    FILE *file_t      = fopen("io/mmes_time.txt","w");
    FILE *file_pos    = fopen("io/mmes_rel_pos.txt","w");
    FILE *file_vel    = fopen("io/mmes_rel_vel.txt","w");
    FILE *file_q1     = fopen("io/mmes_quaternion1.txt","w");
    FILE *file_w1b    = fopen("io/mmes_ang_vel_w1b.txt","w");
    FILE *file_q2     = fopen("io/mmes_quaternion2.txt","w");
    FILE *file_w2b    = fopen("io/mmes_ang_vel_w2b.txt","w");
    FILE *file_rpy1   = fopen("io/mmes_euler_rpy1.txt","w");
    FILE *file_w1i    = fopen("io/mmes_ang_vel_w1i.txt","w");
    FILE *file_rpy2   = fopen("io/mmes_euler_rpy2.txt","w");
    FILE *file_w2i    = fopen("io/mmes_ang_vel_w2i.txt","w");
    FILE *file_EL_err = fopen("io/mmes_ener_mom_rel_error.txt","w");
    FILE *file_pos_spacecraft  = fopen("io/mmes_pos_spacecraft.txt","w");
    FILE *file_vel_spacecraft  = fopen("io/mmes_vel_spacecraft.txt","w");
    FILE *file_EL_spacecraft  = fopen("io/mmes_ener_mom_spacecraft.txt","w");
    double energy_at_t0, energy_rel_err;
    double momentum_at_t0, momentum_rel_err;
    for (size_t i = 0; i < orbit.size(); ++i)
    {
        dvec3 r =   {orbit[i][1], orbit[i][2], orbit[i][3]};
        dvec3 v =   {orbit[i][4], orbit[i][5], orbit[i][6]};
        dvec4 q1 =  {orbit[i][7], orbit[i][8], orbit[i][9], orbit[i][10]};
        dvec3 w1b = {orbit[i][11], orbit[i][12], orbit[i][13]};
        dvec4 q2 =  {orbit[i][14], orbit[i][15], orbit[i][16], orbit[i][17]};
        dvec3 w2b = {orbit[i][18], orbit[i][19], orbit[i][20]};
        dmat3 A1 = quat2mat(q1);
        dmat3 A2 = quat2mat(q2);
        dvec3 w1i = body2iner(w1b,A1);
        dvec3 w2i = body2iner(w2b,A2);
        dvec3 rpy1 = quat2ang(q1)*180.0/pi;
        dvec3 rpy2 = quat2ang(q2)*180.0/pi;
        dvec3 rs = {orbit[i][21], orbit[i][22], orbit[i][23]};
        dvec3 vs = {orbit[i][24], orbit[i][25], orbit[i][26]};
        double energy = 0.5*pars.m*dot(v,v) +
                        0.5*dot( dot(w1b,pars.I1), w1b) +
                        0.5*dot( dot(w2b,pars.I2), w2b) +
                        mut_pot_masc(r, pars.M1,pars.masc1.get_points(),A1, pars.M2,pars.masc2.get_points(),A2);
        double momentum = length(pars.m*cross(r,v) + dot(A1, dot(pars.I1,w1b)) + dot(A2, dot(pars.I2,w2b)));

        double energy_spacecraft = 0.5*dot(vs,vs) +
                                   pot_masc(rs + pars.M2*r/(pars.M1+pars.M2), pars.M1,pars.masc1.get_points(),A1) +
                                   pot_masc(rs - pars.M1*r/(pars.M1+pars.M2), pars.M2,pars.masc2.get_points(),A2);
        double momentum_spacecraft = length(cross(rs,vs));

        if (i == 0)
        {
            energy_at_t0 = energy;
            momentum_at_t0 = momentum;
            energy_rel_err = 0.0;
            momentum_rel_err = 0.0;
        }
        else
        {
            //Energy :
            if (fabs(energy_at_t0) > 1e-16)
                energy_rel_err = fabs((energy - energy_at_t0)/energy_at_t0);
            else //Fallback to absolute error to avoid division by zero.
                energy_rel_err = fabs(energy - energy_at_t0);
        
            //Momentum :
            if (fabs(momentum_at_t0) > 1e-16)
                momentum_rel_err = fabs((momentum - momentum_at_t0)/momentum_at_t0);
            else //The same...
                momentum_rel_err = fabs(momentum - momentum_at_t0);
        }

        fprintf(file_t,"%.16lf\n", orbit[i][0]/86400.0); //Export t in [days]
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
        fprintf(file_EL_err,"%.16lf %.16lf\n", energy_rel_err, momentum_rel_err);

        fprintf(file_pos_spacecraft,"%.16lf %.16lf %.16lf %.16lf\n",rs[0],rs[1],rs[2], length(rs));
        fprintf(file_vel_spacecraft,"%.16lf %.16lf %.16lf %.16lf\n",vs[0],vs[1],vs[2], length(vs));
        fprintf(file_EL_spacecraft,"%.16lf %.16lf\n", energy_spacecraft, momentum_spacecraft);
    }
    fclose(file_t);
    fclose(file_pos);
    fclose(file_vel);
    fclose(file_q1);
    fclose(file_w1b);
    fclose(file_q2);
    fclose(file_w2b);

    fclose(file_rpy1);
    fclose(file_w1i);
    fclose(file_rpy2);
    fclose(file_w2i);
    fclose(file_EL_err);
    fclose(file_pos_spacecraft);
    fclose(file_vel_spacecraft);
    fclose(file_EL_spacecraft);

    FILE *file_collision = fopen("io/mmes_collision.txt","w");
    fprintf(file_collision, "Collision detected : %s", collision ? "Yes" : "No");
    fclose(file_collision);

    printf("\rProgress: 100%%  ");
    fflush(stdout);
    printf("\nDone.\n");

    return 0;
}
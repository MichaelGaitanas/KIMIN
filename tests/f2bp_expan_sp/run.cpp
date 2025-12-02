#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<filesystem>
#include<boost/numeric/odeint.hpp>

#include"../../source/constants.h"
#include"../../source/typedef.h"
#include"../../source/linalg.h"
#include"../../source/file.h"
#include"../../source/conversion.h"
#include"../../source/rigidbody.h"
#include"../../source/gravity.h"
#include"../../source/polyhedron.h"

//Simulation parameters that are not read directly from the input file. They are computed on the fly as a function of the inputs.
struct params
{
    double m; //m = M1*M2/(M1+M2)
    dmat3 I1,I2; //Moments of inertia tensors.
    dtens J1,J2; //Inertial integral tensors.
    double brillouin1, brillouin2; //Brillouin radii of the 2 asteroids.
    dvec3 rsun; //Sun's position vector
};
params pars;

//Simulation file inputs.
struct inputs
{
    char filename1[256], filename2[256];
    int ord; //Potential expansion order.
    double M1,M2; //Masses of the 2 asteroids.
    double t0, tmax, atol, rtol; //Integration time parameters.
    double a,e,i,Om,w,M; //Initial Keplerian elements of the binary (in the inertial C.O.M. frame).
    double q10,q11,q12,q13; //Initial quaternion of asteroid 1.
    double q20,q21,q22,q23; //Initial quaternion of asteroid 2.
    double w1x,w1y,w1z; //Initial angular velocity of asteroid 1 (in the inertial C.O.M. frame).
    double w2x,w2y,w2z; //Initial angular velocity of asteroid 2 (in the inertial C.O.M. frame).
    double xsp,ysp,zsp, vxsp,vysp,vzsp; //Initial position/velocity of the spacecraft (in the inertial C.O.M. frame).
    double rhosp, Asp, msp; //Spacecraft's reflectivity coefficient, effective area and mass (for the SRP).
    double dist_sun, lon_sun, lat_sun; //Sun's distance (in AU), longitude and latitude (for the SRP).
    int shadow; //0 means we do not account for shadow in the SRP, 1 means we account for shadow in the SRP. 

    void read()
    {
        FILE *fpins = fopen("inputs_run.txt","r");
        if (!fpins)
        {
            fprintf(stderr, "Error : File 'inputs_run.txt' was not found. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        //Read one value at a time, after finding the ':=' operator (and with the same order as in the inputs_common.txt).
        if (find_assignment_operator(fpins)) fscanf(fpins, " \"%[^\"]\"", filename1);
        if (find_assignment_operator(fpins)) fscanf(fpins, " \"%[^\"]\"", filename2);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%d" , &ord);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &M1);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &M2);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &t0);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &tmax);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &atol);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &rtol);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &a);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &e);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &i);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &Om);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &M);
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
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &xsp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &ysp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &zsp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vxsp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vysp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &vzsp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &rhosp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &Asp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &msp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &dist_sun);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &lon_sun);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &lat_sun);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%d" , &shadow);
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

    //Normalize the quaternions and convert to rotation matrices.
    q1 = quat2unit(q1);
    q2 = quat2unit(q2);
    dmat3 A1 = quat2mat(q1);
    dmat3 A2 = quat2mat(q2);

    dvec6 force_and_tau1i;
    if (ins.ord == 2)
        force_and_tau1i = mut_force_tau1i_integrals_ord2(r, ins.M1,pars.J1,A1, ins.M2,pars.J2,A2);
    else if (ins.ord == 3)
        force_and_tau1i = mut_force_tau1i_integrals_ord3(r, ins.M1,pars.J1,A1, ins.M2,pars.J2,A2);
    else //Only order 4 remains to be tested.
        force_and_tau1i = mut_force_tau1i_integrals_ord4(r, ins.M1,pars.J1,A1, ins.M2,pars.J2,A2);

    dvec3 force = {force_and_tau1i[0], force_and_tau1i[1], force_and_tau1i[2]};

    dvec3 tau1i = {force_and_tau1i[3], force_and_tau1i[4], force_and_tau1i[5]};
    dvec3 tau2i = -tau1i - cross(r,force);

    dvec3 tau1b = iner2body(tau1i,A1);
    dvec3 tau2b = iner2body(tau2i,A2);

    dvec4 dq1 = quat_rhs(q1,w1b);
    dvec3 dw1b = euler_rhs(w1b,pars.I1,tau1b);

    dvec4 dq2 = quat_rhs(q2,w2b);
    dvec3 dw2b = euler_rhs(w2b,pars.I2,tau2b);

    //Spacecraft's state.
    dvec3 rsp = { state[20], state[21], state[22] };
    dvec3 vsp = { state[23], state[24], state[25] };

    double com1_coeff = -ins.M2/(ins.M1 + ins.M2);
    double com2_coeff =  ins.M1/(ins.M1 + ins.M2);
    dvec3 r1 = com1_coeff*r;
    dvec3 r2 = com2_coeff*r;

    dvec3 asp; //Spacecraft's acceleration due to the presence of the 2 rigid bodies.
    if (ins.ord == 2)
        asp = accel_integrals_ord2(rsp - r1, ins.M1, pars.J1, A1) + accel_integrals_ord2(rsp - r2, ins.M2, pars.J2, A2);
    else if (ins.ord == 3)
        asp = accel_integrals_ord3(rsp - r1, ins.M1, pars.J1, A1) + accel_integrals_ord3(rsp - r2, ins.M2, pars.J2, A2);
    else //Only order 4 remains to be tested.
        asp = accel_integrals_ord4(rsp - r1, ins.M1, pars.J1, A1) + accel_integrals_ord4(rsp - r2, ins.M2, pars.J2, A2);

    bool sp_in_shadow = false;
    if (ins.shadow) //If shadow is taken into account, then check if the spacecraft actually is in shadow now via the bool sp_in_shadow and if not, add SRP.
    {
        sp_in_shadow = line_sphere_intersection(rsp, pars.rsun, r1, pars.brillouin1) || line_sphere_intersection(rsp, pars.rsun, r2, pars.brillouin2);
        if (!sp_in_shadow)
            asp = asp + accel_srp(ins.rhosp, ins.Asp, ins.msp, rsp, pars.rsun);
    }
    else //Otherwise, if shadow is not selected to be taken into account, just add SRP no matter what.
        asp = asp + accel_srp(ins.rhosp, ins.Asp, ins.msp, rsp, pars.rsun);

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

    //Spacecraft's position and velocity rhs (xsp,ysp,zsp, vxsp,vysp,vzsp).
    dstate[20] = vsp[0];
    dstate[21] = vsp[1];
    dstate[22] = vsp[2];
    dstate[23] = asp[0];
    dstate[24] = asp[1];
    dstate[25] = asp[2];
}

int main()
{
    std::filesystem::create_directory("io");

    ins.read();
    pars.m = ins.M1*ins.M2/(ins.M1 + ins.M2);
    
    printf("Loading polyhedra... ");
    polyhedron poly1;
    poly1.load_obj_file(ins.filename1);
    poly1.set_com_zero();
    poly1.set_inertia_diagonal();
    pars.I1 = poly1.get_inertia(ins.M1);
    pars.brillouin1 = poly1.get_farthest_vertex_distance();
    polyhedron poly2;
    poly2.load_obj_file(ins.filename2);
    poly2.set_com_zero();
    poly2.set_inertia_diagonal();
    pars.I2 = poly2.get_inertia(ins.M2);
    pars.brillouin2 = poly2.get_farthest_vertex_distance();
    if (ins.ord == 2)
    {
        pars.J1 = poly1.get_inertial_integrals_ord2(ins.M1);
        pars.J2 = poly2.get_inertial_integrals_ord2(ins.M2);
    }
    else if (ins.ord == 3)
    {
        pars.J1 = poly1.get_inertial_integrals_ord3(ins.M1);
        pars.J2 = poly2.get_inertial_integrals_ord3(ins.M2);
    }
    else
    {
        pars.J1 = poly1.get_inertial_integrals_ord4(ins.M1);
        pars.J2 = poly2.get_inertial_integrals_ord4(ins.M2);
    }
    printf("Done.\n");

    pars.rsun = spher2cart({ins.dist_sun, ins.lon_sun*PI/180.0, ins.lat_sun*PI/180.0}); //{[AU], [AU], [AU]}
    pars.rsun = pars.rsun*AU2KM; //{[km], [km], [km]}

    //Time parameters.
    double t, t0 = ins.t0;  //[sec]
    double tmax = ins.tmax; //[sec]
    double dt_guess = 1.0;  //[sec]

    dvec6 cart = kep2cart({ins.a, ins.e, ins.i*PI/180.0, ins.Om*PI/180, ins.w*PI/180, ins.M*PI/180}, G*(ins.M1+ins.M2));
    dvec3 r   = {cart[0],   cart[1],  cart[2]};        //[km]
    dvec3 v   = {cart[3],   cart[4],  cart[5]};        //[km/sec]
    dvec4 q1  = {ins.q10, ins.q11, ins.q12, ins.q13};  //[ ]
    dvec3 w1i = {ins.w1x, ins.w1y, ins.w1z};           //[rad/sec]
    dvec4 q2  = {ins.q20, ins.q21, ins.q22, ins.q23};  //[ ]
    dvec3 w2i = {ins.w2x, ins.w2y, ins.w2z};           //[rad/sec]

    dvec3 rsp = {ins.xsp,  ins.ysp,  ins.zsp};         //[km]
    dvec3 vsp = {ins.vxsp, ins.vysp, ins.vzsp};        //[km/sec]

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
                                       rsp[0],  rsp[1],  rsp[2],
                                       vsp[0],  vsp[1],  vsp[2] };
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

        //Check for sphere-sphere collision regarding asteroids or sphere-point collision regarding asteroid-spacecraft (point mass).
        dvec3 r   = dvec3{state[0],state[1],state[2]};
        dvec3 rsp = dvec3{state[20],state[21],state[22]};
        if (sphere_sphere_collision(length(r), pars.brillouin1, pars.brillouin2) ||
            length(rsp + ins.M2*r/(ins.M1+ins.M2)) <= pars.brillouin1 ||
            length(rsp - ins.M1*r/(ins.M1+ins.M2)) <= pars.brillouin2 )
        {
            collision = true;
            break;
        }
        //Update state.
        method.try_step(odes, state, t, dt_guess);
    }
    
    //Write 'orbit' data into files.
    FILE *fp_t      = fopen("io/time.txt","w");
    FILE *fp_pos    = fopen("io/rel_pos.txt","w");
    FILE *fp_vel    = fopen("io/rel_vel.txt","w");
    FILE *fp_rpy1   = fopen("io/euler_rpy1.txt","w");
    FILE *fp_w1i    = fopen("io/ang_vel_w1i.txt","w");
    FILE *fp_rpy2   = fopen("io/euler_rpy2.txt","w");
    FILE *fp_w2i    = fopen("io/ang_vel_w2i.txt","w");
    FILE *fp_EL_err = fopen("io/ener_mom_rel_error.txt","w");
    FILE *fp_pos_spacecraft  = fopen("io/pos_spacecraft.txt","w");
    FILE *fp_vel_spacecraft  = fopen("io/vel_spacecraft.txt","w");
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
        dvec3 rpy1 = quat2ang(q1)*180.0/PI;
        dvec3 rpy2 = quat2ang(q2)*180.0/PI;
        dvec3 rs = {orbit[i][21], orbit[i][22], orbit[i][23]};
        dvec3 vs = {orbit[i][24], orbit[i][25], orbit[i][26]};
        double energy = 0.5*pars.m*dot(v,v) + 0.5*dot( dot(w1b,pars.I1), w1b) + 0.5*dot( dot(w2b,pars.I2), w2b); //Kinetic energy part of the binary.

        //And now we add the corresponding potential.
        if (ins.ord == 2)
            energy += mut_pot_integrals_ord2(r, ins.M1,pars.J1,A1, ins.M2,pars.J2,A2);
        else if (ins.ord == 3)
            energy += mut_pot_integrals_ord3(r, ins.M1,pars.J1,A1, ins.M2,pars.J2,A2);
        else
            energy += mut_pot_integrals_ord4(r, ins.M1,pars.J1,A1, ins.M2,pars.J2,A2);
        
        double momentum = length(pars.m*cross(r,v) + dot(A1, dot(pars.I1,w1b)) + dot(A2, dot(pars.I2,w2b)));

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

        fprintf(fp_t,"%.16lf\n", orbit[i][0]/86400.0); //Export t in [days]
        fprintf(fp_pos,"%.16lf %.16lf %.16lf %.16lf\n",r[0],r[1],r[2], length(r));
        fprintf(fp_vel,"%.16lf %.16lf %.16lf %.16lf\n",v[0],v[1],v[2], length(v));

        fprintf(fp_rpy1,"%.16lf %.16lf %.16lf\n",rpy1[0],rpy1[1],rpy1[2]);
        fprintf(fp_w1i,"%.16lf %.16lf %.16lf\n",w1i[0],w1i[1],w1i[2]);
        fprintf(fp_rpy2,"%.16lf %.16lf %.16lf\n",rpy2[0],rpy2[1],rpy2[2]);
        fprintf(fp_w2i,"%.16lf %.16lf %.16lf\n",w2i[0],w2i[1],w2i[2]); 
        fprintf(fp_EL_err,"%.16lf %.16lf\n", energy_rel_err, momentum_rel_err);

        fprintf(fp_pos_spacecraft,"%.16lf %.16lf %.16lf %.16lf\n",rs[0],rs[1],rs[2], length(rs));
        fprintf(fp_vel_spacecraft,"%.16lf %.16lf %.16lf %.16lf\n",vs[0],vs[1],vs[2], length(vs));
    }
    fclose(fp_t);
    fclose(fp_pos);
    fclose(fp_vel);

    fclose(fp_rpy1);
    fclose(fp_w1i);
    fclose(fp_rpy2);
    fclose(fp_w2i);
    fclose(fp_EL_err);
    fclose(fp_pos_spacecraft);
    fclose(fp_vel_spacecraft);

    FILE *fp_collision = fopen("io/collision.txt","w");
    fprintf(fp_collision, "Collision detected : %s", collision ? "Yes" : "No");
    fclose(fp_collision);

    printf("\rProgress: 100%%  ");
    fflush(stdout);
    printf("\nDone.\n");

    return 0;
}
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
    dvec3 rsun; //Sun's position vector.
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
    double rhosp, Asp, msp; //Spacecraft's reflectivity coefficient, effective area and mass (for the SRP).
    double dist_sun, lon_sun, lat_sun; //Sun's distance (in AU), longitude and latitude (for the SRP).
    int shadow; //0 means we do not account for shadow in the SRP, 1 means we account for shadow in the SRP. 
    double a_sp_min, a_sp_max, e_sp_min, e_sp_max; //Initial sate of a,e of the spacecraft.
    int Na, Ne; //Grid resolution in a(0),e(0) of the spacecraft for the batch.
    double i_sp, Om_sp, w_sp, M_sp; //Rest of the spacecraft's Keplerian elements.

    void read()
    {
        FILE *fpins = fopen("inputs_run_batch.txt", "r");
        if (!fpins)
        {
            fprintf(stderr, "Error: file 'inputs_run_batch.txt' was not found. Exiting...\n");
            exit(EXIT_FAILURE);
        }
        if (find_assignment_operator(fpins)) fscanf(fpins, " \"%[^\"]\"", filename1);
        if (find_assignment_operator(fpins)) fscanf(fpins, " \"%[^\"]\"", filename2);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%d",  &ord);
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
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &rhosp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &Asp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &msp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &dist_sun);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &lon_sun);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &lat_sun);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%d", &shadow);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &a_sp_min);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &a_sp_max);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &e_sp_min);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &e_sp_max);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%d", &Na);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%d", &Ne);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &i_sp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &Om_sp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &w_sp);
        if (find_assignment_operator(fpins)) fscanf(fpins, "%lf", &M_sp);
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

    dvec6 cart_bin = kep2cart({ins.a, ins.e, ins.i*PI/180.0, ins.Om*PI/180, ins.w*PI/180, ins.M*PI/180}, G*(ins.M1+ins.M2));
    dvec3 r   = {cart_bin[0], cart_bin[1], cart_bin[2]}; //[km]
    dvec3 v   = {cart_bin[3], cart_bin[4], cart_bin[5]}; //[km/sec]
    dvec4 q1  = {ins.q10, ins.q11, ins.q12, ins.q13};    //[ ]
    dvec3 w1i = {ins.w1x, ins.w1y, ins.w1z};             //[rad/sec]
    dvec4 q2  = {ins.q20, ins.q21, ins.q22, ins.q23};    //[ ]
    dvec3 w2i = {ins.w2x, ins.w2y, ins.w2z};             //[rad/sec]

    q1 = quat2unit(q1);
    q2 = quat2unit(q2);

    dvec3 w1b = iner2body(w1i, quat2mat(q1));
    dvec3 w2b = iner2body(w2i, quat2mat(q2));

    //Template state with only the binary filled in :
    boost::array<double, 26> state0 = {   r[0],    r[1],    r[2],
                                          v[0],    v[1],    v[2],
                                         q1[0],   q1[1],   q1[2], q1[3],
                                        w1b[0],  w1b[1],  w1b[2],
                                         q2[0],   q2[1],   q2[2], q2[3],
                                        w2b[0],  w2b[1],  w2b[2],
                                          0.0,     0.0,     0.0,   
                                          0.0,     0.0,     0.0  };

    auto method = boost::numeric::odeint::make_controlled(ins.atol, ins.rtol, boost::numeric::odeint::runge_kutta_fehlberg78<boost::array<double, 26>>());

    //Output files :
    FILE *fp_map  = fopen("stability_map.txt","w");
    FILE *fp_info = fopen("batch_info.txt","w");
    if (!fp_map || !fp_info)
    {
        fprintf(stderr, "Error : could not open output files.\n");
        exit(EXIT_FAILURE);
    }

    int Nmax_global = 0;

    //Precompute constant elements for spacecraft :
    const double i_sp_rad  = ins.i_sp*PI/180.0;
    const double Om_sp_rad = ins.Om_sp*PI/180.0;
    const double w_sp_rad  = ins.w_sp*PI/180.0;
    const double M_sp_rad  = ins.M_sp*PI/180.0;

    //For progress display.
    const int total_points = ins.Na*ins.Ne;
    int point_counter = 0;

    //Grid over (a_sp, e_sp) :
    for (int ia = 0; ia < ins.Na; ++ia)
    {
        double a_sp = (ins.Na > 1) ? ins.a_sp_min + (ins.a_sp_max - ins.a_sp_min)*ia/(ins.Na - 1) : ins.a_sp_min;
        for (int je = 0; je < ins.Ne; ++je)
        {
            double e_sp = (ins.Ne > 1) ? ins.e_sp_min + (ins.e_sp_max - ins.e_sp_min)*je/(ins.Ne - 1) : ins.e_sp_min;

            ++point_counter;
            double grid_progress = 100.0*point_counter/double(total_points);
            printf("\rGrid progress: %.1f%%", grid_progress);
            fflush(stdout);

            //Spacecraft's initial Keplerian elements (COM frame) :
            dvec6 kep_sp = {a_sp, e_sp, i_sp_rad, Om_sp_rad, w_sp_rad, M_sp_rad};

            dvec6 cart_sp = kep2cart(kep_sp, G*(ins.M1 + ins.M2));
            dvec3 rsp0 = {cart_sp[0], cart_sp[1], cart_sp[2]};
            dvec3 vsp0 = {cart_sp[3], cart_sp[4], cart_sp[5]};

            //Initialize state and time.
            boost::array<double, 26> state = state0;
            state[20] = rsp0[0];
            state[21] = rsp0[1];
            state[22] = rsp0[2];
            state[23] = vsp0[0];
            state[24] = vsp0[1];
            state[25] = vsp0[2];

            double t = t0;

            bool collision_bin = false, collision1 = false, collision2 = false;

            //For plane intersections :
            bool first_intersection = false;
            double x0 = 0.0, z0 = 0.0;
            double max_dist_xz = 0.0;
            int N_intersections = 0;

            boost::array<double, 26> prev_state = state;
            double t_prev = t;
            while (t < tmax)
            {
                method.try_step(odes, state, t, dt_guess);

                //Collision check (sphere–sphere + point–sphere)
                dvec3 r_rel   = { state[0],  state[1],  state[2] };
                dvec3 rsp_now = { state[20], state[21], state[22] };

                if (sphere_sphere_collision(length(r_rel), pars.brillouin1, pars.brillouin2)) //Asteroid - asteroid
                {
                    collision_bin = true;
                    break;
                }
                else if (length(rsp_now + ins.M2*r_rel/(ins.M1 + ins.M2)) <= pars.brillouin1) //Asteroid 1 - spacecraft
                {
                    collision1 = true;
                    break;
                }
                else if (length(rsp_now - ins.M1*r_rel/(ins.M1 + ins.M2)) <= pars.brillouin2) //Asteroid 2 - spacecraft
                {
                    collision2 = true;
                    break;
                }

                //Plane y = 0 intersections (xz plane), with direction vy > 0.
                double y_old = prev_state[21];
                double y_new = state[21];
                double vy_old = prev_state[24];

                if ( (y_old < 0.0 && y_new >= 0.0) || (y_old > 0.0 && y_new <= 0.0) )
                {
                    if (vy_old > 0.0 && fabs(y_new - y_old) > 1e-12)
                    {
                        //Linear interpolation between prev_state and state :
                        double lambda = -y_old/(y_new - y_old); //in [0,1]

                        double x_old = prev_state[20];
                        double z_old = prev_state[22];
                        double x_new = state[20];
                        double z_new = state[22];

                        double x_cross = x_old + lambda*(x_new - x_old);
                        double z_cross = z_old + lambda*(z_new - z_old);

                        if (!first_intersection)
                        {
                            first_intersection = true;
                            x0 = x_cross;
                            z0 = z_cross;
                            max_dist_xz = 0.0;
                        }
                        else
                        {
                            double dx = x_cross - x0;
                            double dz = z_cross - z0;
                            double dist = sqrt(dx*dx + dz*dz);
                            if (dist > max_dist_xz)
                                max_dist_xz = dist;
                        }
                        ++N_intersections;
                    }
                }
                prev_state = state;
                t_prev = t;
            }

            //Record global maximum number of intersections among non-colliding cases
            if (!collision_bin && !collision1 && !collision2 && first_intersection)
            {
                if (N_intersections > Nmax_global)
                    Nmax_global = N_intersections;
            }

            //Output for this (a_sp, e_sp) :
            double value;
            if (collision_bin)
                value = -1.0;
            else if (collision1)
                value = -2.0;
            else if (collision2)
                value = -3.0;
            else if (!first_intersection)
                value = 0.0; //No intersection detected.
            else
                value = max_dist_xz;

            fprintf(fp_map, "%.15g %.15g %.15g\n", a_sp, e_sp, value);
        }
    }
    fprintf(fp_info, "%d %d %.15g\n", Nmax_global, ins.ord, tmax - t0);
    fclose(fp_map);
    fclose(fp_info);
    return 0;
}
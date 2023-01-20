#ifndef IO_HPP
#define IO_HPP

#include<cstdio>
#include<cmath>

#include"typedef.hpp"
#include"constant.hpp"
#include"ellipsoid.hpp"
#include"obj.hpp"
#include"polyhedron.hpp"
#include"mascon.hpp"
#include"conversion.hpp"
#include"force.hpp"
#include"torque.hpp"
#include"rigidbody.hpp"

//#include<boost/numeric/odeint.hpp>
//typedef boost::array<double, 20> boostvec20;
//typedef boost::numeric::odeint::runge_kutta_fehlberg78<boostvec20> rkf78;

class inputs
{
public:

    //Enable the close button on the root imgui window (not the glfw one). Once it is clicked, the game loop terminates.
    bool xclose = true;

    //'Simulation name' field. 50 characters available (plus the '\0' character)
    char simname[51] = "";

    //'Ellipsoids' checkbox state
    bool ell_checkbox = false;
    //Ellipsoids 'OK' button state (from the submenu)
    bool clicked_ell_ok = false;

    //Ellipsoids {'a1', 'b1', 'c1'}, {'a2', 'b2', 'c2'} fields.
    dvec3 semiaxes1 = {0.0,0.0,0.0};
    dvec3 semiaxes2 = {0.0,0.0,0.0};

    //'.obj files' checkbox state
    bool obj_checkbox = false;
    //.obj files 'Body 2' or 'Body 2' radiobuttons reference (1 or 2 only)
    int obj_refer_to_body = 1;
    //Index of the clicked .obj path. Either of mascons/, or polyhedra/ directory.
    //-1 means no path is clicked.
    //Only one path per body can be clicked.
    int clicked_masc1_index = -1, clicked_poly1_index = -1;
    int clicked_masc2_index = -1, clicked_poly2_index = -1;
    //Decide whether body 1 and body 2 will be loaded as polyhedra .obj
    bool clicked_poly1 = false, clicked_poly2 = false;
    //Relative path to the 2 .obj models.
    str obj_path1 = "", obj_path2 = "";
    //fundamental contents of the 2 .obj files
    bvec vfnt1 = {false, false, false, false}, vfnt2 = {false, false, false, false};
    //Cartesian grid resolutions (per axis) for filling the polyhedra with mascons.
    ivec3 grid_reso1 = {2,2,2};
    ivec3 grid_reso2 = {2,2,2};
    ivec3 grid_reso_null {0,0,0};
    //.obj 'OK' button state
    bool clicked_obj_ok = false;

    //'Theory' checkboxes states
    bool ord2_checkbox = false;
    bool ord3_checkbox = false;
    bool ord4_checkbox = false;
    bool mascons_checkbox = false;

    //'M1', 'M2' fields.
    double M1 = 0.0, M2 = 0.0;

    //'Epoch', 'Duration', 'Step' fields.
    double epoch = 0.0, dur = 0.0, step = 0.0;

    //Nature of the relative position and velocity variables.
    const char *cart_kep_var[2] = {"Cartesian ", "Keplerian "};
    //Initial choice. 0 -> Cartesian, 1 -> Keplerian.
    int cart_kep_var_choice = 0;

    //'x', 'y', 'z', 'vx', 'vy', 'vz' fields.
    dvec6 relcart = {0.0,0.0,0.0, 0.0,0.0,0.0};

    //'a', 'e', 'i', 'RAAN', 'w', 'M' fields.
    dvec6 relkep = {0.0,0.0,0.0,0.0,0.0,0.0};

    //Nature of the orientation variables.
    const char *orient_var[2] = {"Euler angles ", "Quaternions "};
    //Initial choice. 0 -> Euler angles (roll, pitch, yaw), 1 -> Quaternions.
    int orient_var_choice = 0;

    //'roll 1', 'pitch 1', 'yaw 1', 'roll 2', 'pitch 2', 'yaw 2' fields.
    dvec3 rpy1 = {0.0,0.0,0.0};
    dvec3 rpy2 = {0.0,0.0,0.0};

    //'q1 0', 'q1 1', 'q1 2', 'q1 3', 'q2 0', 'q2 1', 'q2 2', 'q2 3' fields.
    dvec4 q1 = {1.0,0.0,0.0,0.0};
    dvec4 q2 = {1.0,0.0,0.0,0.0};

    //Nature of frames.
    const char *frame_type[2] = {"Inertial frame", "Body frames"};
    //Initial choice. 0 -> Inertial frame, 1 -> Body frames.
    int frame_type_choice = 0;

    //'w1x', 'w1y', 'w1z', 'w2x', 'w2y, 'w2z' fields.
    dvec3 w1i = {0.0,0.0,0.0};
    dvec3 w2i = {0.0,0.0,0.0};

    //'w11', 'w12', 'w13', 'w21', 'w22, 'w23' fields.
    dvec3 w1b = {0.0,0.0,0.0};
    dvec3 w2b = {0.0,0.0,0.0};

    //Integration method
    const char *integ_method[4] = {"Runge-Kutta-Fehlberg 78 ", "Bulirsch-Stoer", "Dormand-Prince 5 ", "Runge-Kutta 4 (explicit)"};
    //Initial choice. 0 -> Runge-Kutta-Fehlberg 78, 1 -> Bulirsch-Stoer, etc..
    int integ_method_choice = 0;
    //'tolerance' field.
    double tol = 1e-9;

    //'Detect binary collision' checkbox state.
    bool detect_binary_collision = false;

    //'Files to export' checkboxes states.
    bool export_time = true;
    bool export_rel_pos_vel = true;
    bool export_ang = true;
    bool export_quat = true;
    bool export_wi = true;
    bool export_wb = true;
    bool export_ener_mom = true;
    bool export_rel_kep = true;

    //'Detect binary collision' checkbox state.
    bool playback_video = false;

    //'Run' button state
    bool clicked_run = false;

    ////////////////////////////////////////////////////////////////////////////////////////////

    dmatnx3 masc1, masc2; //mascon coordinates
    dtens J1, J2; //inertial integral tensors
    dmat3 I1, I2; //moments of inertia
    dmat sol;

    ////////////////////////////////////////////////////////////////////////////////////////////

    //If all inputs are valid, this member function returns an entirely empty vector of strings.
    //Otherwise the returned vector contains string messages, each corresponding to an invalid input. In this case,
    //the vector of strings will be displayed in the log and the simulation will not run.
    strvec validate()
    {
        strvec errors;
        
        //simname[] errors (empty, pure tabs, begin with tab).
        std::string simnname_copy = simname;
        char first_char = simnname_copy[0];
        char one_space = ' ';
        if ( (simnname_copy.empty()) || (simnname_copy.find_first_not_of(' ') == std::string::npos) || (first_char == one_space) )
        {
            errors.push_back("[Error] :  'Simulation name' is invalid.");
        }

        //Theory model checkboxes error (at least one must be checked).
        if (!ord2_checkbox && !ord3_checkbox && !ord4_checkbox && !mascons_checkbox)
        {
            errors.push_back("[Error] :  Neither 'Order 2', nor 'Order 3', nor 'Order 4', nor 'Mascons' theory is selected.");
        }

        //Shape model checkboxes error (at least one must be checked).
        if (!ell_checkbox && !obj_checkbox)
        {
            errors.push_back("[Error] :  Neither 'Ellipsoids' nor '.obj files' is selected as shape models.");
        }

        //Ellipsoids semiaxes error (all semiaxes must be > 0.0).
        if (ell_checkbox && (semiaxes1[0] <= 0.0 || semiaxes1[1] <= 0.0 || semiaxes1[2] <= 0.0))
        {
            errors.push_back("[Error] :  'a1', 'b1', 'c1' must be positive numbers.");
        }
        if (ell_checkbox && (semiaxes2[0] <= 0.0 || semiaxes2[1] <= 0.0 || semiaxes2[2] <= 0.0))
        {
            errors.push_back("[Error] :  'a2', 'b2', 'c2' must be positive numbers.");
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //.obj files error (at least one .obj file per body must be selected by the user).
        if(obj_checkbox && clicked_masc1_index == -1 && clicked_poly1_index == -1)
        {
            errors.push_back("[Error] :  No .obj file is selected for 'Body 1'.");
        }
        if(obj_checkbox && clicked_masc2_index == -1 && clicked_poly2_index == -1)
        {
            errors.push_back("[Error] :  No .obj file is selected for 'Body 2'.");
        }

        //.obj files error of mascons category (the .obj file must at least contain lines with the format 'v x y z' to be assumed as mascons).
        if(obj_checkbox && clicked_masc1_index != -1)
        {
            vfnt1 = checkobj(obj_path1.c_str());
            if (!vfnt1[0]) //no vertices were found in body1
            {
                errors.push_back("[Error] : In 'Body 1' .obj file, no vertices were found in order to be assumed as mascons.");
            }
        }
        if(obj_checkbox && clicked_masc2_index != -1)
        {
            vfnt2 = checkobj(obj_path2.c_str());
            if (!vfnt2[0]) //no vertices were found in body2
            {
                errors.push_back("[Error] : In 'Body 2' .obj file, no vertices were found in order to be assumed as mascons.");
            }
        }

        //.obj files error of polyhedron category (the .obj file must at least contain lines with the
        //format 'v x y z' AND ('f i j k' OR 'f i11/i12 i21/i22 i31/i32' OR 'f i11//i12 i21//i22 i31//i32' OR 'f i11/i12/i13 i21/i22/i23 i31/i32/i33') to be assumed as polyhedra).
        if (obj_checkbox && clicked_poly1)
        {
            vfnt1 = checkobj(obj_path1.c_str());
            if ( !(vfnt1[0] && vfnt1[1]) )
            {
                errors.push_back("[Error] :  In 'Body 1' .obj file, at least vertices and faces lines must be present.");
            }
        }
        if (obj_checkbox && clicked_poly2)
        {
            vfnt2 = checkobj(obj_path2.c_str());
            if ( !(vfnt2[0] && vfnt2[1]) )
            {
                errors.push_back("[Error] :  In 'Body 2' .obj file, at least vertices and faces lines must be present.");
            }
        }

        //raycast grid error (in order to successfully fill the polyhedron with mascons, the grid resolution must be > 1 per axis)
        if (obj_checkbox && clicked_poly1 && (grid_reso1[0] <= 1 || grid_reso1[1] <= 1 || grid_reso1[2] <= 1))
        {
            errors.push_back("[Error] :  Invalid 'x axis', 'y axis', 'z axis' resolution for 'Body 1'.");
        }
        if (obj_checkbox && clicked_poly2 && (grid_reso2[0] <= 1 || grid_reso2[1] <= 1 || grid_reso2[2] <= 1))
        {
            errors.push_back("[Error] :  Invalid 'x axis', 'y axis', 'z axis' resolution for 'Body 2'.");
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //Masses error (both M1 and M2 must be > 0.0).
        if (M1 <= 0.0)
        {
            errors.push_back("[Error] :  'M1' must be positive.");
        }
        if (M2 <= 0.0)
        {
            errors.push_back("[Error] :  'M2' must be positive.");
        }

        //Time parameters errors (must : epoch >= 0.0, dur >= 0.0, print_step <= dur) <<<<< for now.
        if (epoch < 0.0 || dur < 0.0 || step > dur)
        {
            errors.push_back("[Error] :  Invalid set of 'Epoch', 'Duration', 'Step'.");
        }

        //Relative position/velocity errors. Here, we assume that only the Keplerian elements 'a','e' might be invalid.
        if (cart_kep_var_choice == 1 && rela <= 0.0)
        {
            errors.push_back("[Error] :  Invalid semi-major axis 'a'.");
        }
        if (cart_kep_var_choice == 1 && rele >= 1.0)
        {
            errors.push_back("[Error] :  Invalid eccentricity 'e'.");
        }

        //Quaternion errors (zero quaternion). In case of non normalized quaternions, the program does it automatically.
        if (orient_var_choice == 1) //that is, if the user chose quaternions as orientation variables
        {
            double normq1 = sqrt(q10*q10 + q11*q11 + q12*q12 + q13*q13);
            double normq2 = sqrt(q20*q20 + q21*q21 + q22*q22 + q23*q23);
            if (normq1 <= machine_zero)
            {
                errors.push_back("[Error] :  Quaternion 1 ('q10', 'q11', 'q12', 'q13') must be nonzero.");
            }
            else
            {
                q10 /= normq1;
                q11 /= normq1;
                q12 /= normq1;
                q13 /= normq1;
            }
            if (normq2 <= machine_zero)
            {
                errors.push_back("[Error] :  Quaternion 2 ('q20', 'q21', 'q22', 'q23') must be nonzero.");
            }
            else
            {
                q20 /= normq2;
                q21 /= normq2;
                q22 /= normq2;
                q23 /= normq2;
            }
        }

        //Tolerance error (less than zero).
        if (tol <= 0.0)
        {
            errors.push_back("[Error] :  Invalid tolerance 'tol'.");
        }

        return errors;
    }

    //This member function decides what type of simulation the user intends to run. As a result, it sets up all
    //necessary parameters-variables needed for that very simulation. Next, it propagates the state vector (integrates the odes) in time
    //and ultimately constructs the final outputs object (class) of the simulation.
    /*
    void setup_params()
    {
        //semiaxes of the ellipsoids (IF ellipsoidal geometries are selected)
        dvec3 semiaxes1, semiaxes2;

        dmatnx3 verts1;
        imatnx3 faces1nx3; //for the vf case
        imatnx6 faces1nx6; //for the vfn or vft case
        imatnx9 faces1nx9; //for the vfnt case
        dmatnx3 norms1;
        dmatnx2 texs1;

        dmatnx3 verts2;
        imatnx3 faces2nx3; //for the vf case
        imatnx6 faces2nx6; //for the vfn or vft case
        imatnx9 faces2nx9; //for the vfnt case
        dmatnx3 norms2;
        dmatnx2 texs2;

        if (ell_checkbox)
        {
            semiaxes1 = {a1,b1,c1};
            semiaxes2 = {a2,b2,c2};
            if (ord2_checkbox)
            {
                J1 = ell_integrals(M1, semiaxes1, 2);
                J2 = ell_integrals(M2, semiaxes2, 2);
                I1 = ell_inertia(M1, semiaxes1);
                I1 = ell_inertia(M2, semiaxes2);
            }
            else if (ord3_checkbox)
            {
                J1 = ell_integrals(M1, semiaxes1, 3);
                J2 = ell_integrals(M2, semiaxes2, 3);
                I1 = ell_inertia(M1, semiaxes1);
                I1 = ell_inertia(M2, semiaxes2);
            }
            else if (ord4_checkbox)
            {
                J1 = ell_integrals(M1, semiaxes1, 4);
                J2 = ell_integrals(M2, semiaxes2, 4);
                I1 = ell_inertia(M1, semiaxes1);
                I1 = ell_inertia(M2, semiaxes2);
            }
            else
            {
                masc1 = fill_ell_with_masc(semiaxes1, ivec3{grid_reso_x1, grid_reso_y1, grid_reso_z1});
                masc2 = fill_ell_with_masc(semiaxes2, ivec3{grid_reso_x2, grid_reso_y2, grid_reso_z2});
                I1 = masc_inertia(M1, masc1);
                I1 = masc_inertia(M2, masc2);
            }
            //hence, assuming ellipsoidal geometries, all theories (expansion & mascons) have the necessary parameters for the propagation
        }
        else //obj_checkbox
        {
            //body 1
            if (clicked_masc1_index != -1) //then the user clicked a mascon model from the database for body 1
            {
                masc1 = loadobjv(obj_path1.c_str());
            }
            else //the user clicked polyhedron (v,f,n,t,...) model from the database for body 1
            {
                if (!vfnt1[2] && !vfnt1[3])
                {
                    loadobjvf(obj_path1.c_str(), verts1,faces1nx3);
                }
                else if (vfnt1[2] && !vfnt1[3])
                {
                    loadobjvfn(obj_path1.c_str(), verts1,faces1nx6,norms1);
                    for (int i = 0; i < faces1nx6.size(); ++i)
                    {
                        faces1nx3.push_back({faces1nx6[i][0], faces1nx6[i][2], faces1nx6[i][4]});
                    }
                }
                else if (!vfnt1[2] && vfnt1[3])
                {
                    loadobjvft(obj_path1.c_str(), verts1,faces1nx6,texs1);
                    for (int i = 0; i < faces1nx6.size(); ++i)
                    {
                        faces1nx3.push_back({faces1nx6[i][0], faces1nx6[i][2], faces1nx6[i][4]});
                    }
                }
                else if (vfnt1[2] && vfnt1[3])
                {
                    loadobjvfnt(obj_path1.c_str(), verts1,faces1nx9,norms1,texs1);
                    for (int i = 0; i < faces1nx9.size(); ++i)
                    {
                        faces1nx3.push_back({faces1nx9[i][0], faces1nx9[i][3], faces1nx9[i][6]});
                    }
                }
                masc1 = fill_poly_with_masc(verts1,faces1nx3, ivec3{10,10,10});
                //correct_masc_com(masc1);
                //correct_masc_inertia(M1, masc1);
            }
            //body 2
            if (clicked_masc2_index != -1) //then the user clicked a mascon model from the database for body 2
            {
                masc2 = loadobjv(obj_path2.c_str());
            }
            else //the user clicked polyhedron (v,f,n,t,...) model from the database for body 2
            {
                if (!vfnt2[2] && !vfnt2[3])
                {
                    loadobjvf(obj_path2.c_str(), verts2,faces2nx3);
                }
                else if (vfnt2[2] && !vfnt2[3])
                {
                    loadobjvfn(obj_path2.c_str(), verts2,faces2nx6,norms2);
                    for (int i = 0; i < faces2nx6.size(); ++i)
                    {
                        faces2nx3.push_back({faces2nx6[i][0], faces2nx6[i][2], faces2nx6[i][4]});
                    }
                }
                else if (!vfnt2[2] && vfnt2[3])
                {
                    loadobjvft(obj_path2.c_str(), verts2,faces2nx6,texs2);
                    for (int i = 0; i < faces2nx6.size(); ++i)
                    {
                        faces2nx3.push_back({faces2nx6[i][0], faces2nx6[i][2], faces2nx6[i][4]});
                    }
                }
                else if (vfnt2[2] && vfnt2[3])
                {
                    loadobjvfnt(obj_path2.c_str(), verts2,faces2nx9,norms2,texs2);
                    for (int i = 0; i < faces2nx9.size(); ++i)
                    {
                        faces2nx3.push_back({faces2nx9[i][0], faces2nx9[i][3], faces2nx9[i][6]});
                    }
                }
                masc2 = fill_poly_with_masc(verts2,faces2nx3, ivec3{10,10,10});
                //correct_masc_com(masc2);
                //correct_masc_inertia(M2, masc2);
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
            //hence, assuming .obj geometries, all theories (expansion & mascons) have the necessary parameters for the propagation
        }
        
        //check for bounded ic at some point...
        dvec6 cart;
        if (cart_kep_var_choice == 0) //the user chose Cartesian
        {
            cart = {relx,rely,relz, relvx,relvy,relvz};
        }
        else //the user chose Keplerian
        {
            //transform the Keplerian to Cartesian because the f2bp propagator is written in Cartesian elements
            cart = kep2cart(dvec6{rela,rele,reli,relraan,relw,relM}, G*(M1 + M2));
        }

        dvec4 q1,q2;
        if (orient_var_choice == 1)
        {
            q1 = {q10, q11, q12, q13};
            q2 = {q20, q21, q22, q23};
        }
        else //orientation variables were chosen to be Euler angles
        {
            q1 = ang2quat(dvec3{roll1,pitch1,yaw1});
            q2 = ang2quat(dvec3{roll2,pitch2,yaw2});
        }

        dvec3 w1i,w2i;
        dvec3 w1b,w2b;
        if (frame_type_choice == 0) //angular velocities were given in the "world" inertial frame
        {
            w1b = iner2body(dvec3{w1x, w1y, w1z}, quat2mat(q1));
            w2b = iner2body(dvec3{w2x, w2y, w2z}, quat2mat(q2));
        }
        else //angular velocities were given in the corresponding body frames and need no transformations
        {
            w1b = {w11, w12, w13};
            w2b = {w21, w22, w23};
        }

        //now everything is ready for the actual propagation

        //current state vector (ic)
        boostvec20 state = { cart[0], cart[1], cart[2],
                             cart[3], cart[4], cart[5],
                               q1[0],   q1[1],   q1[2], q1[3],
                              w1b[0],  w1b[1],  w1b[2],
                               q2[0],   q2[1],   q2[2], q2[3],
                              w2b[0],  w2b[1],  w2b[2] };
        
        sol.clear();
        unsigned steps = integrate_adaptive(make_controlled(tol, tol, rkf78()), odes, state, epoch, dur, step, observe);

        //bool madedir = std::filesystem::create_directory(simname);
        //printf("%s\n",std::filesystem::current_path(std::filesystem::temp_directory_path()));

        return;
    }
    */

    /*
    void observe(const boostvec20 &state, const double t)
    {
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
        return;
    }
    */

    /*
    void odes(const boostvec20 &state, boostvec20 &dstate, double t)
    {
        dvec3 r  =  { state[0],  state[1],  state[2] };
        dvec3 v  =  { state[3],  state[4],  state[5] };
        dvec4 q1 =  { state[6],  state[7],  state[8],  state[9] };
        dvec3 w1b = { state[10], state[11], state[12] };
        dvec4 q2 =  { state[13], state[14], state[15], state[16] };
        dvec3 w2b = { state[17], state[18], state[19] };

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
    */



    
};

/*
class ouputs
{
public:

    //here go the solution matrices

    void export_in_dir()
    {

    }
};
*/

#endif
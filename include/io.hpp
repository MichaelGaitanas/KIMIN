#ifndef IO_HPP
#define IO_HPP

#include<cstdio>
#include<cmath>
#include<filesystem>
#include<vector>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"directory.hpp"
#include"obj.hpp"
#include"conversion.hpp"
#include"polyhedron.hpp"
#include"mascon.hpp"
#include"ellipsoid.hpp"
#include"rigidbody.hpp"

class inputs
{
public:

    //Enable the close button on the root imgui window (not the glfw one). Once it is clicked, the game loop terminates.
    bool xclose; 

    //'Simulation name' field. 50 characters available (plus the '\0' character).
    char simname[51];

    //'Ellipsoids' checkbox state.
    bool ell_checkbox;
    //Ellipsoids 'OK' button state (from the submenu).
    bool clicked_ell_ok;

    //Ellipsoids {'a1', 'b1', 'c1'}, {'a2', 'b2', 'c2'} fields.
    dvec3 semiaxes1, semiaxes2;

    //'.obj files' checkbox state.
    bool obj_checkbox;
    //.obj files 'Body 2' or 'Body 2' radiobuttons reference (1 or 2 only).
    int obj_refer_to_body;

    //Index of the clicked .obj path. -1 means no path is clicked. Only one path per body can be clicked.
    int clicked_masc1_index, clicked_poly1_index;
    int clicked_masc2_index, clicked_poly2_index;
    
    //Decide whether body 1 and body 2 will be loaded as polyhedra .obj
    bool clicked_poly1, clicked_poly2;
    
    //Relative paths to mascons/ and polyhedron/ directories.
    std::vector<std::filesystem::path> path_to_masc_obj;
    std::vector<std::filesystem::path> path_to_poly_obj;

    //Relative path to the 2 .obj models.
    str obj_path1, obj_path2;

    //fundamental contents of the 2 .obj files (vertices, faces, normals, textures).
    bvec vfnt1, vfnt2;

    //Cartesian grid resolutions (per axis) for filling the polyhedra with mascons.
    ivec3 grid_reso1;
    ivec3 grid_reso2;
    ivec3 grid_reso_null;

    //.obj 'OK' button state.
    bool clicked_obj_ok;

    //'Theory' checkboxes states.
    bool ord2_checkbox, ord3_checkbox, ord4_checkbox, mascons_checkbox;

    //'M1', 'M2' fields.
    double M1, M2;

    //'Epoch', 'Duration', 'Step' fields.
    double epoch, dur, step;

    //Nature of the relative position and velocity variables.
    const char *cart_kep_var[2];
    //Initial choice. 0 -> Cartesian, 1 -> Keplerian.
    int cart_kep_var_choice;

    //'x', 'y', 'z', 'vx', 'vy', 'vz' fields.
    dvec6 relcart;
    //'a', 'e', 'i', 'RAAN', 'w', 'M' fields.
    dvec6 relkep;

    //Nature of the orientation variables.
    const char *orient_var[2];
    //Initial choice. 0 -> Euler angles (roll, pitch, yaw), 1 -> Quaternions.
    int orient_var_choice;

    //'roll 1', 'pitch 1', 'yaw 1', 'roll 2', 'pitch 2', 'yaw 2' fields.
    dvec3 rpy1, rpy2;
    //'q1 0', 'q1 1', 'q1 2', 'q1 3', 'q2 0', 'q2 1', 'q2 2', 'q2 3' fields
    dvec4 q1, q2;

    //Nature of frames.
    const char *frame_type[2];
    //Initial choice. 0 -> Inertial frame, 1 -> Body frames.
    int frame_type_choice;

    //'w1x', 'w1y', 'w1z', 'w2x', 'w2y, 'w2z' fields.
    dvec3 w1i, w2i;
    //'w11', 'w12', 'w13', 'w21', 'w22, 'w23' fields.
    dvec3 w1b, w2b;

    //Integration methods.
    const char *integ_method[4];
    //Initial choice. 0 -> Runge-Kutta-Fehlberg 78, 1 -> Bulirsch-Stoer, etc..
    int integ_method_choice;
    //'tolerance' field.
    double tol;

    //'Detect binary collision' checkbox state.
    bool detect_binary_collision;

    //'Playback video' checkbox state.
    //This is a magic button that will perform miracles in the future.
    bool playback_video;

    //'Run' button state.
    bool clicked_run;

    //These are some essential asteroid variables needed for the integration of the odes.
    //They are evaluated at run time via some physics functions.
    //However we preserve them as member variables because Boost's odes( ) function has no way of
    //passing user defined variables. But by implementig odes( ) as a member function of this class,
    //the functions that compute the forces, torques, etc, will know these essential parameters.
    ///////////////////////////
    dmatnx3 masc1, masc2;    // mascon coordinates
    dtens J1, J2;            // inertial integral tensors
    dmat3 I1, I2;            // moments of inertia
    dmat sol;                // solution matrix that contains all the state vector in time
    ///////////////////////////

    //Constructor
    inputs() : xclose(true),
               simname(""),
               ell_checkbox(false),
               clicked_ell_ok(false),
               semiaxes1({0.0,0.0,0.0}),
               semiaxes2({0.0,0.0,0.0}),
               obj_checkbox(false),
               obj_refer_to_body(1),
               clicked_masc1_index(-1),
               clicked_poly1_index(-1),
               clicked_masc2_index(-1),
               clicked_poly2_index(-1),
               clicked_poly1(false),
               clicked_poly2(false),
               path_to_masc_obj(lsfiles("../resources/obj/mascons/")),
               path_to_poly_obj(lsfiles("../resources/obj/polyhedra/")),
               obj_path1(""),
               obj_path2(""),
               vfnt1({false, false, false, false}),
               vfnt2({false, false, false, false}),
               grid_reso1({2,2,2}),
               grid_reso2({2,2,2}),
               grid_reso_null({0,0,0}),
               clicked_obj_ok(false),
               ord2_checkbox(false),
               ord3_checkbox(false),
               ord4_checkbox(false),
               mascons_checkbox(false),
               M1(0.0),
               M2(0.0),
               epoch(0.0),
               dur(0.0),
               step(0.0),
               cart_kep_var{"Cartesian ", "Keplerian "},
               cart_kep_var_choice(0),
               relcart({0.0,0.0,0.0,0.0,0.0,0.0}),
               relkep({0.0,0.0,0.0,0.0,0.0,0.0,}),
               orient_var{"Euler angles", "Quaternions"},
               orient_var_choice(0),
               rpy1({0.0,0.0,0.0}),
               rpy2({0.0,0.0,0.0}),
               q1({1.0,0.0,0.0,0.0}),
               q2({1.0,0.0,0.0,0.0}),
               frame_type{"Inertial frame", "Body frames"},
               frame_type_choice(0),
               w1i({0.0,0.0,0.0}),
               w2i({0.0,0.0,0.0}),
               w1b({0.0,0.0,0.0}),
               w2b({0.0,0.0,0.0}),
               integ_method{"Runge-Kutta-Fehlberg 78 ", "Bulirsch-Stoer", "Dormand-Prince 5 ", "Runge-Kutta 4 (explicit)"},
               integ_method_choice(0),
               tol(1e-9),
               detect_binary_collision(false),
               playback_video(false),
               clicked_run(false)
    { }

    //If all user inputs are valid, this member function returns an entirely empty vector of strings.
    //Otherwise the returned vector contains string messages, each corresponding to an invalid input. In this case,
    //the vector of strings will be displayed in the log and the simulation will not run.
    strvec validate()
    {
        strvec errors;
        
        //simname[] errors (empty, pure tabs, begin with tab).
        str simnname_copy = simname;
        char first_char = simnname_copy[0];
        char one_space = ' ';
        if ( (simnname_copy.empty()) || (simnname_copy.find_first_not_of(' ') == std::string::npos) || (first_char == one_space) )
            errors.push_back("[Error] :  'Simulation name' is invalid.");

        //Theory model checkboxes error (at least one must be checked).
        if (!ord2_checkbox && !ord3_checkbox && !ord4_checkbox && !mascons_checkbox)
            errors.push_back("[Error] :  Neither 'Order 2', nor 'Order 3', nor 'Order 4', nor 'Mascons' theory is selected.");

        //Shape model checkboxes error (at least one must be checked).
        if (!ell_checkbox && !obj_checkbox)
            errors.push_back("[Error] :  Neither 'Ellipsoids' nor '.obj files' is selected as shape models.");

        //Ellipsoids semiaxes error (all semiaxes must be > 0.0).
        if (ell_checkbox && (semiaxes1[0] <= 0.0 || semiaxes1[1] <= 0.0 || semiaxes1[2] <= 0.0))
            errors.push_back("[Error] :  'a1', 'b1', 'c1' must be positive numbers.");
        if (ell_checkbox && (semiaxes2[0] <= 0.0 || semiaxes2[1] <= 0.0 || semiaxes2[2] <= 0.0))
            errors.push_back("[Error] :  'a2', 'b2', 'c2' must be positive numbers.");

        //.obj files error (at least one .obj file per body must be selected by the user).
        if(obj_checkbox && clicked_masc1_index == -1 && clicked_poly1_index == -1)
            errors.push_back("[Error] :  No .obj file is selected for 'Body 1'.");
        if(obj_checkbox && clicked_masc2_index == -1 && clicked_poly2_index == -1)
            errors.push_back("[Error] :  No .obj file is selected for 'Body 2'.");

        //.obj files error of mascons category (the .obj file must at least contain lines with the format 'v x y z' to be assumed as mascons).
        if(obj_checkbox && clicked_masc1_index != -1)
        {
            vfnt1 = checkobj(obj_path1.c_str());
            if (!vfnt1[0])
                errors.push_back("[Error] : In 'Body 1' .obj file, no vertices were found in order to be assumed as mascons.");
        }
        if(obj_checkbox && clicked_masc2_index != -1)
        {
            vfnt2 = checkobj(obj_path2.c_str());
            if (!vfnt2[0])
                errors.push_back("[Error] : In 'Body 2' .obj file, no vertices were found in order to be assumed as mascons.");
        }

        //.obj files error of polyhedron category (the .obj file must at least contain lines with the
        //format 'v x y z' AND ('f i j k' OR 'f i11/i12 i21/i22 i31/i32' OR 'f i11//i12 i21//i22 i31//i32' OR 'f i11/i12/i13 i21/i22/i23 i31/i32/i33') to be assumed as polyhedron).
        if (obj_checkbox && clicked_poly1)
        {
            vfnt1 = checkobj(obj_path1.c_str());
            if ( !(vfnt1[0] && vfnt1[1]) )
                errors.push_back("[Error] :  In 'Body 1' .obj file, at least vertices and faces lines must be present.");
        }
        if (obj_checkbox && clicked_poly2)
        {
            vfnt2 = checkobj(obj_path2.c_str());
            if ( !(vfnt2[0] && vfnt2[1]) )
                errors.push_back("[Error] :  In 'Body 2' .obj file, at least vertices and faces lines must be present.");
        }

        //Raycast grid error (in order to successfully fill the polyhedron with mascons, the raycast grid resolution must be > 1 per axis)
        if (obj_checkbox && clicked_poly1 && (grid_reso1[0] <= 1 || grid_reso1[1] <= 1 || grid_reso1[2] <= 1))
            errors.push_back("[Error] :  Invalid 'x axis', 'y axis', 'z axis' resolution for 'Body 1'.");
        if (obj_checkbox && clicked_poly2 && (grid_reso2[0] <= 1 || grid_reso2[1] <= 1 || grid_reso2[2] <= 1))
            errors.push_back("[Error] :  Invalid 'x axis', 'y axis', 'z axis' resolution for 'Body 2'.");

        //Masses error (both M1 and M2 must be > 0.0).
        if (M1 <= 0.0)
            errors.push_back("[Error] :  'M1' must be positive.");
        if (M2 <= 0.0)
            errors.push_back("[Error] :  'M2' must be positive.");

        //Time parameters errors (must : epoch >= 0.0, dur >= 0.0, print_step <= dur)
        if (epoch < 0.0 || dur < 0.0 || step > dur)
            errors.push_back("[Error] :  Invalid set of 'Epoch', 'Duration', 'Step'.");

        //Relative position/velocity errors. Here, we assume that only the Keplerian elements 'a','e' might be invalid.
        if (cart_kep_var_choice == 1 && relkep[0] <= 0.0)
            errors.push_back("[Error] :  Invalid semi-major axis 'a'.");
        if (cart_kep_var_choice == 1 && relkep[1] >= 1.0)
            errors.push_back("[Error] :  Invalid eccentricity 'e'.");

        //Quaternion errors (zero quaternion). In case of non normalized quaternions, the program normalizes it automatically.
        if (orient_var_choice == 1) //that is, if the user chose quaternions as orientation variables
        {
            double normq1 = length(q1);
            if (normq1 <= machine_zero) //error
                errors.push_back("[Error] :  Quaternion 1 ('q10', 'q11', 'q12', 'q13') must be nonzero.");
            else //normalize it no matter what
                q1 = quat2unit(q1);

            //the same for q2 ...
            double normq2 = length(q2);
            if (normq2 <= machine_zero)
                errors.push_back("[Error] :  Quaternion 2 ('q20', 'q21', 'q22', 'q23') must be nonzero.");
            else
                q2 = quat2unit(q2);
        }

        //Tolerance error (must be > 0.0).
        if (tol <= 0.0)
            errors.push_back("[Error] :  Invalid tolerance 'tol'.");

        return errors;
    }

    //This member function processes the (valid) inputs and decides what type of simulation the user intends to run. As a result, it sets up all
    //necessary variables needed for that very simulation. Next, it propagates the state vector (integrates the odes) in time
    //and ultimately constructs the final outputs object (class) of the simulation.
    void propagate()
    {
        if (ell_checkbox)
        {
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
                masc1 = fill_ell_with_masc(semiaxes1, grid_reso1);
                masc2 = fill_ell_with_masc(semiaxes2, grid_reso2);
                I1 = masc_inertia(M1, masc1);
                I1 = masc_inertia(M2, masc2);
            }
            //Note : For an ellipsoid I = { {J[0][2][0] + J[0][0][2],             0,                        0           },
            //                              {           0,             J[2][0][0] + J[0][0][2],             0           },
            //                              {           0,                        0,             J[2][0][0] + J[0][2][0]} }
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

            //The same for body 2. Probably pack it in a function...

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
            relcart = kep2cart(relkep, G*(M1+M2)); //transform Keplerian to Cartesian because the f2bp propagator is written in Cartesian elements

        if (orient_var_choice == 0) //orientation variables were chosen to be Euler angles
        {
            //transform Euler angles (roll, pitch, yaw) to quaternions because the f2bp propagator is written in quaternionic structure
            q1 = ang2quat(rpy1);
            q2 = ang2quat(rpy2);
        }

        if (frame_type_choice == 0) //angular velocities were given in the "world" inertial frame
        {
            //transform inertial angular velocities to the corresponding body ones because the Euler equations of rotation are written in the body frames
            w1b = iner2body(w1i, quat2mat(q1));
            w2b = iner2body(w2i, quat2mat(q2));
        }

        //Now everything is ready for the actual propagation.

        /*
        boostvec20 state = { relcart[0], relcart[1], relcart[2],
                             relcart[3], relcart[4], relcart[5],
                                  q1[0],      q1[1],      q1[2], q1[3],
                                 w1b[0],     w1b[1],     w1b[2],
                                  q2[0],      q2[1],      q2[2], q2[3],
                                 w2b[0],     w2b[1],     w2b[2] };


        for (int i = 0; i < 20; ++i)
        {
            printf("%lf\n",state[i]);
        }
        
        sol.clear();
        unsigned steps = integrate_adaptive(make_controlled(1e-15, 1e-15, rkf78()),
                                            std::bind(&inputs::odes, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                            state, epoch, dur, step,
                                            std::bind(&inputs::observe, this, std::placeholders::_1, std::placeholders::_2) );

        FILE *fpt = fopen("io/t.txt","w");
        FILE *fprv = fopen("io/rv.txt","w");
        FILE *fpq1 = fopen("io/q1.txt","w");
        FILE *fpw1i = fopen("io/w1i.txt","w");
        FILE *fpw1b = fopen("io/w1b.txt","w");
        FILE *fprpy1 = fopen("io/rpy1.txt","w");
        FILE *fpq2 = fopen("io/q2.txt","w");
        FILE *fpw2i = fopen("io/w2i.txt","w");
        FILE *fpw2b = fopen("io/w2b.txt","w");
        FILE *fprpy2 = fopen("io/rpy2.txt","w");
        FILE *fpEL = fopen("io/ener_mom.txt","w");
        for (int i = 0; i < sol.size(); ++i)
        {
            double t = sol[i][0];
            dvec3 r = {sol[i][1], sol[i][2], sol[i][3]};
            dvec3 v = {sol[i][4], sol[i][5], sol[i][6]};
            dvec4 q1 = {sol[i][7], sol[i][8], sol[i][9], sol[i][10]};
            dvec3 w1b = {sol[i][11], sol[i][12], sol[i][13]};
            dvec4 q2 = {sol[i][14], sol[i][15], sol[i][16], sol[i][17]};
            dvec3 w2b = {sol[i][18], sol[i][19], sol[i][20]};
            dmat3 A1 = quat2mat(q1);
            dmat3 A2 = quat2mat(q2);
            dvec3 w1i = body2iner(w1b,A1);
            dvec3 w2i = body2iner(w2b,A2);
            dvec3 rpy1 = quat2ang(q1);
            dvec3 rpy2 = quat2ang(q2);

            double ener = 0.5*((M1*M2)/(M1+M2))*dot(v,v) + 0.5*dot( dot(w1b,I1), w1b) + 0.5*dot( dot(w2b,I2), w2b) + mut_pot_integrals_ord2(r, M1,J1,A1, M2,J2,A2);
            dvec3 mom = (M1*M2/(M1+M2))*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b));

            fprintf(fpt,"%.16lf\n",t);
            fprintf(fprv,"%.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n",r[0],r[1],r[2], v[0],v[1],v[2]);
            fprintf(fpq1,"%.16lf %.16lf %.16lf %.16lf\n",q1[0],q1[1],q1[2],q1[3]);
            fprintf(fprpy1,"%.16lf %.16lf %.16lf\n",rpy1[0],rpy1[1],rpy1[2]);
            fprintf(fpw1b,"%.16lf %.16lf %.16lf\n",w1b[0],w1b[1],w1b[2]);
            fprintf(fpw1i,"%.16lf %.16lf %.16lf\n",w1i[0],w1i[1],w1i[2]);
            fprintf(fpq2,"%.16lf %.16lf %.16lf %.16lf\n",q2[0],q2[1],q2[2],q2[3]);
            fprintf(fprpy2,"%.16lf %.16lf %.16lf\n",rpy2[0],rpy2[1],rpy2[2]);
            fprintf(fpw2b,"%.16lf %.16lf %.16lf\n",w2b[0],w2b[1],w2b[2]);
            fprintf(fpw2i,"%.16lf %.16lf %.16lf\n",w2i[0],w2i[1],w2i[2]); 
            fprintf(fpEL,"%.16lf %.16lf %.16lf %.16lf\n", ener, mom[0],mom[1],mom[2]);
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
        fprintf(fpsteps,"%d\n",steps);
        fclose(fpsteps);
        

        //while (epoch <= dur)
        //{
        //    rkf78().do_step( odes, state, epoch, dur);
        //    epoch += step;
        //}

        //bool madedir = std::filesystem::create_directory(simname);
        //printf("%s\n",std::filesystem::current_path(std::filesystem::temp_directory_path()));
        */

        return;
    }

};

/*
class ouputs (inherit the inputs class)
{
public:

    //here go the solution matrices

    void export_in_dir()
    {

    }
};
*/

#endif
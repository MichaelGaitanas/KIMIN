#ifndef INPUTS_HPP
#define INPUTS_HPP

#include<cstdio>
#include<cmath>
#include<boost/numeric/odeint.hpp>

#include"typedef.hpp"
#include"constant.hpp"
#include"ellipsoid.hpp"
#include"loader.hpp"
#include"polyhedron.hpp"
#include"mascon.hpp"

typedef boost::array<double, 20> boostvec20;
typedef boost::numeric::odeint::runge_kutta_fehlberg78<boostvec20> rkf78;

class inputs
{
public:

    //Enable the close button on the root window. Once it is clicked, the game loop terminates.
    bool xclose = true;

    //'Simulation name' field. 50 characters available (plus the '\0' character)
    char simname[51] = "";

    //'Ellipsoids' checkbox state
    bool ell_checkbox = false;
    //Ellipsoids 'OK' button state
    bool clicked_ell_ok = false;

    //Ellipsoids ['a1', 'b1', 'c1'], ['a2', 'b2', 'c2'] fields.
    double a1 = 0.0, b1 = 0.0, c1 = 0.0;
    double a2 = 0.0, b2 = 0.0, c2 = 0.0;

    //'.obj files' checkbox state
    bool obj_checkbox = false;
    //.obj files 'Body 2' or 'Body 2' radiobuttons reference (1 or 2 only)
    int obj_refer_to_body = 1;
    //Index of the clicked .obj path. Either of mascons/, or polyhedra/ directory.
    //-1 means no path is clicked.
    //Only one path per body can be clicked.
    int clicked_masc1_index = -1, clicked_poly1_index = -1;
    int clicked_masc2_index = -1, clicked_poly2_index = -1;
    //Relative path to the 2 .obj models.
    str obj_path1, obj_path2;
    //.obj 'OK' button state
    bool clicked_obj_ok = false;

    //'Theory' checkboxes states
    bool ord2_checkbox = true;
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
    double relx = 0.0, rely = 0.0, relz = 0.0;
    double relvx = 0.0, relvy = 0.0, relvz = 0.0;

    //'a', 'e', 'i', 'RAAN', 'w', 'M' fields.
    double rela = 0.0, rele = 0.0, reli = 0.0;
    double relraan = 0.0, relw = 0.0, relM = 0.0;

    //Nature of the orientation variables.
    const char *orient_var[2] = {"Euler angles ", "Quaternions "};
    //Initial choice. 0 -> Euler angles (roll, pitch, yaw), 1 -> Quaternions.
    int orient_var_choice = 0;

    //'roll 1', 'pitch 1', 'yaw 1', 'roll 2', 'pitch 2', 'yaw 2' fields.
    double roll1 = 0.0, pitch1 = 0.0, yaw1 = 0.0;
    double roll2 = 0.0, pitch2 = 0.0, yaw2 = 0.0;

    //'q1 0', 'q1 1', 'q1 2', 'q1 3', 'q2 0', 'q2 1', 'q2 2', 'q2 3' fields.
    double q10 = 1.0, q11 = 0.0, q12 = 0.0, q13 = 0.0;
    double q20 = 1.0, q21 = 0.0, q22 = 0.0, q23 = 0.0;

    //Nature of frames.
    const char *frame_type[2] = {"Inertial frame", "Body frames"};
    //Initial choice. 0 -> Inertial frame, 1 -> Body frames.
    int frame_type_choice = 0;

    //'w1 x', 'w1 y', 'w1 z', 'w2 x', 'w2 y, 'w2 z' fields.
    double w1x = 0.0, w1y = 0.0, w1z = 0.0;
    double w2x = 0.0, w2y = 0.0, w2z = 0.0;

    //'w1 1', 'w1 2', 'w1 3', 'w2 1', 'w2 2, 'w2 3' fields.
    double w11 = 0.0, w12 = 0.0, w13 = 0.0;
    double w21 = 0.0, w22 = 0.0, w23 = 0.0;

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

    //If all inputs are valid, then this function returns an entirely empty vector of strings.
    //Otherwise the returned vector contains strings, each corresponding to an invalid input. In this case,
    //the vector of strings will be displayed in the log as errors.
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
            errors.push_back("[Error] :  Neither 'Ellipsoids' nor '.obj files' is selected.");
        }

        //Ellipsoids semiaxes error (all semiaxes must be > 0.0).
        if (ell_checkbox && (a1 <= 0.0 || b1 <= 0.0 || c1 <= 0.0))
        {
            errors.push_back("[Error] :  'a1', 'b1', 'c1' must be positive numbers.");
        }
        if (ell_checkbox && (a2 <= 0.0 || b2 <= 0.0 || b2 <= 0.0))
        {
            errors.push_back("[Error] :  'a2', 'b2', 'c2' must be positive numbers.");
        }

        //.obj files error (at leat one .obj file must be selected from the user).
        if(obj_checkbox && clicked_masc1_index == -1 && clicked_poly1_index == -1)
        {
            errors.push_back("[Error] :  No .obj file is selected for 'Body 1'.");
        }
        if(obj_checkbox && clicked_masc2_index == -1 && clicked_poly2_index == -1)
        {
            errors.push_back("[Error] :  No .obj file is selected for 'Body 2'.");
        }

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
            errors.push_back("[Error] :  Invalid set of 'Epoch','Duration','Step'.");
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

        //Tolerance error (less than zero).
        if (tol <= 0.0)
        {
            errors.push_back("[Error] :  Invalid tolerance 'tol'.");
        }

        return errors;
    }

    outputs propagate()
    {
        if (ell_checkbox)
        {
            dvec3 semiaxes1 = {a1,b1,c1};
            dvec3 semiaxes2 = {a2,b2,c2};

            if (ord2_checkbox)
            {
                dtens J1 = ell_integrals(M1, semiaxes1, 2);
                dtens J2 = ell_integrals(M2, semiaxes2, 2);
            }
            else if (ord3_checkbox)
            {
                dtens J1 = ell_integrals(M1, semiaxes1, 3);
                dtens J2 = ell_integrals(M2, semiaxes2, 3);
            }
            else if (ord4_checkbox)
            {
                dtens J1 = ell_integrals(M1, semiaxes1, 4);
                dtens J2 = ell_integrals(M2, semiaxes2, 4);
            }
            else
            {
                //add the grid_reso as parameter to the gui
                dmatnx3 masc1 = fill_ell_with_masc(semiaxes1, ivec3{15,15,15});
                dmatnx3 masc2 = fill_ell_with_masc(semiaxes2, ivec3{15,15,15});
            }
        }
        else //obj_checkbox
        {
            //body 1
            if (clicked_masc1_index != -1) //then the user clicked existing mascons model
            {
                dmatnx3 masc1 = loadobjv(obj_path1.c_str());
            }
            else //the user clicked polyhedron model
            {
                dmatnx3 verts;
                imatnx3 faces;
                loadobjvf(obj_path1.c_str(), verts,faces);
                dmatnx3 masc1 = fill_poly_with_masc(verts,faces, ivec3{15,15,15});
                correct_masc_com(masc1);
                correct_masc_inertia(M1, masc1);
            }
        }
        
        /*
        ell_ell_ord2
        ell_ell_ord3
        ell_ell_ord4
        dvec3 semiaxis1 = {a1,b1,c1};
        dvec3 semiaxis2 = {a2,b2,c2};
        dvec3 r   = {relx, rely, relz};
        dvec3 v   = {0.0, 0.00017421523858789, 0.0};
        dvec4 q1  = {1.0, 0.0, 0.0, 0.0};
        dvec3 w1i = {0.0, 0.0, 0.000772269580528465};
        dvec4 q2  = {1.0, 0.0, 0.0, 0.0};
        dvec3 w2i = {0.0, 0.0, 0.000146399360157891};
        */
        //unsigned steps = integrate_adaptive(make_controlled(tol, tol, rkf78()), odes, state, epoch, dur, step, observe);

        //return outs;
    }
    
};

#endif
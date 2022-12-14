#ifndef INPUTS_HPP
#define INPUTS_HPP

#include<cstdio>

class inputs
{
public:

    char simname[51] = "";

    bool ell_checkbox = false;
    bool pressed_ell_ok = false;

    double a1 = 0.0, b1 = 0.0, c1 = 0.0;
    double a2 = 0.0, b2 = 0.0, c2 = 0.0;

    bool obj_checkbox = false;
    int refer_to_body = 1;
    int current_v1 = -1, current_vf1 = -1, current_vfn1 = -1;
    int current_v2 = -1, current_vf2 = -1, current_vfn2 = -1;
    bool pressed_obj_ok = false;

    double M1 = 0.0, M2 = 0.0;

    double t0 = 0.0, tmax = 0.0, print_step = 0.0;

    const char *cart_kep_var[2] = {"Cartesian ", "Keplerian "};
    int cart_kep_var_choice = 0;

    double relx = 0.0, rely = 0.0, relz = 0.0;
    double relvx = 0.0, relvy = 0.0, relvz = 0.0;

    double rela = 0.0, rele = 0.0, reli = 0.0;
    double relOm = 0.0, relw = 0.0, relM = 0.0;

    const char *orient_var[2] = {"Euler angles ", "Quaternions "};
    int orient_var_choice = 0;

    double roll1 = 0.0, pitch1 = 0.0, yaw1 = 0.0;
    double roll2 = 0.0, pitch2 = 0.0, yaw2 = 0.0;

    double q10 = 1.0, q11 = 0.0, q12 = 0.0, q13 = 0.0;
    double q20 = 1.0, q21 = 0.0, q22 = 0.0, q23 = 0.0;

    const char *frame_type[2] = {"Inertial frame", "Body frame"};
    int frame_type_choice = 0;

    double w1x = 0.0, w1y = 0.0, w1z = 0.0;
    double w2x = 0.0, w2y = 0.0, w2z = 0.0;

    double w11 = 0.0, w12 = 0.0, w13 = 0.0;
    double w21 = 0.0, w22 = 0.0, w23 = 0.0;

    const char *integ_method[4] = {"Runge-Kutta-Fehlberg 78 ", "Bulirsch-Stoer", "Dormand-Prince 5 ", "Runge-Kutta 4 (explicit)"};
    int integ_method_choice = 0;
    double tol = 1e-9;

    bool detect_binary_collision = false;

    bool playback_video = false;

    bool export_time = true;
    bool export_rel_pos_vel = true;
    bool export_ang = true;
    bool export_quat = true;
    bool export_wi = true;
    bool export_wb = true;
    bool export_ener_mom = true;
    bool export_rel_kep = true;

    bool pressed_run = false;

    
};

#endif
/* This class handles the rendering logic of the left panel (properties) in the gui. */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include<cstdio>
#include<cmath>
#include<cstring>
#include<vector>
#include<filesystem>
#include<atomic>
#include<string>

#include"../imgui/imgui.h"

#include"constants.h"
#include"linalg.h"
#include"conversion.h"
#include"polyhedron.h"
#include"console.h"
#include"file.h"

class properties
{
public:
    //The following members are in the same order that u will encounter them in the gui.

    char sim_name[128]; //Simulation name text field (imgui shall stop the input characters to 127 to prevent stack overflow).
    
    bool ell_checkbox; //Ellipsoids checkbox state.
    dvec3 semiaxes1, semiaxes2; //Ellipsoids 'a1', 'b1', 'c1', 'a2', 'b2', 'c2' double fields.
    bool ell_clicked_ok; //Ellipsoids 'OK' button.

    bool obj_checkbox; //.obj file checkbox state.
    std::string obj1_path, obj2_path; //Relative paths to the 2 .obj shape models.
    bool obj_clicked_ok; //.obj 'OK' button.

    bool ord2_checkbox, ord3_checkbox, ord4_checkbox; //Potential expansion order available checkboxes. It corresponds to both the mutual potential of the binary and the solo potential of the spacecraft.

    double M1, M2; //Masses 'M1', 'M2' double fields.

    enum
    {
        RKF78_FIXED,
        RKF78_ADAPTIVE,
        BSTOER_ADAPTIVE,
        ABM5_FIXED
    } integration_method;
    char epoch[64]; //"Epoch" text field (calendar).
    double epoch_jd; //Julian day corresponding to the calendar epoch. Think of it like a hash.
    double dur; //'Duration' double field.
    double step; //'Step' double field (if a fixed-step method is chosen).
    double target_error; //'Target error' double field (if an adaptive-step method is chosen).

    enum
    {
        CARTESIAN_MUT,
        KEPLERIAN_MUT
    } pos_vel_mut_var;
    dvec6 cart_mut; //Binary's mutual Cartesian elements 'x', 'y', 'z', 'υx', 'υy', 'υz'.
    dvec6 kep_mut;  //Binary's mutual Keplerian elements 'a', 'e', 'i', 'Ω', 'ω', 'M'.

    enum
    {
        EULER_XYZ,
        QUATERNION
    } orient_var;
    dvec3 rpy1, rpy2; //'roll 1', 'pitch 1', 'yaw 1', 'roll 2', 'pitch 2', 'yaw 2' double fields.
    dvec4 q1, q2; //'q10', 'q11', 'q12', 'q13', 'q20', 'q21', 'q22', 'q23' double fields.

    enum
    {
        ANGVEL_HELIO,
        ANGVEL_BODY
    } angvel_frame;
    dvec3 w1i, w2i; //'ω1x', 'ω1y', 'ω1z', 'ω2x', 'ω2y', 'ω2z' double fields.
    dvec3 w1b, w2b; //'ω11', 'ω12', 'ω13', 'ω21', 'ω22', 'ω23' double fields.

    enum
    {
        CARTESIAN_COM_HELIO,
        KEPLERIAN_COM_HELIO
    } pos_vel_com_var;
    dvec6 cart_com_helio; //Binary's COM Cartesian elements 'x', 'y', 'z', 'υx', 'υy', 'υz' in Heliocentric frame.
    dvec6 kep_com_helio;  //Binary's COM Keplerian elements'a', 'e', 'i', 'Ω', 'ω', 'M' in Heliocentric frame.

    bool sun_gravity; //Assume Sun's gravity choice.

    bool collision_no, collision_spheres; //Collision criterion to apply in the simulation.

    bool impactors_checkbox; //'Kinetic impactors' checkbox state.
    double mD1, mD2; //Impactor's 'm' double field.
    dvec3 vD1, vD2; //Impactor's 'υx', 'υy', 'υz' double fields.
    double beta1, beta2; //Ejecta parameter 'β' double field.
    double tD1, tD2; //Impact time 't' double field.
    bool impactors_clicked_ok; //Impactors 'OK' button.

    bool spacecraft_checkbox; //'Spacecraft orbiter' checkbox state.
    double sp_mass; //Spacecraft mass 'm'.
    enum
    {
        CARTESIAN_SP_COM,
        CARTESIAN_SP_COM1,
        CARTESIAN_SP_COM2,
        KEPLERIAN_SP_COM,
        KEPLERIAN_SP_COM1,
        KEPLERIAN_SP_COM2
    } pos_vel_sp_var;
    dvec6 cart_sp_com, cart_sp_com1, cart_sp_com2; //Spacecraft's Cartesian elements 'x', 'y', 'z' and 'υx', 'υy', 'υz' in either COM, COM1 or COM2 frame.
    dvec6 kep_sp_com, kep_sp_com1, kep_sp_com2; //Spacecraft's Keplerian elements 'a', 'e', 'i', 'Ω', 'ω', 'M' double fields in either COM, COM1 or COM2 frame.
    bool sp_is_rigidbody_checkbox;
    bool sp_ell_checkbox;
    dvec3 sp_semiaxes;
    bool sp_ell_clicked_ok;
    bool sp_obj_checkbox;
    std::string sp_obj_path;
    bool sp_obj_clicked_ok;
    enum
    {
        EULER_XYZ_SP,
        QUATERNION_SP
    } sp_orient_var;
    dvec3 rpy_sp;
    dvec4 qsp;
    enum
    {
        ANGVEL_SP_HELIO,
        ANGVEL_SP_BODY
    } sp_angvel_frame;
    dvec3 wi_sp, wb_sp;
    bool srp_checkbox; //'Account for SRP' checkbox state.
    double sp_refl, sp_area; //Spacecraft's 'ρ', 'A' double fields.
    bool srp_shadow_checkbox; //'Account for shadow' checkbox state.
    bool spacecraft_clicked_ok; //Spacecraft's 'OK' button.

    bool run_pressed; //Whether or not the 'Run' button has been pressed.
    bool abort_pressed; //Whether or not the 'Abort' button has been pressed.

    polyhedron poly1, poly2, poly_sp; //Polyhedra instances.

    properties() : sim_name(""),
                   ell_checkbox(false),
                   semiaxes1({0.0,0.0,0.0}),
                   semiaxes2({0.0,0.0,0.0}),
                   ell_clicked_ok(false),
                   obj_checkbox(false),
                   obj1_path(""),
                   obj2_path(""),
                   obj_clicked_ok(false),
                   ord2_checkbox(false),
                   ord3_checkbox(false),
                   ord4_checkbox(false),
                   M1(0.0),
                   M2(0.0),
                   integration_method(RKF78_FIXED),
                   epoch("2000-01-01T12:00:00"),
                   epoch_jd(JD_J2000),
                   dur(0.0),
                   step(0.0),
                   target_error(1e-12),
                   pos_vel_mut_var(CARTESIAN_MUT),
                   cart_mut({0.0,0.0,0.0,0.0,0.0,0.0}),
                   kep_mut({0.0,0.0,0.0,0.0,0.0,0.0}),
                   orient_var(EULER_XYZ),
                   rpy1({0.0,0.0,0.0}),
                   rpy2({0.0,0.0,0.0}),
                   q1({1.0,0.0,0.0,0.0}),
                   q2({1.0,0.0,0.0,0.0}),
                   angvel_frame(ANGVEL_HELIO),
                   w1i({0.0,0.0,0.0}),
                   w2i({0.0,0.0,0.0}),
                   w1b({0.0,0.0,0.0}),
                   w2b({0.0,0.0,0.0}),
                   pos_vel_com_var(CARTESIAN_COM_HELIO),
                   cart_com_helio({0.0,0.0,0.0,0.0,0.0,0.0}),
                   kep_com_helio({0.0,0.0,0.0,0.0,0.0,0.0}),
                   sun_gravity(false),
                   collision_no(false),
                   collision_spheres(false),
                   impactors_checkbox(false),
                   mD1(0.0),
                   mD2(0.0),
                   vD1({0.0,0.0,0.0}),
                   vD2({0.0,0.0,0.0}),
                   beta1(0.0),
                   beta2(0.0),
                   tD1(0.0),
                   tD2(0.0),
                   impactors_clicked_ok(false),
                   spacecraft_checkbox(false),
                   sp_mass(0.0),
                   pos_vel_sp_var(CARTESIAN_SP_COM),
                   cart_sp_com({0.0,0.0,0.0,0.0,0.0,0.0}),
                   cart_sp_com1({0.0,0.0,0.0,0.0,0.0,0.0}),
                   cart_sp_com2({0.0,0.0,0.0,0.0,0.0,0.0}),
                   kep_sp_com({0.0,0.0,0.0,0.0,0.0,0.0}),
                   kep_sp_com1({0.0,0.0,0.0,0.0,0.0,0.0}),
                   kep_sp_com2({0.0,0.0,0.0,0.0,0.0,0.0}),
                   sp_is_rigidbody_checkbox(false),
                   sp_ell_checkbox(false),
                   sp_semiaxes({0.0,0.0,0.0}),
                   sp_ell_clicked_ok(false),
                   sp_obj_checkbox(false),
                   sp_obj_path(""),
                   sp_obj_clicked_ok(false),
                   sp_orient_var(EULER_XYZ_SP),
                   rpy_sp({0.0,0.0,0.0}),
                   qsp({1.0,0.0,0.0,0.0}),
                   sp_angvel_frame(ANGVEL_SP_HELIO),
                   wi_sp({0.0,0.0,0.0}),
                   wb_sp({0.0,0.0,0.0}),
                   srp_checkbox(false),
                   sp_refl(0.0),
                   sp_area(0.0),
                   srp_shadow_checkbox(false),
                   spacecraft_clicked_ok(false),
                   run_pressed(false),
                   abort_pressed(false),
                   poly1(),
                   poly2(),
                   poly_sp()
    { }

    //This function loads to the gui the user-chosen properties file.
    void import_file(const char *path, console &cons)
    {
        *this = properties{}; //Reset the inputs. This command basically re-runs the constructor.

        FILE *fp = fopen(path,"r");
        if (!fp) //This should never happen. It is already verified by topbar that the file exists, otherwise it would not appear in the gui.
        {
            cons.print("[Error] : The selected properties file could not be opened.\n");
            return;
        }

        //Parse the simulation name.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", sim_name);

        char buffer[128];

        //Parse the shape models.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Ellipsoids") == 0)
        {
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &semiaxes1[i]);
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &semiaxes2[i]);
            ell_checkbox = ell_clicked_ok = true;
        }
        else //".obj files"
        {
            if (find_assignment_operator(fp))
            {   
                fscanf(fp, " \"%127[^\"]\"", buffer);
                obj1_path = buffer;
            }
            if (find_assignment_operator(fp))
            {
                fscanf(fp, " \"%127[^\"]\"", buffer);
                obj2_path = buffer;
            }
            obj_checkbox = obj_clicked_ok = true;
        }

        //Parse the potential expansion order.
        int Vord;
        if (find_assignment_operator(fp)) fscanf(fp, "%d", &Vord);
        if (Vord <= 2)
            ord2_checkbox = true;
        else if (Vord == 3)
            ord3_checkbox = true;
        else
            ord4_checkbox = true;

        //Parse the masses.
        if (find_assignment_operator(fp)) fscanf(fp, "%lf", &M1);
        if (find_assignment_operator(fp)) fscanf(fp, "%lf", &M2);

        //Parse the numerical method of the ODEs integration.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "RKF78 (fixed)") == 0)
        {
            integration_method = RKF78_FIXED;
            if (find_assignment_operator(fp)) fscanf(fp, " \"%63[^\"]\"", epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &step);
        }
        else if (strcmp(buffer, "RKF78 (adaptive)") == 0)
        {
            integration_method = RKF78_ADAPTIVE;
            if (find_assignment_operator(fp)) fscanf(fp, " \"%63[^\"]\"", epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &target_error);
        }
        else if (strcmp(buffer, "BStoer (adaptive)") == 0)
        {
            integration_method = BSTOER_ADAPTIVE;
            if (find_assignment_operator(fp)) fscanf(fp, " \"%63[^\"]\"", epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &target_error);
        }
        else
        {
            integration_method = ABM5_FIXED;
            if (find_assignment_operator(fp)) fscanf(fp, " \"%63[^\"]\"", epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &step);
        }

        //Parse the binary's initial mutual position/velocity.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Cartesian") == 0)
        {
            pos_vel_mut_var = CARTESIAN_MUT;
            for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart_mut[i]);
        }
        else
        {
            pos_vel_mut_var = KEPLERIAN_MUT;
            for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep_mut[i]);
        }

        //Parse the bodies' initial orientations.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Euler angles (XYZ)") == 0)
        {
            orient_var = EULER_XYZ;
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &rpy1[i]);
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &rpy2[i]);
        }
        else
        {
            orient_var = QUATERNION;
            for (int i = 0; i < 4; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &q1[i]);
            for (int i = 0; i < 4; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &q2[i]);
        }

        //Parse the binary's initial angular velocity.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Heliocentric (inertial)") == 0)
        {
            angvel_frame = ANGVEL_HELIO;
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w1i[i]);
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w2i[i]);
        }
        else
        {
            angvel_frame = ANGVEL_BODY;
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w1b[i]);
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w2b[i]);
        }

        //Parse the COM initial Heliocentric state.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Cartesian") == 0)
        {
            pos_vel_com_var = CARTESIAN_COM_HELIO;
            for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart_com_helio[i]);
        }
        else
        {
            pos_vel_com_var = KEPLERIAN_COM_HELIO;
            for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep_com_helio[i]);
        }

        //Parse the Sun's gravity assumption.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Yes") == 0)
            sun_gravity = true;

        //Parse the collision shapes.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "No collision") == 0)
            collision_no = true;
        else
            collision_spheres = true;

        //Parse the kinetic impactors.
        if (find_assignment_operator(fp))
        {
            fscanf(fp, " \"%127[^\"]\"", buffer);
            if (strcmp(buffer, "Yes") == 0)
            {
                if (find_assignment_operator(fp)) fscanf(fp, "%lf", &mD1);
                for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &vD1[i]);
                if (find_assignment_operator(fp)) fscanf(fp, "%lf", &beta1);
                if (find_assignment_operator(fp)) fscanf(fp, "%lf", &tD1);

                if (find_assignment_operator(fp)) fscanf(fp, "%lf",&mD2);
                for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &vD2[i]);
                if (find_assignment_operator(fp)) fscanf(fp, "%lf", &beta2);
                if (find_assignment_operator(fp)) fscanf(fp, "%lf", &tD2);

                impactors_checkbox = impactors_clicked_ok = true;
            }
        }

        //Parse the spacecraft's state.
        if (find_assignment_operator(fp))
        {
            fscanf(fp, " \"%127[^\"]\"", buffer);
            if (strcmp(buffer, "Yes") == 0)
            {
                //Mass.
                if (find_assignment_operator(fp)) fscanf(fp, "%lf", &sp_mass);

                //Initial position/velocity.
                if (find_assignment_operator(fp))
                {
                    fscanf(fp, " \"%127[^\"]\"", buffer);
                    if (strcmp(buffer, "Cartesian (binary COM)") == 0)
                    {
                        pos_vel_sp_var = CARTESIAN_SP_COM;
                        for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart_sp_com[i]);
                    }
                    else if (strcmp(buffer, "Cartesian (body 1)") == 0)
                    {
                        pos_vel_sp_var = CARTESIAN_SP_COM1;
                        for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart_sp_com1[i]);   
                    }
                    else if (strcmp(buffer, "Cartesian (body 2)") == 0)
                    {
                        pos_vel_sp_var = CARTESIAN_SP_COM2;
                        for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart_sp_com2[i]);   
                    }
                    else if (strcmp(buffer, "Keplerian (binary COM)") == 0)
                    {
                        pos_vel_sp_var = KEPLERIAN_SP_COM;
                        for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep_sp_com[i]);
                    }
                    else if (strcmp(buffer, "Keplerian (body 1)") == 0)
                    {
                        pos_vel_sp_var = KEPLERIAN_SP_COM1;
                        for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep_sp_com1[i]);
                    }
                    else //pos_vel_sp_var == KEPLERIAN_SP_COM2;
                    {
                        pos_vel_sp_var = KEPLERIAN_SP_COM2;
                        for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep_sp_com2[i]);
                    }
                }

                //Is rigid body settings.
                if (find_assignment_operator(fp))
                {
                    fscanf(fp, " \"%127[^\"]\"", buffer);
                    if (strcmp(buffer, "Yes") == 0)
                    {
                        //Parse spacecraft's shape model.
                        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
                        if (strcmp(buffer, "Ellipsoid") == 0)
                        {
                            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &sp_semiaxes[i]);
                            sp_ell_checkbox = sp_ell_clicked_ok = true;
                        }
                        else //".obj file"
                        {
                            if (find_assignment_operator(fp))
                            {   
                                fscanf(fp, " \"%127[^\"]\"", buffer);
                                sp_obj_path = buffer;
                            }
                            sp_obj_checkbox = sp_obj_clicked_ok = true;
                        }

                        //Parse spacecraft's initial orientation.
                        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
                        if (strcmp(buffer, "Euler angles (XYZ)") == 0)
                        {
                            sp_orient_var = EULER_XYZ_SP;
                            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &rpy_sp[i]);
                        }
                        else
                        {
                            sp_orient_var = QUATERNION_SP;
                            for (int i = 0; i < 4; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &qsp[i]);
                        }

                        //Parse spacecraft's initial angular velocity.
                        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
                        if (strcmp(buffer, "Heliocentric (inertial)") == 0)
                        {
                            sp_angvel_frame = ANGVEL_SP_HELIO;
                            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &wi_sp[i]);
                        }
                        else
                        {
                            sp_angvel_frame = ANGVEL_SP_BODY;
                            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &wb_sp[i]);
                        }

                        sp_is_rigidbody_checkbox = true;
                    }
                }

                //SRP settings.
                if (find_assignment_operator(fp))
                {
                    fscanf(fp, " \"%127[^\"]\"", buffer);
                    if (strcmp(buffer, "Yes") == 0)
                    {
                        if (find_assignment_operator(fp)) fscanf(fp, "%lf", &sp_refl);
                        if (find_assignment_operator(fp)) fscanf(fp, "%lf", &sp_area);

                        if (find_assignment_operator(fp))
                        {
                            fscanf(fp, " \"%127[^\"]\"", buffer);
                            if (strcmp(buffer, "Yes") == 0)
                                srp_shadow_checkbox = true;
                        }
                        srp_checkbox = true;
                    }
                }

                spacecraft_checkbox = spacecraft_clicked_ok = true;
            }
        }

        fclose(fp);
    }

private:
    //This function receives as input a 'path' to a directory and as a result it returns a vector of paths, corresponding to all the .obj files found inside 'path'.
    std::vector<std::filesystem::path> list_obj_files(const char *path)
    {
        std::vector<std::filesystem::path> paths;
        for (const auto &entry : std::filesystem::directory_iterator(path))
            if (entry.is_regular_file() && entry.path().extension() == ".obj")
                paths.push_back(entry.path().filename());
        return paths;
    }
    
    //This function checks whether or not the user input calendar date follows the expected ISO rules.
    //If so, it returns true and updates the member 'epoch_jd' accordingly.
    bool parse_epoch(const char *str)
    {
        ///////////////////////////////////////////////////////////////////////////
        auto is_digit = [](char x)
        {
            return x >= '0' && x <= '9';
        };

        auto to2 = [&](int i)
        {
            return (str[i]-'0')*10 + (str[i+1]-'0');
        };

        auto is_leap = [](int y)
        {
            return (y%4 == 0 && y%100 != 0) || (y%400 == 0);
        };

        auto dim = [&](int y, int mo)
        {
            static const int d[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
            return (mo == 2) ? d[1] + (is_leap(y) ? 1 : 0) : d[mo - 1];
        };
        ///////////////////////////////////////////////////////////////////////////

        //Rule 1 : verify that the date is not empty.
        if (!str || str[0] == '\0')
            return false;

        //Rule 2 : measure exactly 19 characters ("YYYY-MM-DDTHH:MM:SS").
        if (std::strlen(str) != 19)
            return false;

        //Rule 3 : enforce the use of the following separators.
        if (str[4] != '-' || str[7] != '-' || str[10] != 'T' || str[13] != ':' || str[16] != ':')
            return false;

        //Rule 4 : ensure that digits sit in the correct position of the string.
        for (int i : {0,1,2,3,5,6,8,9,11,12,14,15,17,18})
            if (!is_digit(str[i]))
                return false;

        int year   = (str[0]-'0')*1000 + (str[1]-'0')*100 + (str[2]-'0')*10 + (str[3]-'0');
        int month  = to2(5);
        int day    = to2(8);
        int hour   = to2(11);
        int minute = to2(14);
        int second = to2(17);

        //Rule 5 : do not allow limits to be exeeded.
        if (month < 1 || month > 12)
            return false;
        if (hour < 0 || hour > 23)
            return false;
        if (minute < 0 || minute > 59)
            return false;
        if (second < 0 || second > 59)
            return false;
        if (day < 1 || day > dim(year, month))
            return false;

        epoch_jd = calendar2jd(year, month, day, hour, minute, second);
        return true;
    }

    //This function automates common double inputs via the keyboard.
    void input_double(const char *left_str, const float item_width, const float x_start, int &id, const char *right_str, double &variable)
    {
        ImGui::Text(left_str);
        ImGui::SameLine();
        ImGui::SetCursorPosX(x_start);
        ImGui::PushItemWidth(item_width);
            ImGui::PushID(id++);
                ImGui::InputDouble(right_str, &variable, 0.0, 0.0, "%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
    }

    void render_impactor_menu(double &mD, dvec3 &vD, double &beta, double &tD, int &id)
    {
        ImGui::Text("Mass (dry + fuel)");
        input_double("m ", 100.0f*SCX, 40.0f*SCX, id, "[kg]", mD);
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
        ImGui::Text("Velocity (relative to body)");
        input_double("υx ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", vD[0]);
        input_double("υy ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", vD[1]);
        input_double("υz ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", vD[2]);
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
        ImGui::Text("Momentum enhancement factor (ejecta)");
        input_double("β ", 100.0f*SCX, 40.0f*SCX, id, "[  ]", beta);
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
        ImGui::Text("Relative day of impact (post-epoch)");
        input_double("t ", 100.0f*SCX, 40.0f*SCX, id, "[days]", tD);
    }

public:
    //This member function processes all the user inputs and checks if they are valid, assuming some rules, defined by me.
    //If at least 1 rule is violated, the corresponding error(s) are displayed on the console and the simulation will not run.
    bool validate(console &cons)
    {   
        //Rule : Simulation folder wrong naming (empty, pure spaces, begin with space, illegal characters).
        std::string name = sim_name;
        if ( (name.empty()) || (name.find_first_not_of(' ') == std::string::npos) || (name[0] == ' ') || (name.find_first_of("<>:\"/\\|?*") != std::string::npos) )
            {cons.print("[Error] : 'Simulation name' is invalid.\n"); return false;}

        //Rule : Shape model checkboxes (at least one must be checked when the 'Run' button has been pressed).
        if (!ell_checkbox && !obj_checkbox)
            {cons.print("[Error] : Neither 'Ellipsoids', nor '.obj files' is selected for determining the shapes.\n"); return false;}

        //Rule : 'OK' button in the Elliposid parameters window (it must be clicked so that the parameters are taken into account).
        if (ell_checkbox && !ell_clicked_ok)
            {cons.print("[Error] : 'OK' button must be pressed in the 'Ellipsoid parameters' window.\n"); return false;}

        //Rule : 'OK' button in the '.obj files' window (it must be clicked so that the .obj files are taken into account).
        if (obj_checkbox && !obj_clicked_ok)
            {cons.print("[Error] : 'OK' button must be pressed in the '.obj files' window.\n"); return false;}

        //Rule : Mutual potential checkboxes (at least one must be checked).
        if (!ord2_checkbox && !ord3_checkbox && !ord4_checkbox)
            {cons.print("[Error] : Neither 'Order 2', nor 'Order 3', nor 'Order 4' mutual potential is selected.\n"); return false;}

        //Rule : Masses (both M1 and M2 must be > 0).
        if (M1 <= 0.0 || M2 <= 0.0)
            {cons.print("[Error] : 'M1', 'M2' must be positive.\n"); return false;}

        //Rule : 'Epoch' must be in a meaningful calendar format.
        if (!parse_epoch(epoch))
            {cons.print("[Error] : Invalid 'Epoch' format.\n"); return false;}

        //Rule : Time parameters ('Duration' must be >= 0, 'Step' must be <= 'Duration' and 'Target error' must be > 0).
        if (integration_method == RKF78_FIXED || integration_method == ABM5_FIXED)
        {
            if (!(dur > 0.0 && step > 0.0 && step <= dur))
                {cons.print("[Error] : Invalid set of 'Duration', 'Step'.\n"); return false;}
        }
        else
        {
            if (!(dur > 0.0 && target_error > 0.0))
                {cons.print("[Error] : Invalid set of 'Duration', 'Target error'.\n"); return false;}
        }

        //Rule : Binary's mutual position/velocity.
        if (pos_vel_mut_var == CARTESIAN_MUT)
        {
            //Distance must be > 0.
            if (length(dvec3{cart_mut[0], cart_mut[1], cart_mut[2]}) <= 0.0)
                {cons.print("[Error] : Invalid set of 'x', 'y', 'z' (mutual distance must be positive).\n"); return false;}
        }
        else //pos_vel_mut_var == KEPLERIAN_MUT
        {
            //Mutual 'a' must be nonzero, 'e' must be in [0,1)U(1,inf).
            if (fabs(kep_mut[0]) <= 1e-15)
                {cons.print("[Error] : Binary's mutual semi-major axis 'a' must be nonzero.\n"); return false;}
            if (kep_mut[1] < 0.0 || fabs(kep_mut[1] - 1.0) <= 1e-15)
                {cons.print("[Error] : Binary's mutual eccentricity 'e' must be in [0,1)U(1,inf).\n"); return false;}
        }

        //Rule : Both quaternions must be nonzero.
        if (orient_var == QUATERNION)
        {
            if (length(q1) <= 1e-15)
                {cons.print("[Error] : Quaternion 1 ('q10', 'q11', 'q12', 'q13') must be nonzero.\n"); return false;}
            else //Normalize it no matter what.
                q1 = quat2unit(q1); //This correction will be visible in the gui.

            //Same for q2 :
            if (length(q2) <= 1e-15)
                {cons.print("[Error] : Quaternion 2 ('q20', 'q21', 'q22', 'q23') must be nonzero.\n"); return false;}
            else
                q2 = quat2unit(q2);
        }

        //Rule : The binary system must not be too close to the Sun.
        if (pos_vel_com_var == CARTESIAN_COM_HELIO)
        {
            if (length(dvec3{cart_com_helio[0],cart_com_helio[1],cart_com_helio[2]}) < MIN_SUN_BODY_DIST)
                {cons.print("[Error] : Binary's Heliocentric COM is too close to the Sun. Increase heliocentric distance.\n"); return false;}
        }
        else //pos_vel_com_var == KEPLERIAN_COM_HELIO
        {
            //COM's Heliocentric 'a' must be nonzero, 'e' must be in [0,1)U(1,inf).
            if (fabs(kep_com_helio[0]) <= 1e-15)
                {cons.print("[Error] : Binary's Heliocentric semi-major axis 'a' must be nonzero.\n"); return false;}
            if (kep_com_helio[1] < 0.0 || fabs(kep_com_helio[1] - 1.0) <= 1e-15)
                {cons.print("[Error] : Binary's Heliocentric eccentricity 'e' must be in [0,1)U(1,inf).\n"); return false;}
            
            //So if no return; statement is called, it means that Heliocentric Keplerian elements were chosen and they are valid according to the above rules.
            //Therefore do the following :
            dvec6 temp_kep_com_helio  = {kep_com_helio[0]*AU2KM, kep_com_helio[1], kep_com_helio[2]*PI/180.0, kep_com_helio[3]*PI/180.0, kep_com_helio[4]*PI/180.0, kep_com_helio[5]*PI/180.0};
            dvec6 temp_cart_com_helio = kep2cart(temp_kep_com_helio, G*MSUN);
            if (length(dvec3{temp_cart_com_helio[0], temp_cart_com_helio[1], temp_cart_com_helio[2]}) < MIN_SUN_BODY_DIST*AU2KM)
                {cons.print("[Error] : Binary COM is too close to the Sun. Increase heliocentric distance.\n"); return false;}   
        }

        //Rule : Collision criterion (at least one must be checked).
        if (!collision_no && !collision_spheres)
            {cons.print("[Error] : At least one collision criterion must be selected.\n"); return false;}

        //Rules regarding the kinetic impactors :
        if (impactors_checkbox)
        {
            //Rule : Impactors' parameters (masses must be >= 0).
            if (mD1 < 0.0 || mD2 < 0.0)
                {cons.print("[Error] : Both impactors' masses, 'm1' and 'm2' must be non negative.\n"); return false;}

            //Rule : Times of impacts must range in the simulated time range.
            if (tD1 < 0.0 || tD1 > dur || tD2 < 0.0 || tD2 > dur)
                {cons.print("[Error] : Impact times must range in [0, Duration].\n"); return false;}

            //Rule : 'OK' button in the Impactors' parameters window (it must be clicked so that the parameters are taken into account).
            if (!impactors_clicked_ok)
                {cons.print("[Error] : 'OK' button must be pressed in the 'Impactor parameters' window.\n"); return false;}
        }

        //Rules regarding the spacecraft orbiter :
        if (spacecraft_checkbox)
        {
            if (sp_mass < 1e-15)
                    {cons.print("[Error] : Spacecraft's mass 'm' must positive.\n"); return false;}

            //Rule : Initial position/velocity, must be valid (similar to binary's mutual state logic).
            if (pos_vel_sp_var == KEPLERIAN_SP_COM)
            {
                if (fabs(kep_sp_com[0]) <= 1e-15)
                    {cons.print("[Error] : Spacecraft's semi-major axis 'a' must be nonzero.\n"); return false;}
                if (kep_sp_com[1] < 0.0 || fabs(kep_sp_com[1] - 1.0) <= 1e-15)
                    {cons.print("[Error] : Spacecraft's eccentricity 'e' must be in [0,1)U(1,inf).\n"); return false;}
            }
            else if (pos_vel_sp_var == KEPLERIAN_SP_COM1)
            {
                if (fabs(kep_sp_com1[0]) <= 1e-15)
                    {cons.print("[Error] : Spacecraft's semi-major axis 'a' must be nonzero.\n"); return false;}
                if (kep_sp_com1[1] < 0.0 || fabs(kep_sp_com1[1] - 1.0) <= 1e-15)
                    {cons.print("[Error] : Spacecraft's eccentricity 'e' must be in [0,1)U(1,inf).\n"); return false;}
            }
            else if (pos_vel_sp_var == KEPLERIAN_SP_COM2)
            {
                if (fabs(kep_sp_com2[0]) <= 1e-15)
                    {cons.print("[Error] : Spacecraft's semi-major axis 'a' must be nonzero.\n"); return false;}
                if (kep_sp_com2[1] < 0.0 || fabs(kep_sp_com2[1] - 1.0) <= 1e-15)
                    {cons.print("[Error] : Spacecraft's eccentricity 'e' must be in [0,1)U(1,inf).\n"); return false;}
            }

            if (sp_is_rigidbody_checkbox)
            {
                //Rule : Shape model checkbox (at least one must be checked when the 'Run' button has been pressed).
                if (!sp_ell_checkbox && !sp_obj_checkbox)
                    {cons.print("[Error] : Neither 'Ellipsoid', nor '.obj file' is selected for determining the spacecraft shape.\n"); return false;}

                //Rule : 'OK' button in the Elliposid parameters window (it must be clicked so that the parameters are taken into account).
                if (sp_ell_checkbox && !sp_ell_clicked_ok)
                    {cons.print("[Error] : 'OK' button must be pressed in the 'Ellipsoid parameters' window.\n"); return false;}

                //Rule : 'OK' button in the '.obj file' window (it must be clicked so that the spacecraft .obj file is taken into account).
                if (sp_obj_checkbox && !sp_obj_clicked_ok)
                    {cons.print("[Error] : 'OK' button must be pressed in the '.obj file' window.\n"); return false;}

                //Rule : Spacecraft's quaternion must be nonzero.
                if (sp_orient_var == QUATERNION_SP)
                {
                    if (length(qsp) <= 1e-15)
                        {cons.print("[Error] : Spacecraft quaternion ('q0', 'q1', 'q2', 'q3') must be nonzero.\n"); return false;}
                    else //Normalize it no matter what.
                        qsp = quat2unit(qsp); //This correction will be visible in the gui.
                }
            }
            
            //Rule : SRP inputs must be valid.
            if (srp_checkbox)
            {
                if (sp_refl < 0.0 || sp_refl > 1.0 + 1e-15)
                    {cons.print("[Error] : Spacecraft's reflectance 'ρ' must range in [0,1].\n"); return false;}
                if (sp_area < 1e-15)
                    {cons.print("[Error] : Spacecraft's area 'A' must positive.\n"); return false;}
            }
            //Rule : 'OK' button must be clicked in the end.
            if (!spacecraft_clicked_ok)
                {cons.print("[Error] : 'OK' button must be pressed in the 'Spacecraft's state' window.\n"); return false;}
        }

        //Rule : Binary ellipsoid semiaxes (all semiaxes must be > 0).
        if (ell_checkbox)
        {
            if (semiaxes1[0] <= 0.0 || semiaxes1[1] <= 0.0 || semiaxes1[2] <= 0.0)
                {cons.print("[Error] : 'a1', 'b1', 'c1' must be positive numbers.\n"); return false;}
            if (semiaxes2[0] <= 0.0 || semiaxes2[1] <= 0.0 || semiaxes2[2] <= 0.0)
                {cons.print("[Error] : 'a2', 'b2', 'c2' must be positive numbers.\n"); return false;}

            if (semiaxes1[0] > 0.0 && semiaxes1[1] > 0.0 && semiaxes1[2] > 0.0 &&
                semiaxes2[0] > 0.0 && semiaxes2[1] > 0.0 && semiaxes2[2] > 0.0)
            {
                poly1.load_obj_file("../obj/polyhedra/uvsphere64x64_R1km.obj");
                poly2 = poly1;
                poly1.set_scale(semiaxes1);
                poly1.gen_norms();
                poly2.set_scale(semiaxes2);
                poly2.gen_norms();
            }
        }

        //Rule : Spacecraft ellipsoid semiaxes (all semiaxes must be > 0).
        if (sp_ell_checkbox)
        {
            if (sp_semiaxes[0] <= 0.0 || sp_semiaxes[1] <= 0.0 || sp_semiaxes[2] <= 0.0)
                {cons.print("[Error] : Spacecraft's 'a', 'b', 'c' must be positive numbers.\n"); return false;}

            if (sp_semiaxes[0] > 0.0 && sp_semiaxes[1] > 0.0 && sp_semiaxes[2] > 0.0)
            {
                poly_sp.load_obj_file("../obj/polyhedra/uvsphere64x64_R1km.obj");
                poly_sp.set_scale(sp_semiaxes);
                poly_sp.gen_norms();
            }
        }

        //Rule : Regarding binary .obj files, at least one .obj file per body must be selected. Also the polyhedra must be closed manifold geometries.
        if (obj_checkbox)
        {   
            if (obj1_path.empty())
                {cons.print("[Error] : No .obj file is selected for 'Body 1'.\n"); return false;}
            else
            {
                cons.print("[Polyhedron] : Loading .obj file 1... ");
                if (poly1.is_kimin_valid_obj(("../obj/polyhedra/" + obj1_path).c_str()))
                {
                    poly1.load_obj_file(("../obj/polyhedra/" + obj1_path).c_str());
                    if (!poly1.is_closed_manifold())
                        {cons.print("< Invalid .obj file for 'Body 1' (non closed manifold). >\n"); return false;}
                    else
                        cons.print("Done.\n");
                }
                else
                    {cons.print("< Invalid .obj file for 'Body 1' (it must contain only vertices and faces). >\n"); return false;}
            }
            if (obj2_path.empty())
                {cons.print("[Error] : No .obj file is selected for 'Body 2'.\n"); return false;}
            else
            {
                cons.print("[Polyhedron] : Loading .obj file 2... ");
                if (poly2.is_kimin_valid_obj(("../obj/polyhedra/" + obj2_path).c_str()))
                {
                    poly2.load_obj_file(("../obj/polyhedra/" + obj2_path).c_str());
                    if (!poly2.is_closed_manifold())
                        {cons.print("< Invalid .obj file for 'Body 2' (non closed manifold). >\n"); return false;}
                    else
                        cons.print("Done.\n");
                }
                else
                    {cons.print("< Invalid .obj file for 'Body 2' (it must contain only vertices and faces). >\n"); return false;}
            }
        }

        //Rule : Regarding spacecraft .obj files, at least one .obj file per body must be selected. Also the polyhedra must be closed manifold geometries.
        if (sp_obj_checkbox)
        {   
            if (sp_obj_path.empty())
                {cons.print("[Error] : No .obj file is selected for spacecraft.\n"); return false;}
            else
            {
                cons.print("[Polyhedron] : Loading spacecraft .obj file... ");
                if (poly_sp.is_kimin_valid_obj(("../obj/polyhedra/" + sp_obj_path).c_str()))
                {
                    poly_sp.load_obj_file(("../obj/polyhedra/" + sp_obj_path).c_str());
                    if (!poly_sp.is_closed_manifold())
                        {cons.print("< Invalid .obj file for spacecraft (non closed manifold). >\n"); return false;}
                    else
                        cons.print("Done.\n");
                }
                else
                    {cons.print("< Invalid .obj file for spacecraft (it must contain only vertices and faces). >\n"); return false;}
            }
        }

        return true;
    }

    //This is the function that draws the properties panel and processes the corresponding logic.
    void render(std::atomic<bool> task_is_running, std::atomic<bool> task_was_aborted, std::atomic<float> task_progress)
    {
        const float sx = ImGui::GetIO().DisplaySize.x;
        const float sy = ImGui::GetIO().DisplaySize.y;

        int id = 0; //Hash for imgui widgets of similar type.

        ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetFrameHeight()), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.15f*sx, sy - ImGui::GetFrameHeight()), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties", nullptr);


        //Simulation name text field :
        ImGui::SeparatorText("Simulation name");
        ImGui::PushItemWidth(220.0f*SCX);
            ImGui::InputText("##sim_name", sim_name, IM_ARRAYSIZE(sim_name));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Shape model logic :
        ImGui::SeparatorText("Shape models");
        //Ellipsoid shape logic :
        if (ImGui::Checkbox("Ellipsoids##ell_checkbox", &ell_checkbox) && ell_checkbox)
            ell_clicked_ok = false;
        if (ell_checkbox && !ell_clicked_ok)
        {
            obj_checkbox = false; //Untick the obj checkbox in case it is ticked.

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(0.15f*sx, 0.4f*sy), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters##binary", &ell_checkbox);
            
            ImGui::Text("Body 1 semi - axes");
            input_double("a1 ", 100.0f*SCX, 30.0f*SCX, id, "[km]", semiaxes1[0]);
            input_double("b1 ", 100.0f*SCX, 30.0f*SCX, id, "[km]", semiaxes1[1]);
            input_double("c1 ", 100.0f*SCX, 30.0f*SCX, id, "[km]", semiaxes1[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
            ImGui::Text("Body 2 semi - axes");
            input_double("a2 ", 100.0f*SCX, 30.0f*SCX, id, "[km]", semiaxes2[0]);
            input_double("b2 ", 100.0f*SCX, 30.0f*SCX, id, "[km]", semiaxes2[1]);
            input_double("c2 ", 100.0f*SCX, 30.0f*SCX, id, "[km]", semiaxes2[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
            if (ImGui::Button("OK", ImVec2(50.0f*SCX,30.0f*SCY)))
                ell_clicked_ok = true;

            ImGui::End();
        }
        //.obj shape logic :
        if (ImGui::Checkbox(".obj files##obj_checkbox", &obj_checkbox) && obj_checkbox)
            obj_clicked_ok = false;
        if (obj_checkbox && !obj_clicked_ok)
        {
            ell_checkbox = false; //Untick the ellipsoids checkbox in case it is ticked.

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(0.15f*sx, 0.4f*sy), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj files##binary", &obj_checkbox);

            //Radiobuttons logic : At least one will always be active and to this, (the active one) the loaded obj file will correspond.
            static int obj_refers_to_body = 1; //To which body ('Body 1' or 'Body 2') does the obj file listing refer to (via radiobutton). 'Body 1' is the default choice.
            if (ImGui::RadioButton("Body 1", obj_refers_to_body == 1))
                obj_refers_to_body = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Body 2", obj_refers_to_body == 2))
                obj_refers_to_body = 2;
            ImGui::Dummy(ImVec2(0.0f,5.0f*SCY));

            //File listing and selection logic :
            static std::vector<std::filesystem::path> all_obj_files;
            bool tree_opened = ImGui::TreeNodeEx("Available .obj files", ImGuiTreeNodeFlags_Framed);
            if (ImGui::IsItemToggledOpen() && tree_opened) //Opened >>this<< frame only.
                all_obj_files = list_obj_files("../obj/polyhedra/");
            if (tree_opened)
            {
                for (size_t i = 0; i < all_obj_files.size(); ++i)
                {
                    if (obj_refers_to_body == 1)
                    {
                        bool selected = (all_obj_files[i].string() == obj1_path);
                        if (ImGui::Selectable(all_obj_files[i].string().c_str(), selected))
                        {
                            obj1_path = all_obj_files[i].string();
                            obj_clicked_ok = false;
                        }
                    }
                    else
                    {
                        bool selected = (all_obj_files[i].string() == obj2_path);
                        if (ImGui::Selectable(all_obj_files[i].string().c_str(), selected))
                        {
                            obj2_path = all_obj_files[i].string();
                            obj_clicked_ok = false;
                        }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,20.0f*SCY));
            if (ImGui::Button("OK", ImVec2(50.0f*SCX,30.0f*SCY)))   
                obj_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Potential expansion :
        ImGui::SeparatorText("Potential expansion");
        if (ImGui::Checkbox("Order 2", &ord2_checkbox))
            ord3_checkbox = ord4_checkbox = false;
        if (ImGui::Checkbox("Order 3", &ord3_checkbox))
            ord2_checkbox = ord4_checkbox = false;
        if (ImGui::Checkbox("Order 4", &ord4_checkbox))
            ord2_checkbox = ord3_checkbox = false;
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Masses :
        ImGui::SeparatorText("Masses");
        input_double("M1 ", 150.0f*SCX, 40.0f*SCX, id, "[kg]", M1);
        input_double("M2 ", 150.0f*SCX, 40.0f*SCX, id, "[kg]", M2);
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Integration method and time parameters :
        ImGui::SeparatorText("Numerical integration");
        ImGui::Indent();
        //Method logic :
        ImGui::Text("Method");
        static const char *ode_methods[4] = {"RKF78 (fixed)",
                                             "RKF78 (adaptive)",
                                             "BStoer (adaptive)",
                                             "ABM5 (fixed)"};
        ImGui::PushItemWidth(220.0f*SCX);
            ImGui::PushID(id++);
                ImGui::Combo("  ", (int*)(&integration_method), ode_methods, IM_ARRAYSIZE(ode_methods));
            ImGui::PopID();
        ImGui::PopItemWidth();
        //Calendar logic :
        ImGui::Text("Epoch");
        ImGui::SameLine();
        ImGui::SetCursorPosX(100.0f*SCX);
        ImGui::PushItemWidth(130.0f*SCX);
            ImGui::InputText("##epoch", epoch, IM_ARRAYSIZE(epoch));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("[cal]");
        //Duration and step :
        input_double("Duration ", 130.0f*SCX, 100.0f*SCX, id, "[days]", dur);
        if (integration_method == RKF78_FIXED || integration_method == ABM5_FIXED)
            input_double("Step ", 130.0f*SCX, 100.0f*SCX, id, "[days]", step);
        else
            input_double("Target error ", 130.0f*SCX, 100.0f*SCX, id, "[    ]", target_error);
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
        ImGui::Unindent();


        //Initial state of the binary logic :
        ImGui::SeparatorText("Binary's initial state");
        ImGui::Indent();
        ImGui::Text("Mutual position and velocity");
        static const char *cart_kep_mut_var[2] = {"Cartesian",
                                                  "Keplerian"};
        ImGui::PushItemWidth(220.0f*SCX);
            ImGui::PushID(id++);
                ImGui::Combo("  ", (int*)(&pos_vel_mut_var), cart_kep_mut_var, IM_ARRAYSIZE(cart_kep_mut_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (pos_vel_mut_var == CARTESIAN_MUT)
        {
            input_double("x ",  100.0f*SCX, 55.0f*SCX, id, "[km]",     cart_mut[0]);
            input_double("y ",  100.0f*SCX, 55.0f*SCX, id, "[km]",     cart_mut[1]);
            input_double("z ",  100.0f*SCX, 55.0f*SCX, id, "[km]",     cart_mut[2]);
            input_double("υx ", 100.0f*SCX, 55.0f*SCX, id, "[km/sec]", cart_mut[3]);
            input_double("υy ", 100.0f*SCX, 55.0f*SCX, id, "[km/sec]", cart_mut[4]);
            input_double("υz ", 100.0f*SCX, 55.0f*SCX, id, "[km/sec]", cart_mut[5]);
        }
        else //pos_vel_mut_var == KEPLERIAN_MUT
        {
            input_double("a ", 100.0f*SCX, 55.0f*SCX, id, "[km]",   kep_mut[0]);
            input_double("e ", 100.0f*SCX, 55.0f*SCX, id, "[    ]", kep_mut[1]);
            input_double("i ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_mut[2]);
            input_double("Ω ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_mut[3]);
            input_double("ω ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_mut[4]);
            input_double("M ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_mut[5]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        ImGui::Text("Orientations");
        static const char *rpy_quat_var[2] = {"Euler angles (XYZ)",
                                              "Quaternions (WXYZ)"};
        ImGui::PushItemWidth(220.0f*SCX);
            ImGui::PushID(id++);
                ImGui::Combo("  ", (int*)(&orient_var), rpy_quat_var, IM_ARRAYSIZE(rpy_quat_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (orient_var == EULER_XYZ)
        {
            input_double("Roll 1 " , 100.0f*SCX, 80.0f*SCX, id, "[deg]", rpy1[0]);
            input_double("Pitch 1 ", 100.0f*SCX, 80.0f*SCX, id, "[deg]", rpy1[1]);
            input_double("Yaw 1 ",   100.0f*SCX, 80.0f*SCX, id, "[deg]", rpy1[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            input_double("Roll 2 ",  100.0f*SCX, 80.0f*SCX, id, "[deg]", rpy2[0]);
            input_double("Pitch 2 ", 100.0f*SCX, 80.0f*SCX, id, "[deg]", rpy2[1]);
            input_double("Yaw 2 ",   100.0f*SCX, 80.0f*SCX, id, "[deg]", rpy2[2]);
        }
        else //orient_var == QUATERNION
        {
            input_double("q10 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q1[0]);
            input_double("q11 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q1[1]);
            input_double("q12 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q1[2]);
            input_double("q13 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q1[3]);
            ImGui::Dummy(ImVec2(0.0f,5.0f*SCY));
            input_double("q20 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q2[0]);
            input_double("q21 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q2[1]);
            input_double("q22 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q2[2]);
            input_double("q23 ", 100.0f*SCX, 70.0f*SCX, id, "[    ]", q2[3]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        ImGui::Text("Angular velocities");
        static const char *omega_frame[2] = {"Heliocentric (inertial)",
                                             "Body frames"};
        ImGui::PushItemWidth(220.0f*SCX);
            ImGui::PushID(id++);
                ImGui::Combo("  ", (int*)(&angvel_frame), omega_frame, IM_ARRAYSIZE(omega_frame));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (angvel_frame == ANGVEL_HELIO)
        {
            input_double("ω1x ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w1i[0]);
            input_double("ω1y ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w1i[1]);
            input_double("ω1z ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w1i[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f*SCY));
            input_double("ω2x ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w2i[0]);
            input_double("ω2y ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w2i[1]);
            input_double("ω2z ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w2i[2]);
        }
        else //angvel_frame == ANGVEL_BODY
        {
            input_double("ω11 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w1b[0]);
            input_double("ω12 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w1b[1]);
            input_double("ω13 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w1b[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f*SCY));
            input_double("ω21 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w2b[0]);
            input_double("ω22 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w2b[1]);
            input_double("ω23 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", w2b[2]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        ImGui::Text("Binary COM (Heliocentric)");
        static const char *cart_kep_com_var[2] = {"Cartesian",
                                                  "Keplerian"};
        ImGui::PushItemWidth(220.0f*SCX);
            ImGui::PushID(id++);
                ImGui::Combo("  ", (int*)(&pos_vel_com_var), cart_kep_com_var, IM_ARRAYSIZE(cart_kep_com_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (pos_vel_com_var == CARTESIAN_COM_HELIO)
        {
            input_double("x ",  100.0f*SCX, 55.0f*SCX, id, "[AU]",     cart_com_helio[0]);
            input_double("y ",  100.0f*SCX, 55.0f*SCX, id, "[AU]",     cart_com_helio[1]);
            input_double("z ",  100.0f*SCX, 55.0f*SCX, id, "[AU]",     cart_com_helio[2]);
            input_double("υx ", 100.0f*SCX, 55.0f*SCX, id, "[km/sec]", cart_com_helio[3]);
            input_double("υy ", 100.0f*SCX, 55.0f*SCX, id, "[km/sec]", cart_com_helio[4]);
            input_double("υz ", 100.0f*SCX, 55.0f*SCX, id, "[km/sec]", cart_com_helio[5]);
        }
        else //pos_vel_com_var == KEPLERIAN_COM_HELIO
        {
            input_double("a ", 100.0f*SCX, 55.0f*SCX, id, "[AU]",   kep_com_helio[0]);
            input_double("e ", 100.0f*SCX, 55.0f*SCX, id, "[    ]", kep_com_helio[1]);
            input_double("i ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_com_helio[2]);
            input_double("Ω ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_com_helio[3]);
            input_double("ω ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_com_helio[4]);
            input_double("M ", 100.0f*SCX, 55.0f*SCX, id, "[deg]",  kep_com_helio[5]);
        }
        ImGui::Unindent();
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        ImGui::Checkbox("Assume Sun's gravity", &sun_gravity);
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Collision choice logic :
        ImGui::SeparatorText("Collision shapes");
        if (ImGui::Checkbox("No collision  (1/r singularity risk)", &collision_no))
            collision_spheres = false;
        if (ImGui::Checkbox("Spheres", &collision_spheres))
            collision_no = false;
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));



        //Kinetic impactors logic :
        ImGui::SeparatorText("Kinetic impactors");
        if (ImGui::Checkbox("Assume impactors at bodies", &impactors_checkbox) && impactors_checkbox)
            impactors_clicked_ok = false;
        if (impactors_checkbox && !impactors_clicked_ok)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(0.15f*sx, 0.4f*sy), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Impactor parameters", &impactors_checkbox);

            //Radiobuttons logic : At least one will always be active and to this, the impactor's parameters shall correspond.
            static int impactor_refers_to_body = 1;
            if (ImGui::RadioButton("Body 1", impactor_refers_to_body == 1))
                impactor_refers_to_body = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Body 2", impactor_refers_to_body == 2))
                impactor_refers_to_body = 2;
            ImGui::Dummy(ImVec2(0.0f,5.0f*SCY));
            //Impactor menu :
            if (impactor_refers_to_body == 1)
                render_impactor_menu(mD1, vD1, beta1, tD1, id);
            else
                render_impactor_menu(mD2, vD2, beta2, tD2, id);
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
            if (ImGui::Button("OK", ImVec2(50.0f*SCX,30.0f*SCY)))
                impactors_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Spacecraft orbiter logic :
        ImGui::SeparatorText("Spacecraft orbiter");
        if (ImGui::Checkbox("Assume spacecraft orbiter", &spacecraft_checkbox) && spacecraft_checkbox)
            spacecraft_clicked_ok = false;
        if (spacecraft_checkbox && !spacecraft_clicked_ok)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(0.15f*sx, 0.4f*sy), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Spacecraft state", &spacecraft_checkbox);

            //Spacecraft mass :
            ImGui::Text("Mass");
            input_double("m ", 150.0f*SCX, 30.0f*SCX, id, "[kg]", sp_mass);
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));

            ImGui::Text("Position and velocity");
            static const char *cart_kep_sp_var[6] = {"Cartesian (binary COM)",
                                                     "Cartesian (body 1)",
                                                     "Cartesian (body 2)",
                                                     "Keplerian (binary COM)",
                                                     "Keplerian (body 1)",
                                                     "Keplerian (body 2)"};
            ImGui::PushItemWidth(220.0f*SCX);
                ImGui::PushID(id++);
                    ImGui::Combo("  ", (int*)(&pos_vel_sp_var), cart_kep_sp_var, IM_ARRAYSIZE(cart_kep_sp_var));
                ImGui::PopID();
            ImGui::PopItemWidth();
            if (pos_vel_sp_var == CARTESIAN_SP_COM)
            {
                input_double("x ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com[0]);
                input_double("y ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com[1]);
                input_double("z ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com[2]);
                input_double("υx ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com[3]);
                input_double("υy ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com[4]);
                input_double("υz ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com[5]);
            }
            else if (pos_vel_sp_var == CARTESIAN_SP_COM1)
            {
                input_double("x ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com1[0]);
                input_double("y ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com1[1]);
                input_double("z ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com1[2]);
                input_double("υx ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com1[3]);
                input_double("υy ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com1[4]);
                input_double("υz ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com1[5]);
            }
            else if (pos_vel_sp_var == CARTESIAN_SP_COM2)
            {
                input_double("x ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com2[0]);
                input_double("y ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com2[1]);
                input_double("z ",  100.0f*SCX, 40.0f*SCX, id, "[km]",     cart_sp_com2[2]);
                input_double("υx ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com2[3]);
                input_double("υy ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com2[4]);
                input_double("υz ", 100.0f*SCX, 40.0f*SCX, id, "[km/sec]", cart_sp_com2[5]);
            }
            else if (pos_vel_sp_var == KEPLERIAN_SP_COM)
            {
                input_double("a ", 100.0f*SCX, 40.0f*SCX, id, "[km]",   kep_sp_com[0]);
                input_double("e ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", kep_sp_com[1]);
                input_double("i ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com[2]);
                input_double("Ω ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com[3]);
                input_double("ω ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com[4]);
                input_double("M ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com[5]);
            }
            else if (pos_vel_sp_var == KEPLERIAN_SP_COM1)
            {
                input_double("a ", 100.0f*SCX, 40.0f*SCX, id, "[km]",   kep_sp_com1[0]);
                input_double("e ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", kep_sp_com1[1]);
                input_double("i ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com1[2]);
                input_double("Ω ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com1[3]);
                input_double("ω ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com1[4]);
                input_double("M ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com1[5]);
            }
            else //pos_vel_sp_var == KEPLERIAN_SP_COM2
            {
                input_double("a ", 100.0f*SCX, 40.0f*SCX, id, "[km]",   kep_sp_com2[0]);
                input_double("e ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", kep_sp_com2[1]);
                input_double("i ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com2[2]);
                input_double("Ω ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com2[3]);
                input_double("ω ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com2[4]);
                input_double("M ", 100.0f*SCX, 40.0f*SCX, id, "[deg]",  kep_sp_com2[5]);
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));



































            ImGui::Checkbox("Is rigid body", &sp_is_rigidbody_checkbox);
            if (!sp_is_rigidbody_checkbox)
            {
                sp_ell_checkbox = sp_obj_checkbox = sp_ell_clicked_ok = sp_obj_clicked_ok = false;
                ImGui::BeginDisabled();
            }

            
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            ImGui::Text("Shape model");
            
            //Ellipsoid shape logic :
            if (ImGui::Checkbox("Ellipsoid##sp_ell_checkbox", &sp_ell_checkbox) && sp_ell_checkbox)
                sp_ell_clicked_ok = false;
            if (sp_ell_checkbox && !sp_ell_clicked_ok)
            {
                sp_obj_checkbox = false; //Untick the obj checkbox in case it is ticked.

                ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
                ImGui::SetNextWindowSize(ImVec2(0.15f*sx, 0.4f*sy), ImGuiCond_FirstUseEver); 
                ImGui::Begin("Ellipsoid parameters (spacecraft)##spacecraft", &sp_ell_checkbox);
                
                input_double("a ", 100.0f*SCX, 30.0f*SCX, id, "[km]", sp_semiaxes[0]);
                input_double("b ", 100.0f*SCX, 30.0f*SCX, id, "[km]", sp_semiaxes[1]);
                input_double("c ", 100.0f*SCX, 30.0f*SCX, id, "[km]", sp_semiaxes[2]);
                ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
                if (ImGui::Button("OK", ImVec2(50.0f*SCX,30.0f*SCY)))
                    sp_ell_clicked_ok = true;

                ImGui::End();
            }
            //.obj shape logic :
            if (ImGui::Checkbox(".obj file##sp_obj_checkbox", &sp_obj_checkbox) && sp_obj_checkbox)
                sp_obj_clicked_ok = false;
            if (sp_obj_checkbox && !sp_obj_clicked_ok)
            {
                sp_ell_checkbox = false; //Untick the ellipsoid checkbox in case it is ticked.

                ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(0.15f*sx, 0.4f*sy), ImGuiCond_FirstUseEver); 
                ImGui::Begin(".obj files (spacecraft)##spacecraft", &sp_obj_checkbox);

                //File listing and selection logic :
                static std::vector<std::filesystem::path> all_obj_files;
                bool tree_opened = ImGui::TreeNodeEx("Available .obj files", ImGuiTreeNodeFlags_Framed);
                if (ImGui::IsItemToggledOpen() && tree_opened) //Opened >>this<< frame only.
                    all_obj_files = list_obj_files("../obj/polyhedra/");
                if (tree_opened)
                {
                    for (size_t i = 0; i < all_obj_files.size(); ++i)
                    {
                        bool selected = (all_obj_files[i].string() == sp_obj_path);
                        if (ImGui::Selectable(all_obj_files[i].string().c_str(), selected))
                        {
                            sp_obj_path = all_obj_files[i].string();
                            sp_obj_clicked_ok = false;
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::Dummy(ImVec2(0.0f,20.0f*SCY));
                if (ImGui::Button("OK", ImVec2(50.0f*SCX,30.0f*SCY)))   
                    sp_obj_clicked_ok = true;

                ImGui::End();
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));

            ImGui::Text("Orientation");
            static const char *sp_rpy_quat_var[2] = {"Euler angles (XYZ)",
                                                     "Quaternions (WXYZ)"};
            ImGui::PushItemWidth(220.0f*SCX);
                ImGui::PushID(id++);
                    ImGui::Combo("  ", (int*)(&sp_orient_var), sp_rpy_quat_var, IM_ARRAYSIZE(sp_rpy_quat_var));
                ImGui::PopID();
            ImGui::PopItemWidth();
            if (sp_orient_var == EULER_XYZ_SP)
            {
                input_double("Roll " , 100.0f*SCX, 50.0f*SCX, id, "[deg]", rpy_sp[0]);
                input_double("Pitch ", 100.0f*SCX, 50.0f*SCX, id, "[deg]", rpy_sp[1]);
                input_double("Yaw ",   100.0f*SCX, 50.0f*SCX, id, "[deg]", rpy_sp[2]);
            }
            else //sp_orient_var == QUATERNION_SP
            {
                input_double("q0 ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", qsp[0]);
                input_double("q1 ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", qsp[1]);
                input_double("q2 ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", qsp[2]);
                input_double("q3 ", 100.0f*SCX, 40.0f*SCX, id, "[    ]", qsp[3]);
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


            ImGui::Text("Angular velocity");
            static const char *sp_omega_frame[2] = {"Heliocentric (inertial)",
                                                    "Body frames"};
            ImGui::PushItemWidth(220.0f*SCX);
                ImGui::PushID(id++);
                    ImGui::Combo("  ", (int*)(&sp_angvel_frame), sp_omega_frame, IM_ARRAYSIZE(sp_omega_frame));
                ImGui::PopID();
            ImGui::PopItemWidth();
            if (sp_angvel_frame == ANGVEL_SP_HELIO)
            {
                input_double("ωx ", 100.0f*SCX, 40.0f*SCX, id, "[rad/sec]", wi_sp[0]);
                input_double("ωy ", 100.0f*SCX, 40.0f*SCX, id, "[rad/sec]", wi_sp[1]);
                input_double("ωz ", 100.0f*SCX, 40.0f*SCX, id, "[rad/sec]", wi_sp[2]);
            }
            else //sp_angvel_frame == ANGVEL_BODY
            {
                input_double("ω1 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", wb_sp[0]);
                input_double("ω2 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", wb_sp[1]);
                input_double("ω3 ", 100.0f*SCX, 70.0f*SCX, id, "[rad/sec]", wb_sp[2]);
            }

            if (!sp_is_rigidbody_checkbox)
                ImGui::EndDisabled();
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));

            ImGui::Checkbox("Account for SRP", &srp_checkbox);
            if (!srp_checkbox)
                ImGui::BeginDisabled();
            ImGui::Dummy(ImVec2(0.0f,7.5f*SCY));
            ImGui::Text("SRP parameters");
            input_double("ρ ", 100.0f*SCX, 40.0f*SCX, id, "[  ]",  sp_refl);
            input_double("A ", 100.0f*SCX, 40.0f*SCX, id, "[m^2]", sp_area);
            ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));
            ImGui::Checkbox("Account for shadows", &srp_shadow_checkbox);
            if (!srp_checkbox)
                ImGui::EndDisabled();
            ImGui::Dummy(ImVec2(0.0f,30.0f*SCY));
            if (ImGui::Button("OK", ImVec2(50.0f*SCX,30.0f*SCY)))
                spacecraft_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f*SCY));


        //Run/Abort buttons rendering logic :
        ImGui::SeparatorText("Simulation controls");

        if (!task_is_running.load()) //In this case a task is NOT currently running, hence "Run" can be pressed, but "Abort", cannot be pressed (nothing to abort).
        {
            if (ImGui::Button("Run", ImVec2(70.0f*SCX, 25.0f*SCY)))
                run_pressed = true;
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::Button("Abort", ImVec2(70.0f*SCX, 25.0f*SCY));
            ImGui::EndDisabled();
        }
        else //Now the opposite happens. "Run" is disabled, coz a task is running and "Abort" is enabled, so that one may stop the running task.
        {
            ImGui::BeginDisabled();
            ImGui::Button("Run", ImVec2(70.0f*SCX, 25.0f*SCY));
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Abort", ImVec2(70.0f*SCX, 25.0f*SCY)))
                abort_pressed = true;
        }


        //Progress bar logic :
        ImGui::Text("Integrator progress");
        if (task_was_aborted.load())
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f,0.0f,0.0f, 1.0f)); //Red.
        else
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f,0.7f,0.0f, 1.0f)); //Green.
        ImGui::ProgressBar(task_progress.load(), ImVec2(150.0f*SCX,17.0f*SCY));
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0.0f,700.0f*SCY)); //Some extra y-space in order to be able to scroll down comfortably along the properties panel.

        ImGui::End();
    }
};

#endif
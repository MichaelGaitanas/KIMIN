/* This class handles the rendering logic of the left panel (properties) in the gui. */

#ifndef PROPERTIES_PANEL_H
#define PROPERTIES_PANEL_H

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<vector>
#include<filesystem>
#include<atomic>
#include<cstring>
#include<string>

#include"constants.h"
#include"linalg.h"
#include"conversion.h"
#include"polyhedron.h"
#include"console_panel.h"
#include"file.h"

class properties_panel
{
public:
    char sim_name[128]; //'Simulation name' text field (imgui shall stop the input characters to 127 to prevent overflow).
    
    bool ell_checkbox; //'Ellipsoids' checkbox state.
    dvec3 semiaxes1, semiaxes2; //'a1', 'b1', 'c1', 'a2', 'b2', 'c2' double fields.
    bool ell_clicked_ok; //Ellipsoids 'OK' button.

    bool obj_checkbox; //'.obj file' checkbox state.
    std::string obj1_path, obj2_path; //Relative paths to the 2 .obj models.
    bool obj_clicked_ok; //.obj 'OK' button.

    bool ord2_checkbox, ord3_checkbox, ord4_checkbox; //'Mutual potential' available checkboxes.

    double M1, M2; //'M1', 'M2' double fields.

    enum
    {
        RKF78_FIXED,
        RKF78_ADAPTIVE,
        BSTOER_ADAPTIVE,
        ABM5_FIXED
    } integration_method;
    double epoch, dur; //'Epoch', 'Duration' double fields.
    double step; //'Step' double field (if a fixed-step method is chosen).
    double target_error; //'Target error' double field (if an adaptive method is chosen).

    enum
    {
        CARTESIAN,
        KEPLERIAN
    } pos_vel_var;
    dvec6 cart; //'x', 'y', 'z', 'υx', 'υy', 'υz' double fields of the relative position/velocity.
    dvec6 kep; //'a', 'e', 'i', 'Ω', 'ω', 'M' double fields of the relative position/velocity.

    enum
    {
        EULER_XYZ,
        QUATERNION
    } orient_var;
    dvec3 rpy1, rpy2; //'roll 1', 'pitch 1', 'yaw 1', 'roll 2', 'pitch 2', 'yaw 2' double fields.
    dvec4 q1, q2; //'q10', 'q11', 'q12', 'q13', 'q20', 'q21', 'q22', 'q23' double fields.

    enum
    {
        INERTIAL, //Either WORLD or COM frame.
        BODY
    } angvel_frame;
    dvec3 w1i, w2i; //'ω1ix', 'ω1iy', 'ω1iz', 'ω2ix', 'ω2iy, 'ω2iz' double fields.
    dvec3 w1b, w2b; //'ω1bx', 'ω1by', 'ω1bz', 'ω2bx', 'ω2by, 'ω2bz' double fields.

    dvec3 r_com, v_com; //'x', 'y', 'z', 'υx', 'υy', 'υz' double fields of the center of mass of the binary.

    bool collision_no, collision_spheres, collision_polyhedra; //Which type of collision criterion to apply in the simulation.

    bool impactors_checkbox; //'Kinetic impactors' checkbox state.
    double mD1, mD2; //'m1', 'm2' double fields.
    dvec3 vD1, vD2; //'υx1', 'υy1', 'υz1', 'υx2', 'υy2', 'υz2' double fields.
    double beta1, beta2; //'β1', 'β2' double fields.
    double tD1, tD2; //'t1', 't2' double fields.
    bool impactors_clicked_ok; //Impactors 'OK' button.

    bool spacecraft_checkbox; //'Spacecraft orbiter' checkbox state.
    dvec3 r_sp, v_sp; //Spacecraft's 'xs', 'ys', 'zs' and 'υxs', 'υys', 'υzs' double fields.
    bool spacecraft_clicked_ok; //Spacecraft's 'OK' button.

    bool run_pressed; //Whether or not the 'Run' button has been pressed.
    bool abort_pressed; //Whether or not the 'Abort' button has been pressed.

    polyhedron poly1, poly2; //Polyhedra instances.

    properties_panel() : sim_name(""),
                         ell_checkbox(false),
                         semiaxes1(dvec3{0.0,0.0,0.0}),
                         semiaxes2(dvec3{0.0,0.0,0.0}),
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
                         epoch(0.0),
                         dur(0.0),
                         step(0.0),
                         target_error(1.0e-12),
                         pos_vel_var(CARTESIAN),
                         cart(dvec6{0.0,0.0,0.0,0.0,0.0,0.0}),
                         kep(dvec6{0.0,0.0,0.0,0.0,0.0,0.0}),
                         orient_var(EULER_XYZ),
                         rpy1(dvec3{0.0,0.0,0.0}),
                         rpy2(dvec3{0.0,0.0,0.0}),
                         q1(dvec4{1.0,0.0,0.0,0.0}),
                         q2(dvec4{1.0,0.0,0.0,0.0}),
                         angvel_frame(INERTIAL),
                         w1i(dvec3{0.0,0.0,0.0}),
                         w2i(dvec3{0.0,0.0,0.0}),
                         w1b(dvec3{0.0,0.0,0.0}),
                         w2b(dvec3{0.0,0.0,0.0}),
                         r_com(dvec3{0.0,0.0,0.0}),
                         v_com(dvec3{0.0,0.0,0.0}),
                         collision_no(false),
                         collision_spheres(false),
                         collision_polyhedra(false),
                         impactors_checkbox(false),
                         mD1(0.0),
                         mD2(0.0),
                         vD1(dvec3{0.0,0.0,0.0}),
                         vD2(dvec3{0.0,0.0,0.0}),
                         beta1(0.0),
                         beta2(0.0),
                         tD1(0.0),
                         tD2(0.0),
                         impactors_clicked_ok(false),
                         spacecraft_checkbox(false),
                         r_sp(dvec3{0.0,0.0,0.0}),
                         v_sp(dvec3{0.0,0.0,0.0}),
                         spacecraft_clicked_ok(false),
                         run_pressed(false),
                         abort_pressed(false),
                         poly1(),
                         poly2()
    { }

    //This function parses the user-chosen input file regarding the properties.
    void import_file(const char *path, console_panel &console)
    {
        *this = properties_panel{}; //Reset the inputs. This command basically re-runs the constructor.

        FILE *fp = fopen(path,"r");
        if (!fp) //Safety check, though this should never happen. It is already verified by the top_bar_panel that the file exists, otherwise it would not appear as an available choice in the gui.
        {
            console.add_timed_text("[Error] : The selected properties file could not be opened.\n");
            return;
        }

        //Parse the simulation name.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", sim_name);

        char buffer[128];

        //Parse the shape model.
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

        //Parse the mutual potential order.
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

        //Parse the numerical method of integration.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "RKF78 (fixed)") == 0)
        {
            integration_method = RKF78_FIXED;
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &step);
        }
        else if (strcmp(buffer, "RKF78 (adaptive)") == 0)
        {
            integration_method = RKF78_ADAPTIVE;
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &target_error);
        }
        else if (strcmp(buffer, "BStoer (adaptive)") == 0)
        {
            integration_method = BSTOER_ADAPTIVE;
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &target_error);
        }
        else
        {
            integration_method = ABM5_FIXED;
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &epoch);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &dur);
            if (find_assignment_operator(fp)) fscanf(fp, "%lf", &step);
        }

        //Parse the binary's initial position/velocity.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Cartesian") == 0)
        {
            pos_vel_var = CARTESIAN;
            for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &cart[i]);
        }
        else
        {
            pos_vel_var = KEPLERIAN;
            for (int i = 0; i < 6; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &kep[i]);
        }

        //Parse the bodies' initial orientations.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "Euler angles") == 0)
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
        if (strcmp(buffer, "At inertial frame") == 0)
        {
            angvel_frame = INERTIAL;
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w1i[i]);
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w2i[i]);
        }
        else
        {
            angvel_frame = BODY;
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w1b[i]);
            for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &w2b[i]);
        }

        //Parse the C.O.M. initial state.
        for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &r_com[i]);
        for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &v_com[i]);

        //Parse the collision shapes.
        if (find_assignment_operator(fp)) fscanf(fp, " \"%127[^\"]\"", buffer);
        if (strcmp(buffer, "No collision") == 0)
            collision_no = true;
        else if (strcmp(buffer, "Spheres") == 0)
            collision_spheres = true;
        else
            collision_polyhedra = true;
        
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

        //Parse the spacecraft orbiter's initial state.
        if (find_assignment_operator(fp))
        {
            fscanf(fp, " \"%127[^\"]\"", buffer);
            if (strcmp(buffer, "Yes") == 0)
            {
                for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &r_sp[i]);
                for (int i = 0; i < 3; ++i) if (find_assignment_operator(fp)) fscanf(fp, "%lf", &v_sp[i]);
                spacecraft_checkbox = spacecraft_clicked_ok = true;
            }
        }

        fclose(fp);
    }

private:
    //This function receives as input a 'path' to a directory and as a result it returns a vector of paths, corresponding
    //to all the .obj files found inside 'path'.
    std::vector<std::filesystem::path> list_obj_files(const char *path)
    {
        std::vector<std::filesystem::path> paths;
        for (const auto &entry : std::filesystem::directory_iterator(path))
            if (entry.is_regular_file() && entry.path().extension() == ".obj")
                paths.push_back(entry.path().filename());
        return paths;
    }

public:
    //This function automates common double inputs via the keyboard. It creates a rectangle, inside of which the user may enter a double.
    //'label' is a string written on the left of the rectangle. 'item_width' is the horizontal legth (space) of the rectangle. 'id' is a unique
    //int via which the computer identifies which variable to affect. 'unit' is a string written on the right of the rectangle (acting as unit of measurement).
    //'variable' is the variable itself, passed by reference to InputDouble(), so it may change.
    void double_field(const char *label, const float item_width, const float align_width, int &id, const char *unit, double &variable)
    {
        ImGui::Text(label);
        ImGui::SameLine();
        ImGui::SetCursorPosX(align_width); //Set a width for left label spacing, so that all double input rectangles appear aligned.
        ImGui::PushItemWidth(item_width);
            ImGui::PushID(id++);
                ImGui::InputDouble(unit, &variable, 0.0, 0.0, "%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
    }

    //This member function processes all the user inputs and checks if they are valid, assuming some rules, defined by me.
    //If at least 1 rule is not satisfied, the corresponding errors are displayed in the console and the simulation will not run, until fixed.
    bool validate(console_panel &console)
    {   
        //Possible error 1 : Simulation name (empty, pure spaces, begin with space, illegal characters).
        std::string sim_name_copy = sim_name;
        if ( (sim_name_copy.empty()) || (sim_name_copy.find_first_not_of(' ') == std::string::npos) || (sim_name_copy[0] == ' ') || (sim_name_copy.find_first_of("<>:\"/\\|?*") != std::string::npos) )
            {console.add_timed_text("[Error] : 'Simulation name' is invalid.\n"); return false;}

        //Possible error 2 : Shape model checkboxes (at least one must be checked when the 'Run' button has been pressed).
        if (!ell_checkbox && !obj_checkbox)
            {console.add_timed_text("[Error] : Neither 'Ellipsoids', nor '.obj files' is selected for determining the shapes.\n"); return false;}

        //Possible error 3 : 'OK' button in the Elliposid parameters window (it must be clicked so that the parameters are taken into account).
        if (ell_checkbox && !ell_clicked_ok)
            {console.add_timed_text("[Error] : 'OK' button must be pressed in the 'Ellipsoid parameters' window.\n"); return false;}

        //Possible error 4 : 'OK' button in the '.obj files' window (it must be clicked so that the .obj files are taken into account).
        if (obj_checkbox && !obj_clicked_ok)
            {console.add_timed_text("[Error] : 'OK' button must be pressed in the '.obj files' window.\n"); return false;}

        //Possible error 5 : Ellipsoids semiaxes (all semiaxes must be > 0).
        if (ell_checkbox)
        {
            if (semiaxes1[0] <= 0.0 || semiaxes1[1] <= 0.0 || semiaxes1[2] <= 0.0)
                {console.add_timed_text("[Error] : 'a1', 'b1', 'c1' must be positive numbers.\n"); return false;}
            if (semiaxes2[0] <= 0.0 || semiaxes2[1] <= 0.0 || semiaxes2[2] <= 0.0)
                {console.add_timed_text("[Error] : 'a2', 'b2', 'c2' must be positive numbers.\n"); return false;}

            if (semiaxes1[0] > 0.0 && semiaxes1[1] > 0.0 && semiaxes1[2] > 0.0 &&
                semiaxes2[0] > 0.0 && semiaxes2[1] > 0.0 && semiaxes2[2] > 0.0)
            {
                poly1.load_obj_file("../obj/polyhedra/uvsphere64x64_R1km.obj");
                poly2 = poly1; //Do not parse the same .obj file... But the assignment must happen BEFORE altering the poly1 mesh!

                poly1.set_scale(semiaxes1);
                poly1.gen_norms();
                poly2.set_scale(semiaxes2);
                poly2.gen_norms();
            }
        }

        //Possible error 6 : .obj files (at least one .obj file per body must be selected). Also the polyhedra must be closed manifold geometries.
        if (obj_checkbox)
        {   
            if (obj1_path.empty())
                {console.add_timed_text("[Error] : No .obj file is selected for 'Body 1'.\n"); return false;}
            else
            {
                console.add_timed_text("[Polyhedron] : Loading .obj file 1... ");
                if (poly1.is_kimin_valid_obj(("../obj/polyhedra/" + obj1_path).c_str()))
                {
                    poly1.load_obj_file(("../obj/polyhedra/" + obj1_path).c_str());
                    if (!poly1.is_closed_manifold())
                        {console.add_text("< Invalid .obj file for 'Body 1' (non closed manifold). >\n"); return false;}
                    else
                        console.add_text("Done.\n");
                }
                else
                    {console.add_text("< Invalid .obj file for 'Body 1' (it must contain only vertices and faces). >\n"); return false;}
            }
            if (obj2_path.empty())
                {console.add_timed_text("[Error] : No .obj file is selected for 'Body 2'.\n"); return false;}
            else
            {
                console.add_timed_text("[Polyhedron] : Loading .obj file 2... ");
                if (poly2.is_kimin_valid_obj(("../obj/polyhedra/" + obj2_path).c_str()))
                {
                    poly2.load_obj_file(("../obj/polyhedra/" + obj2_path).c_str());
                    if (!poly2.is_closed_manifold())
                        {console.add_text("< Invalid .obj file for 'Body 2' (non closed manifold). >\n"); return false;}
                    else
                        console.add_text("Done.\n");
                }
                else
                    {console.add_text("< Invalid .obj file for 'Body 2' (it must contain only vertices and faces). >\n"); return false;}
            }
        }

        //Possible error 7 : Mutual potential checkboxes (at least one must be checked).
        if (!ord2_checkbox && !ord3_checkbox && !ord4_checkbox)
            {console.add_timed_text("[Error] : Neither 'Order 2', nor 'Order 3', nor 'Order 4' mutual potential is selected.\n"); return false;}

        //Possible error 8 : Masses (both M1 and M2 must be > 0).
        if (M1 <= 0.0 || M2 <= 0.0)
            {console.add_timed_text("[Error] : 'M1', 'M2' must be positive numbers.\n"); return false;}

        //Possible error 9 : Time parameters ('Epoch' and 'Duration' must be >= 0, 'Step' must be <= 'Duration' and 'Target error' must be > 0).
        if (integration_method == RKF78_FIXED || integration_method == ABM5_FIXED)
        {
            if (!(epoch >= 0.0 && dur > 0.0 && step <= dur && step > 0.0))
                {console.add_timed_text("[Error] : Invalid set of 'Epoch', 'Duration', 'Step'.\n"); return false;}
        }
        else
        {
            if (!(epoch >= 0.0 && dur > 0.0 && target_error > 0.0))
                {console.add_timed_text("[Error] : Invalid set of 'Epoch', 'Duration', 'Target error'.\n"); return false;}
        }

        //Possible error 10 : Relative position/velocity (mutual distance must be > 0).
        if (pos_vel_var == CARTESIAN && length(dvec3{cart[0], cart[1], cart[2]}) <= 0.0)
            {console.add_timed_text("[Error] : Invalid set of 'x', 'y', 'z' (mutual distance must be positive).\n"); return false;}

        //Possible error 11 : Relative Keplerian elements ( 'a' must be > 0, 'e' must be in [0,1)U(1,inf) )
        if (pos_vel_var == KEPLERIAN)
        {
            if (kep[0] <= 0.0)
                {console.add_timed_text("[Error] : Semi-major axis 'a' must be positive.\n"); return false;}
            if (kep[1] < 0.0 || fabs(kep[1] - 1.0) <= 1e-15)
                {console.add_timed_text("[Error] : Eccentricity 'e' must be in [0,1)U(1,inf).\n"); return false;}
        }

        //Possible error 12 : Quaternion (both must be nonzero).
        //Note : In case of non normalized quaternion input, the program normalizes them both automatically.
        if (orient_var == QUATERNION)
        {
            if (length(q1) <= 1e-15)
                {console.add_timed_text("[Error] : Quaternion 1 ('q10', 'q11', 'q12', 'q13') must be nonzero.\n"); return false;}
            else //Normalize it no matter what.
                q1 = quat2unit(q1); //This correction will be visible in the gui.

            //The same for q2 :
            if (length(q2) <= 1e-15)
                {console.add_timed_text("[Error] : Quaternion 2 ('q20', 'q21', 'q22', 'q23') must be nonzero.\n"); return false;}
            else //Normalize it no matter what.
                q2 = quat2unit(q2); //This correction will be visible in the gui.
        }

        //Possible error 13 : Collision shapes (at least one must be checked).
        if (!collision_no && !collision_spheres && !collision_polyhedra)
            {console.add_timed_text("[Error] : At least one collision criterion must be selected.\n"); return false;}

        //Possible error 14 : 'OK' button in the impactors' parameters window (it must be clicked so that the parameters are taken into account).
        if (impactors_checkbox && !impactors_clicked_ok)
            {console.add_timed_text("[Error] : 'OK' button must be pressed in the 'Impactors' parameters' window.\n"); return false;}

        //Possible error 15 : Impactors' parameters (masses must be >= 0).
        if (impactors_checkbox && (mD1 < 0.0 || mD2 < 0.0))
            {console.add_timed_text("[Error] : Both impactors' masses, 'm1' and 'm2' must be non negative.\n"); return false;}

        //Possible error 16 : Times of impacts must range in the simulated time range, i.e. in [Epoch, Epoch + Duration]
        if (impactors_checkbox && (tD1 < epoch || tD1 > epoch + dur || tD2 < epoch || tD2 > epoch + dur))
            {console.add_timed_text("[Error] : Impact times must range in [Epoch,  Epoch + Duration].\n"); return false;}

        //Possible error 17 : 'OK' button in the spacecraft orbiter parameters window (it must be clicked so that the i.c. are taken into account).
        if (spacecraft_checkbox && !spacecraft_clicked_ok)
            {console.add_timed_text("[Error] : 'OK' button must be pressed in the 'Orbiter's initial state' window.\n"); return false;}

        return true;
    }

    //This is the function that draws the properties panel and processes the corresponding logic.
    void render(std::atomic<bool> task_is_running, std::atomic<bool> task_was_aborted, std::atomic<float> task_progress)
    {
        int id = 0;

        //Properties panel "main" window.
        ImGui::SetNextWindowPos(ImVec2(0.0f, 21.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 21.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties", nullptr);

        //Simulation name text field. Basically this is the name of the folder that will be created later, holding the orbit data.
        ImGui::Text("Simulation name");
        ImGui::PushItemWidth(200.0f);
            ImGui::InputText(" ", sim_name, IM_ARRAYSIZE(sim_name));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Ellipsoid and .obj shape logic.
        ImGui::Text("Shape models");

        //Ellipsoid shape logic.
        if (ImGui::Checkbox("Ellipsoids", &ell_checkbox) && ell_checkbox)
            ell_clicked_ok = false;
        if (ell_checkbox && !ell_clicked_ok)
        {
            obj_checkbox = false; //Untick the obj checkbox in case it is ticked.

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, 0.4f*ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters", &ell_checkbox);
            
            //Ellipsoids semiaxes menu.
            ImGui::Text("Body 1 semi - axes");
            double_field("a1 ", 100.0f, 30.0f, id, "[km]", semiaxes1[0]);
            double_field("b1 ", 100.0f, 30.0f, id, "[km]", semiaxes1[1]);
            double_field("c1 ", 100.0f, 30.0f, id, "[km]", semiaxes1[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Body 2 semi - axes");
            double_field("a2 ", 100.0f, 30.0f, id, "[km]", semiaxes2[0]);
            double_field("b2 ", 100.0f, 30.0f, id, "[km]", semiaxes2[1]);
            double_field("c2 ", 100.0f, 30.0f, id, "[km]", semiaxes2[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "OK" button. This must be pressed, otherwise the semi-axes values will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                ell_clicked_ok = true;

            ImGui::End();
        }
    
        //.obj shape logic.
        if (ImGui::Checkbox(".obj files", &obj_checkbox) && obj_checkbox)
            obj_clicked_ok = false;
        if (obj_checkbox && !obj_clicked_ok)
        {
            ell_checkbox = false; //Untick the ellipsoids checkbox in case it is ticked.

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, 0.4f*ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj files", &obj_checkbox);

            //Radiobuttons logic : At least one will always be active and to this, (the active one) the loaded obj file will correspond.
            static int obj_refers_to_body = 1; //To which body ('Body 1' or 'Body 2') does the obj file listing refer to (via radiobutton). 'Body 1' is the default choice.
            if (ImGui::RadioButton("Body 1", obj_refers_to_body == 1))
                obj_refers_to_body = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Body 2", obj_refers_to_body == 2))
                obj_refers_to_body = 2;
            ImGui::Dummy(ImVec2(0.0f,5.0f));

            //File listing and selection logic.
            static std::vector<std::filesystem::path> all_obj_files = list_obj_files("../obj/polyhedra/"); //Store all the .obj files located in the obj/polyhedra/ directory.
            if (ImGui::TreeNodeEx("Available .obj files", ImGuiTreeNodeFlags_Framed))
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
            ImGui::Dummy(ImVec2(0.0f,20.0f));

            //Final "OK" button. This must be pressed, otherwise the chosen obj files will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))   
                obj_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Mutual potential expansion desired order.
        ImGui::Text("Mutual potential");
        if (ImGui::Checkbox("Order 2", &ord2_checkbox))
            ord3_checkbox = ord4_checkbox = false;
        if (ImGui::Checkbox("Order 3", &ord3_checkbox))
            ord2_checkbox = ord4_checkbox = false;
        if (ImGui::Checkbox("Order 4", &ord4_checkbox))
            ord2_checkbox = ord3_checkbox = false;
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Masses.
        ImGui::Text("Masses");
        double_field("M1 ", 150.0f, 40.0f, id, "[kg]", M1);
        double_field("M2 ", 150.0f, 40.0f, id, "[kg]", M2);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Integration method and time parameters.
        ImGui::Text("Numerical integration");
        ImGui::Indent();

        ImGui::Text("Method");

        //Integration method (RKF78 constant, RKF78 adaptive, Bulirsch–Stoer adaptive).
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *methods[4] = {"RKF78 (fixed)",
                                                 "RKF78 (adaptive)",
                                                 "BStoer (adaptive)",
                                                 "ABM5  (fixed)"};
                ImGui::Combo("  ", (int*)(&integration_method), methods, IM_ARRAYSIZE(methods));
            ImGui::PopID();
        ImGui::PopItemWidth();

        double_field("Epoch ",     100.0f, 105.0f, id, "[days]", epoch);
        double_field("Duration ",  100.0f, 105.0f, id, "[days]", dur);
        if (integration_method == RKF78_FIXED || integration_method == ABM5_FIXED)
            double_field("Step ",  100.0f, 105.0f, id, "[days]", step);
        else //integration_method is adaptive, thus render the 'Target error' input field.
            double_field("Target error ", 100.0f, 105.0f, id, "[    ]", target_error);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Unindent();

        ImGui::Text("Initial state");
        ImGui::Indent();

        ImGui::Text("Relative position and velocity");

        //Initial position/velocity variables, either in the form of Cartesian coords, or Keplerian elements.
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *cart_kep_var[2] = {"Cartesian", "Keplerian"}; //Nature of the relative position and velocity variables.
                ImGui::Combo("  ", (int*)(&pos_vel_var), cart_kep_var, IM_ARRAYSIZE(cart_kep_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (pos_vel_var == CARTESIAN)
        {
            double_field("x ",  100.0f, 55.0f, id, "[km]",     cart[0]);
            double_field("y ",  100.0f, 55.0f, id, "[km]",     cart[1]);
            double_field("z ",  100.0f, 55.0f, id, "[km]",     cart[2]);
            double_field("υx ", 100.0f, 55.0f, id, "[km/sec]", cart[3]);
            double_field("υy ", 100.0f, 55.0f, id, "[km/sec]", cart[4]);
            double_field("υz ", 100.0f, 55.0f, id, "[km/sec]", cart[5]);
        }
        else //pos_vel_var == KEPLERIAN
        {
            double_field("a ", 100.0f, 55.0f, id, "[km]",  kep[0]);
            double_field("e ", 100.0f, 55.0f, id, "[    ]",  kep[1]);
            double_field("i ", 100.0f, 55.0f, id, "[deg]", kep[2]);
            double_field("Ω ", 100.0f, 55.0f, id, "[deg]", kep[3]);
            double_field("ω ", 100.0f, 55.0f, id, "[deg]", kep[4]);
            double_field("M ", 100.0f, 55.0f, id, "[deg]", kep[5]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Orientations");

        //Orientation variables (Euler angles (roll, pitch, yaw) or quaternions).
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *rpy_quat_var[2] = {"Euler angles (XYZ)", "Quaternions (WXYZ)"}; //Nature of the orientation variables.
                ImGui::Combo("  ", (int*)(&orient_var), rpy_quat_var, IM_ARRAYSIZE(rpy_quat_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (orient_var == EULER_XYZ)
        {
            double_field("roll 1 " , 100.0f, 80.0f, id, "[deg]", rpy1[0]);
            double_field("pitch 1 ", 100.0f, 80.0f, id, "[deg]", rpy1[1]);
            double_field("yaw 1 ",   100.0f, 80.0f, id, "[deg]", rpy1[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("roll 2 ",  100.0f, 80.0f, id, "[deg]", rpy2[0]);
            double_field("pitch 2 ", 100.0f, 80.0f, id, "[deg]", rpy2[1]);
            double_field("yaw 2 ",   100.0f, 80.0f, id, "[deg]", rpy2[2]);
        }
        else //orient_var == QUATERNION
        {
            double_field("q10 ", 100.0f, 70.0f, id, "[    ]", q1[0]);
            double_field("q11 ", 100.0f, 70.0f, id, "[    ]", q1[1]);
            double_field("q12 ", 100.0f, 70.0f, id, "[    ]", q1[2]);
            double_field("q13 ", 100.0f, 70.0f, id, "[    ]", q1[3]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("q20 ", 100.0f, 70.0f, id, "[    ]", q2[0]);
            double_field("q21 ", 100.0f, 70.0f, id, "[    ]", q2[1]);
            double_field("q22 ", 100.0f, 70.0f, id, "[    ]", q2[2]);
            double_field("q23 ", 100.0f, 70.0f, id, "[    ]", q2[3]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Angular velocities");

        //Angular velocities reference frames (C.O.M. or corresponding body frame).
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *omega_frame[2] = {"Inertial frame", "Body frames"}; //Which frame for the angular velocities.
                ImGui::Combo("  ", (int*)(&angvel_frame), omega_frame, IM_ARRAYSIZE(omega_frame));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (angvel_frame == INERTIAL)
        {
            double_field("ω1ix ", 100.0f, 70.0f, id, "[rad/sec]", w1i[0]);
            double_field("ω1iy ", 100.0f, 70.0f, id, "[rad/sec]", w1i[1]);
            double_field("ω1iz ", 100.0f, 70.0f, id, "[rad/sec]", w1i[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("ω2ix ", 100.0f, 70.0f, id, "[rad/sec]", w2i[0]);
            double_field("ω2iy ", 100.0f, 70.0f, id, "[rad/sec]", w2i[1]);
            double_field("ω2iz ", 100.0f, 70.0f, id, "[rad/sec]", w2i[2]);
        }
        else //angvel_frame == BODY
        {
            double_field("ω1bx ", 100.0f, 70.0f, id, "[rad/sec]", w1b[0]);
            double_field("ω1by ", 100.0f, 70.0f, id, "[rad/sec]", w1b[1]);
            double_field("ω1bz ", 100.0f, 70.0f, id, "[rad/sec]", w1b[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("ω2bx ", 100.0f, 70.0f, id, "[rad/sec]", w2b[0]);
            double_field("ω2by ", 100.0f, 70.0f, id, "[rad/sec]", w2b[1]);
            double_field("ω2bz ", 100.0f, 70.0f, id, "[rad/sec]", w2b[2]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        //C.O.M. initial position and velocity.
        ImGui::Text("C.O.M. motion relative to world");
        double_field("x ",  100.0f, 70.0f, id, "[km]",     r_com[0]);
        double_field("y ",  100.0f, 70.0f, id, "[km]",     r_com[1]);
        double_field("z ",  100.0f, 70.0f, id, "[km]",     r_com[2]);
        double_field("υx ", 100.0f, 70.0f, id, "[km/sec]", v_com[0]);
        double_field("υy ", 100.0f, 70.0f, id, "[km/sec]", v_com[1]);
        double_field("υz ", 100.0f, 70.0f, id, "[km/sec]", v_com[2]);

        ImGui::Unindent();

        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Collision choice logic.
        ImGui::Text("Collision shapes");
        if (ImGui::Checkbox("No collision  (1/r singularity risk)", &collision_no))
            collision_spheres = collision_polyhedra = false;
        if (ImGui::Checkbox("Spheres", &collision_spheres))
            collision_no = collision_polyhedra = false;
        if (ImGui::Checkbox("Polyhedra  (slow for high-res meshes)", &collision_polyhedra))
            collision_no = collision_spheres = false;

        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Kinetic impactors logic.
        ImGui::Text("Kinetic impactors");
        if (ImGui::Checkbox("Assume impactors at Bodies 1 & 2", &impactors_checkbox) && impactors_checkbox)
            impactors_clicked_ok = false;
        if (impactors_checkbox && !impactors_clicked_ok)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, 0.4f*ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Impactors' parameters", &impactors_checkbox);

            //Radiobuttons logic : At least one will always be active and to this, (the active one) the impactor's parameters shall correspond.
            static int impactor_refers_to_body = 1;
            if (ImGui::RadioButton("Body 1", impactor_refers_to_body == 1))
                impactor_refers_to_body = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Body 2", impactor_refers_to_body == 2))
                impactor_refers_to_body = 2;
            ImGui::Dummy(ImVec2(0.0f,5.0f));

            //Impactor menu.
            if (impactor_refers_to_body == 1)
            {
                ImGui::Text("Mass (dry + fuel)");
                double_field("m1 ", 100.0f, 40.0f, id, "[kg]", mD1);
                ImGui::Dummy(ImVec2(0.0f,15.0f));
                ImGui::Text("Velocity (relative to body 1)");
                double_field("υx1 ", 100.0f, 40.0f, id, "[km/sec]", vD1[0]);
                double_field("υy1 ", 100.0f, 40.0f, id, "[km/sec]", vD1[1]);
                double_field("υz1 ", 100.0f, 40.0f, id, "[km/sec]", vD1[2]);
                ImGui::Dummy(ImVec2(0.0f,15.0f));
                ImGui::Text("Momentum enhancement factor (ejecta)");
                double_field("β1 ", 100.0f, 40.0f, id, "[  ]", beta1);
                ImGui::Dummy(ImVec2(0.0f,15.0f));
                ImGui::Text("Impact epoch");
                double_field("t1 ", 100.0f, 40.0f, id, "[days]", tD1);
            }
            else
            {
                ImGui::Text("Mass (dry + fuel)");
                double_field("m2 ", 100.0f, 40.0f, id, "[kg]", mD2);
                ImGui::Dummy(ImVec2(0.0f,15.0f));
                ImGui::Text("Velocity (relative to body 2)");
                double_field("υx2 ", 100.0f, 40.0f, id, "[km/sec]", vD2[0]);
                double_field("υy2 ", 100.0f, 40.0f, id, "[km/sec]", vD2[1]);
                double_field("υz2 ", 100.0f, 40.0f, id, "[km/sec]", vD2[2]);
                ImGui::Dummy(ImVec2(0.0f,15.0f));
                ImGui::Text("Momentum enhancement factor (ejecta)");
                double_field("β2 ", 100.0f, 40.0f, id, "[  ]", beta2);
                ImGui::Dummy(ImVec2(0.0f,15.0f));
                ImGui::Text("Impact epoch");
                double_field("t2 ", 100.0f, 40.0f, id, "[days]", tD2);
            }
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "OK" button. This must be pressed, otherwise the impactor values will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                impactors_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Spacecraft orbiter logic.
        ImGui::Text("Spacecraft orbiter");
        if (ImGui::Checkbox("Assume spacecraft orbiter", &spacecraft_checkbox) && spacecraft_checkbox)
            spacecraft_clicked_ok = false;
        if (spacecraft_checkbox && !spacecraft_clicked_ok)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, 0.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(0.15f*ImGui::GetIO().DisplaySize.x, 0.4f*ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Orbiter's initial state", &spacecraft_checkbox);

            ImGui::Text("Position (relative to C.O.M.)");
            double_field("xs ", 100.0f, 40.0f, id, "[km]", r_sp[0]);
            double_field("ys ", 100.0f, 40.0f, id, "[km]", r_sp[1]);
            double_field("zs ", 100.0f, 40.0f, id, "[km]", r_sp[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Velocity (relative to C.O.M.)");
            double_field("υxs ", 100.0f, 40.0f, id, "[km/sec]", v_sp[0]);
            double_field("υys ", 100.0f, 40.0f, id, "[km/sec]", v_sp[1]);
            double_field("υzs ", 100.0f, 40.0f, id, "[km/sec]", v_sp[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "OK" button. This must be pressed, otherwise the spacecraft's i.c. will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                spacecraft_clicked_ok = true;

            ImGui::End();
        }

        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Run/Abort buttons rendering logic.
        ImGui::Text("Simulation controls");

        if (!task_is_running.load()) //In this case a task is NOT currently running, hence "Run" can be pressed, but "Abort", cannot be pressed (nothing to abort).
        {
            if (ImGui::Button("Run", ImVec2(70.0f, 25.0f)))
                run_pressed = true;
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::Button("Abort", ImVec2(70.0f, 25.0f));
            ImGui::EndDisabled();
        }
        else //Now the opposite happens. "Run" is disabled, coz a task is running and "Abort" is enabled, so that one may stop the running task (integration).
        {
            ImGui::BeginDisabled();
            ImGui::Button("Run", ImVec2(70.0f, 25.0f));
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Abort", ImVec2(70.0f, 25.0f)))
                abort_pressed = true;
        }

        //Progress bar.
        ImGui::Text("Integrator progress");
        if (task_was_aborted.load())
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f,0.0f,0.0f, 1.0f)); //Red.
        else
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f,0.7f,0.0f, 1.0f)); //Green.
        ImGui::ProgressBar(task_progress.load(), ImVec2(150.0f,17.0f));
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f,700.0f)); //Some extra y-space in order to be able to scroll down comfortably along the properties panel.

        ImGui::End();
    }
};

#endif
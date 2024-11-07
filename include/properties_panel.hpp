#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<filesystem>
#include<atomic>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"conversion.hpp"

class properties_panel
{
public:
    char sim_name[51]; //'Simulation name' text field. 50 characters available (plus the '\0' terminating character).
    
    bool ell_checkbox; //'Ellipsoids' checkbox state.
    dvec3 semiaxes1, semiaxes2; //Ellipsoids 'a1', 'b1', 'c1', 'a2', 'b2', 'c2' double fields.
    bool ell_clicked_ok; //Ellipsoids 'OK' button (pressed or not).

    bool obj_checkbox; //'.obj file' checkbox state.
    int obj1_clicked_index, obj2_clicked_index; //Index of the clicked .obj path. -1 means no path is clicked. Only one path per body can be clicked.
    bool obj1_clicked, obj2_clicked; //Decide if an obj file (from 'Body 1' or 'Body 2') is clicked.
    str obj1_path, obj2_path; //Relative paths to the 2 .obj models.
    bool obj_clicked_ok; //.obj 'OK' button (pressed or not).

    bool ord2_checkbox, ord3_checkbox, ord4_checkbox; //'Mutual potential' available options. Only one of them may be chosen (or none, but it will produce an error in the console :P).

    double M1, M2; //'M1', 'M2' double fields (referring to 'Body 1' and 'Body 2' respectively).

    double epoch, dur, step; //'Epoch', 'Duration', 'Step' double fields.

    int cart_kep_var_choice; //Initial choice. 0 -> Cartesian, 1 -> Keplerian.
    dvec6 cart; //'x', 'y', 'z', 'υx', 'υy', 'υz' double fields.
    dvec6 kep; //'a', 'e', 'i', 'Ω', 'ω', 'M' double fields.

    int orient_var_choice; //Initial choice. 0 -> Euler angles (roll, pitch, yaw), 1 -> Quaternions.
    dvec3 rpy1, rpy2; //'roll 1', 'pitch 1', 'yaw 1', 'roll 2', 'pitch 2', 'yaw 2' double fields.
    dvec4 q1, q2; //'q10', 'q11', 'q12', 'q13', 'q20', 'q21', 'q22', 'q23' double fields.

    int frame_type_choice; //Initial choice. 0 -> Global inertial frame, 1 -> Corresponding body frames.
    dvec3 w1i, w2i, w1b, w2b; //'ω1x', 'ω1y', 'ω1z', 'ω2x', 'ω2y, 'ω2z' double fields (nature of the frame depends on 'frame_type_choice').

    bool impactor_checkbox; //'Kinetic impactor' checkbox state.
    bool impactor_clicked_ok; //'OK' button in the kinetic impactor parameters window (pressed or not).
    double M_impact; //Impactor's mass.
    dvec3 v_impact; //Impactor's velocity vector.
    double beta; //Momentum enhancement factor.

    bool run_pressed; //Whether or not the 'Run' button has been pressed.
    bool abort_pressed; //Whether or not the 'Abort' button has been pressed.

    properties_panel() : sim_name(""),
                         ell_checkbox(false),
                         semiaxes1(dvec3{0.0,0.0,0.0}),
                         semiaxes2(dvec3{0.0,0.0,0.0}),
                         ell_clicked_ok(false),
                         obj_checkbox(false),
                         obj1_clicked_index(-1),
                         obj2_clicked_index(-1),
                         obj1_clicked(false),
                         obj2_clicked(false),
                         obj1_path(""),
                         obj2_path(""),
                         obj_clicked_ok(false),
                         ord2_checkbox(false),
                         ord3_checkbox(false),
                         ord4_checkbox(false),
                         M1(0.0),
                         M2(0.0),
                         epoch(0.0),
                         dur(0.0),
                         step(0.0),
                         cart_kep_var_choice(0),
                         cart(dvec6{0.0,0.0,0.0,0.0,0.0,0.0}),
                         kep(dvec6{0.0,0.0,0.0,0.0,0.0,0.0,}),
                         orient_var_choice(0),
                         rpy1(dvec3{0.0,0.0,0.0}),
                         rpy2(dvec3{0.0,0.0,0.0}),
                         q1(dvec4{1.0,0.0,0.0,0.0}),
                         q2(dvec4{1.0,0.0,0.0,0.0}),
                         frame_type_choice(0),
                         w1i(dvec3{0.0,0.0,0.0}),
                         w2i(dvec3{0.0,0.0,0.0}),
                         w1b(dvec3{0.0,0.0,0.0}),
                         w2b(dvec3{0.0,0.0,0.0}),
                         impactor_checkbox(false),
                         impactor_clicked_ok(false),
                         M_impact(0.0),
                         v_impact(dvec3{0.0,0.0,0.0}),
                         beta(0.0),
                         run_pressed(false),
                         abort_pressed(false)
    { }

    //This function receives as input a 'path' to a directory and as a result it returns a vector of paths, corresponding
    //to all the .obj files (even inside child directories) found inside 'path'.
    std::vector<std::filesystem::path> list_obj_files(const char *path)
    {
        std::vector<std::filesystem::path> paths; 
        for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
            //Check if the entry is a regular file and has a ".obj" extension.
            if (entry.is_regular_file() && entry.path().extension() == ".obj")
                paths.push_back(entry.path().filename());
        return paths;
    }

    //This function automates common double inputs via the keyboard. It creates a rectangle, inside of which the user may enter a double.
    //'label' is a string written on the left of the rectangle. 'item_width' is the horizontal legth (space) of the rectangle. 'id' is a unique
    //int with which the computer identifies which variable to affect (coz you may have multiple input fields). 'unit' is a string written on the right
    //of the rectangle (for us it is always the unit of measurement of the current variable). 'variable' is the variable itself, passed by reference to
    //InputDouble(), so it may change.
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

    //This member functions processes all the user inputs and checkes if they are valid (assuming some rules).
    //If all inputs are valid, the function returns an entirely empty vector of strings (empty because it contains no erros).
    //Otherwise the returned vector contains string messages, each corresponding to an invalid input. In this case,
    //the vector of strings will be displayed on the console and the simulation will not run.
    strvec validate()
    {
        strvec errors; //This is empty now coz errors.size() is 0.
        
        //Possible error 1 : Simulation name (empty, pure spaces, begin with space, illegal characters).
        str sim_name_copy = sim_name;
        if ( (sim_name_copy.empty()) || (sim_name_copy.find_first_not_of(' ') == str::npos) || (sim_name_copy[0] == ' ') || (sim_name_copy.find_first_of("<>:\"/\\|?*") != str::npos) )
            errors.push_back("[Error] :  'Simulation name' is invalid.");

        //Possible error 2 : Shape model checkboxes (at least one must be checked when the 'Run' button has been pressed).
        if (!ell_checkbox && !obj_checkbox)
            errors.push_back("[Error] :  Neither 'Ellipsoids', nor '.obj files' is selected for determining the shapes.");

        //Possible error 3 : 'OK' button in the Elliposid parameters window (it must be clicked so that the parameters are taken into account).
        if (ell_checkbox && !ell_clicked_ok)
            errors.push_back("[Error] :  'OK' button must be pressed in the 'Ellipsoid parameters' window.");

        //Possible error 4 : 'OK' button in the '.obj files' window (it must be clicked so that the .obj files are taken into account).
        if (obj_checkbox && !obj_clicked_ok)
            errors.push_back("[Error] :  'OK' button must be pressed in the '.obj files' window.");

        //Possible error 5 : Ellipsoids semiaxes (all semiaxes must be > 0).
        if (ell_checkbox)
        {
            if (semiaxes1[0] <= 0.0 || semiaxes1[1] <= 0.0 || semiaxes1[2] <= 0.0)
                errors.push_back("[Error] :  'a1', 'b1', 'c1' must be positive numbers.");
            if (semiaxes2[0] <= 0.0 || semiaxes2[1] <= 0.0 || semiaxes2[2] <= 0.0)
                errors.push_back("[Error] :  'a2', 'b2', 'c2' must be positive numbers.");
        }

        //Possible error 6 : .obj files (at least one .obj file per body must be selected).
        if (obj_checkbox)
        {
            if (obj1_clicked_index == -1)
                errors.push_back("[Error] :  No .obj file is selected for 'Body 1'.");
            if (obj2_clicked_index == -1)
                errors.push_back("[Error] :  No .obj file is selected for 'Body 2'.");
        }

        //Possible error 8 : Mutual potential checkboxes (at least one must be checked).
        if (!ord2_checkbox && !ord3_checkbox && !ord4_checkbox)
            errors.push_back("[Error] :  Neither 'Order 2', nor 'Order 3', nor 'Order 4' mutual potential is selected.");

        //Possible error 9 : Masses (both M1 and M2 must be > 0).
        if (M1 <= 0.0 || M2 <= 0.0)
            errors.push_back("[Error] :  'M1', 'M2' must be positive numbers.");

        //Possible error 10 : Time parameters ('Epoch' and 'Duration' must be >= 0 and 'Step' must be <= 'Duration').
        if (!(epoch >= 0.0 && dur > 0.0 && step <= dur))
            errors.push_back("[Error] :  Invalid set of 'Epoch', 'Duration', 'Step'.");

        //Possible error 11 : Relative position/velocity (mutual distance must be > 0).
        if (cart_kep_var_choice == 0 && length(dvec3{cart[0], cart[1], cart[2]}) <= 0.0)
            errors.push_back("[Error] :  Invalid set of 'x', 'y', 'z' (mutual distance must be positive).");

        //Possible error 12 : Relative Keplerian elements ('a' must be > 0, 'e' must be in [0,1))
        //Note : 'e' can actually become >= 1 and handled, but first we need to extend the functions cart2kep() and kep2cart() a little bit (maybe later...).
        if (cart_kep_var_choice == 1)
        {
            if (kep[0] <= 0.0)
                errors.push_back("[Error] :  Semi-major axis 'a' must be positive.");
            if (kep[1] < 0.0 || kep[1] >= 1.0)
                errors.push_back("[Error] :  Eccentricity 'e' must be in [0,1).");
            if (kep[2] < 0.0 || kep[2] >= 360.0)
                errors.push_back("[Error] :  Inclination 'i' must be in [0,360).");
            if (kep[3] < 0.0 || kep[3] >= 360.0)
                errors.push_back("[Error] :  Longitude of ascending node 'Ω' must be in [0,360).");
            if (kep[4] < 0.0 || kep[4] >= 360.0)
                errors.push_back("[Error] :  Argument of periapsis 'ω' must be in [0,360).");
            if (kep[5] < 0.0 || kep[5] >= 360.0)
                errors.push_back("[Error] :  Mean anomaly 'M' must be in [0,360).");
        }

        //Possible error 13 : Quaternion (both must be nonzero).
        //Note : In case of non normalized quaternion input, the program normalizes them both automatically.
        if (orient_var_choice == 1)
        {
            if (length(q1) <= machine_zero)
                errors.push_back("[Error] :  Quaternion 1 ('q10', 'q11', 'q12', 'q13') must be nonzero.");
            else //Normalize it no matter what.
                q1 = quat2unit(q1); //This will be visible in the gui.

            //The same for q2 :
            if (length(q2) <= machine_zero)
                errors.push_back("[Error] :  Quaternion 2 ('q20', 'q21', 'q22', 'q23') must be nonzero.");
            else //Normalize it no matter what.
                q2 = quat2unit(q2); //This will be visible in the gui.
        }

        //Possible error 14 : 'OK' button in the Elliposid parameters window (it must be clicked so that the parameters are taken into account).
        if (impactor_checkbox && !impactor_clicked_ok)
            errors.push_back("[Error] :  'OK' button must be pressed in the 'Impactor parameters' window.");

        //Possible error 15 : Impactor's parameters (mass must be >= 0).
        if (impactor_checkbox && M_impact < 0.0)
            errors.push_back("[Error] : Impactor's 'Mass' must be non negative.");
   
        return errors;
    }

    //This is the function that draws the properties panel and processes the corresponding logic.
    void render(std::atomic<bool> simulation_is_running, std::atomic<bool> simulation_was_aborted, std::atomic<float> simulation_progress)
    {
        //Reinitialized every frame at 0. Making it static, will also work, but if the app's total frames (glfw while loop) exceed the
        //maximum int value (or unsigned, or long, or whatever the variable type of id is), then we will have an overflow, which means
        //unexpected behavior or crash or wrap around to the negative side...
        int id = 0;

        //Properties panel "main" window.
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties", nullptr);

        //Simunlation name text field. Basically this is the name of the folder that will be created later, holding the orbit data.
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

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 320.0f), ImGuiCond_FirstUseEver); 
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

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 320.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj files", &obj_checkbox);

            //Radiobuttons logic : At least one will always be active and to this (the active one) the loaded obj file will correspond.
            static int obj_refers_to_body = 1; //To which body ('Body 1' or 'Body 2') does the obj file listing refer to (via radiobutton). 'Body 1' is the default choice.
            if (ImGui::RadioButton("Body 1", obj_refers_to_body == 1))
                obj_refers_to_body = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Body 2", obj_refers_to_body == 2))
                obj_refers_to_body = 2;
            ImGui::Dummy(ImVec2(0.0f,5.0f));

            //File list logic.
            static std::vector<std::filesystem::path> all_obj_files = list_obj_files("../obj/"); //Store all the .obj files located in the obj/ directory.
            if (ImGui::TreeNodeEx("Available .obj files in obj/ directory :"))
            {
                for (size_t i = 0; i < all_obj_files.size(); ++i)
                {
                    //Which .obj path for Body 1.
                    if (obj_refers_to_body == 1)
                    {
                        if (ImGui::Selectable(all_obj_files[i].string().c_str(), (obj1_clicked_index == (int)i)))
                        {
                            obj1_clicked = true;
                            obj1_clicked_index = i;
                            obj1_path = all_obj_files[i].string();
                        }
                    }
                    else
                    {
                        //Which .obj path for Body 2.
                        if (ImGui::Selectable(all_obj_files[i].string().c_str(), (obj2_clicked_index == (int)i)))
                        {
                            obj2_clicked = true;
                            obj2_clicked_index = i;
                            obj2_path = all_obj_files[i].string();
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

        //Time parameters.
        ImGui::Text("Integration time");
        double_field("Epoch ",    100.0f, 70.0f, id, "[days]", epoch);
        double_field("Duration ", 100.0f, 70.0f, id, "[days]", dur);
        double_field("Step ",     100.0f, 70.0f, id, "[days]", step);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Initial state");
        ImGui::Indent();

        ImGui::Text("Relative position and velocity");

        //Initial position/velocity variables, either in the form of Cartesian coords, or Keplerian elements.
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *cart_kep_var[2] = {"Cartesian", "Keplerian"}; //Nature of the relative position and velocity variables.
                ImGui::Combo("  ", &cart_kep_var_choice, cart_kep_var, IM_ARRAYSIZE(cart_kep_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (cart_kep_var_choice == 0)
        {
            double_field("x ",  100.0f, 55.0f, id, "[km]",     cart[0]);
            double_field("y ",  100.0f, 55.0f, id, "[km]",     cart[1]);
            double_field("z ",  100.0f, 55.0f, id, "[km]",     cart[2]);
            double_field("υx ", 100.0f, 55.0f, id, "[km/sec]", cart[3]);
            double_field("υy ", 100.0f, 55.0f, id, "[km/sec]", cart[4]);
            double_field("υz ", 100.0f, 55.0f, id, "[km/sec]", cart[5]);
        }
        else //cart_kep_var_choice is 1, thus render the Keplerian elements.
        {
            double_field("a ", 100.0f, 55.0f, id, "[km]",  kep[0]);
            double_field("e ", 100.0f, 55.0f, id, "[  ]",  kep[1]);
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
                static const char *orient_var[2] = {"Euler angles (XYZ)", "Quaternions (WXYZ)"}; //Nature of the orientation variables.
                ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (orient_var_choice == 0)
        {
            double_field("roll 1 " , 100.0f, 80.0f, id, "[deg]", rpy1[0]);
            double_field("pitch 1 ", 100.0f, 80.0f, id, "[deg]", rpy1[1]);
            double_field("yaw 1 ",   100.0f, 80.0f, id, "[deg]", rpy1[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("roll 2 ",  100.0f, 80.0f, id, "[deg]", rpy2[0]);
            double_field("pitch 2 ", 100.0f, 80.0f, id, "[deg]", rpy2[1]);
            double_field("yaw 2 ",   100.0f, 80.0f, id, "[deg]", rpy2[2]);
        }
        else //orient_var_choice is 1, thus render the quaternion format.
        {
            double_field("q10 ", 100.0f, 70.0f, id, "[  ]", q1[0]);
            double_field("q11 ", 100.0f, 70.0f, id, "[  ]", q1[1]);
            double_field("q12 ", 100.0f, 70.0f, id, "[  ]", q1[2]);
            double_field("q13 ", 100.0f, 70.0f, id, "[  ]", q1[3]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("q20 ", 100.0f, 70.0f, id, "[  ]", q2[0]);
            double_field("q21 ", 100.0f, 70.0f, id, "[  ]", q2[1]);
            double_field("q22 ", 100.0f, 70.0f, id, "[  ]", q2[2]);
            double_field("q23 ", 100.0f, 70.0f, id, "[  ]", q2[3]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Angular velocities");

        //Angular velocities reference frames (inertial/world or corresponding body).
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *frame_type[2] = {"At inertial frame", "At body frames"}; //Which frame for the angular velocities.
                ImGui::Combo("  ", &frame_type_choice, frame_type, IM_ARRAYSIZE(frame_type));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (frame_type_choice == 0)
        {
            double_field("ω1x ", 100.0f, 70.0f, id, "[rad/sec]", w1i[0]);
            double_field("ω1y ", 100.0f, 70.0f, id, "[rad/sec]", w1i[1]);
            double_field("ω1z ", 100.0f, 70.0f, id, "[rad/sec]", w1i[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("ω2x ", 100.0f, 70.0f, id, "[rad/sec]", w2i[0]);
            double_field("ω2y ", 100.0f, 70.0f, id, "[rad/sec]", w2i[1]);
            double_field("ω2z ", 100.0f, 70.0f, id, "[rad/sec]", w2i[2]);
        }
        else //frame_type_choice is 1, thus assume individual body frames.
        {
            double_field("ω1x ", 100.0f, 70.0f, id, "[rad/sec]", w1b[0]);
            double_field("ω1y ", 100.0f, 70.0f, id, "[rad/sec]", w1b[1]);
            double_field("ω1z ", 100.0f, 70.0f, id, "[rad/sec]", w1b[2]);
            ImGui::Dummy(ImVec2(0.0f,5.0f));
            double_field("ω2x ", 100.0f, 70.0f, id, "[rad/sec]", w2b[0]);
            double_field("ω2y ", 100.0f, 70.0f, id, "[rad/sec]", w2b[1]);
            double_field("ω2z ", 100.0f, 70.0f, id, "[rad/sec]", w2b[2]);
        }
        ImGui::Unindent();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));


        //Kinetic impactor logic.
        ImGui::Text("Kinetic impactor");
        if (ImGui::Checkbox("Assume impactor at Body 2", &impactor_checkbox) && impactor_checkbox)
            impactor_clicked_ok = false;
        if (impactor_checkbox && !impactor_clicked_ok)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 320.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Impactor parameters", &impactor_checkbox);

            //Impactor menu.
            ImGui::Text("Mass (dry + fuel)");
            double_field("m ", 100.0f, 30.0f, id, "[kg]", M_impact);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Velocity (inertial)");
            double_field("υx ", 100.0f, 30.0f, id, "[km/sec]", v_impact[0]);
            double_field("υy ", 100.0f, 30.0f, id, "[km/sec]", v_impact[1]);
            double_field("υz ", 100.0f, 30.0f, id, "[km/sec]", v_impact[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Momentum enhancement factor (ejecta)");
            double_field("β ", 100.0f, 30.0f, id, "[  ]", beta);
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "OK" button. This must be pressed, otherwise the impactor values will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                impactor_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Run/Abort buttons rendering logic.
        ImGui::Text("Simulation controls");

        if (!simulation_is_running.load()) //In this case the simulation is NOT currently running, hence "Run" can be pressed (to start), but "Abort", cannot be pressed (nothing to abort).
        {
            if (ImGui::Button("Run", ImVec2(70.0f, 25.0f)))
                run_pressed = true;
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::Button("Abort", ImVec2(70.0f, 25.0f));
            ImGui::EndDisabled();
        }
        else //Now the opposite happens. "Run" is disabled coz the simulation is running and "Abort" is enabled, so that one may stop the running.
        {
            ImGui::BeginDisabled();
            ImGui::Button("Run", ImVec2(70.0f, 25.0f));
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Abort", ImVec2(70.0f, 25.0f)))
                abort_pressed = true;
        }

        //Progress bar.
        ImGui::Text("Simulation progress");
        if (simulation_was_aborted.load())
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f,0.0f,0.0f, 1.0f)); //Red.
        else
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f,0.7f,0.0f, 1.0f)); //Green.
        ImGui::ProgressBar(simulation_progress.load(), ImVec2(150.0f,20.0f));
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f,700.0f)); //Some extra y-space in order to be able to scroll down along properties panel.

        ImGui::End();
    }
};

#endif
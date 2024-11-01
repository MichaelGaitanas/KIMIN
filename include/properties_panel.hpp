#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<filesystem>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"obj.hpp"
#include"conversion.hpp"

class properties_panel
{
private:
    char sim_name[101]; //'Simulation name' text field. 100 characters available (plus the '\0' terminating character).
    
    bool ell_checkbox; //'Ellipsoids' checkbox state.
    bool ell_win_closable; //Whether or not the 'Ellipsoid parameters' window is closable.
    dvec3 semiaxes1, semiaxes2; //Ellipsoids 'a1', 'b1', 'c1', 'a2', 'b2', 'c2' double fields.
    bool ell_clicked_ok; //Ellipsoids 'OK' button (pressed or not).

    bool obj_checkbox; //'.obj file' checkbox state.
    bool obj_win_closable; //Whether or not the '.obj files' is closable.
    int obj_refers_to_body; //To which body ('Body 1' or 'Body 2') does the obj file listing refer to (via radiobutton).
    std::vector<std::filesystem::path> path_to_obj_dir; //Relative path to the obj models directory.
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

    //'ω1x', 'ω1y', 'ω1z', 'ω2x', 'ω2y, 'ω2z' fields (inertial or body frame).
    dvec3 w1i, w2i;
    dvec3 w1b, w2b;

    bool impactor_checkbox; //'Kinetic impactor' checkbox state.
    bool impactor_clicked_ok; //'OK' button in the kinetic impactor parameters window (pressed or not).
    dvec3 v_impact; //Impactor's velocity vector.
    double M_impact; //Impactor's mass.
    double beta; //Momentum enhancement factor.

    //fundamental contents of the 2 .obj files (vertices, faces).
    //bvec vf1, vf2;

public:
    properties_panel() : sim_name(""),
                         ell_checkbox(false),
                         ell_win_closable(true),
                         semiaxes1(dvec3{0.0,0.0,0.0}),
                         semiaxes2(dvec3{0.0,0.0,0.0}),
                         ell_clicked_ok(false),
                         obj_checkbox(false),
                         obj_win_closable(true),
                         obj_refers_to_body(1),
                         path_to_obj_dir(list_obj_files("../obj/")),
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
                         cart_kep_var_choice(1),
                         cart(dvec6{0.0,0.0,0.0,0.0,0.0,0.0}),
                         kep(dvec6{0.0,0.0,0.0,0.0,0.0,0.0,}),
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
                         impactor_checkbox(false),
                         impactor_clicked_ok(false),
                         v_impact(dvec3{0.0,0.0,0.0}),
                         M_impact(0.0),
                         beta(0.0)
                         //vf1({false, false}),
                         //vf2({false, false})
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

    //This function automates common double input via the keyboard. It creates a rectangle, inside of which the user may enter a double.
    //'label' is a string written on the left of the rectangle. 'item_width' is the horizontal legth (space) of the rectangle. 'id' is a unique
    //int with which the computer identifies which variable to affect (coz you may have multiple input fields). 'unit' is a string written on the right
    //of the rectangle (for us it is always the unit of measurement of the current variable). 'variable' is the variable itself, passed by reference to
    //InputDouble(), so it may change
    void double_field(const char *label, const float item_width, int &id, const char *unit, double &variable)
    {      
        ImGui::Text(label);
        ImGui::SameLine();
        ImGui::PushItemWidth(item_width);
            ImGui::PushID(id++);
                ImGui::InputDouble(unit, &variable, 0.0, 0.0, "%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
    }

    //Draw the properties panel and process the corresponding logic.
    void render()
    {
        //Reinitialized every frame at 0. Making it static, will also work, but if the app's total frames (glfw while loop) exceed the
        //maximum int value (or unsigned, or long, or whatever the variable type of id is), then we will have an overflow, which means
        //unexpected behavior or crash or wrap around to the negative side...
        int id = 0;

        //Properties panel "main" window.
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties ", nullptr);

        //Simunlation name text field. Basically this is the name of the folder that will be created later, holding the orbit data.
        ImGui::Text("Simulation name");
        ImGui::PushItemWidth(200.0f);
            ImGui::InputText(" ", sim_name, IM_ARRAYSIZE(sim_name));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Ellipsoid and .obj shape logic.
        ImGui::Text("Shape model");


        //Ellipsoid shape logic.
        if (ImGui::Checkbox("Ellipsoids", &ell_checkbox) && ell_checkbox)
            ell_clicked_ok = false;
        if (ell_checkbox && !ell_clicked_ok)
        {
            obj_checkbox = false; //Untick the obj checkbox in case it is ticked.

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 300.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters", &ell_checkbox);
            
            //Ellipsoids semiaxes menu.
            ImGui::Text("Body 1 semi - axes");
            double_field("a1 ", 100.0f, id, "[km]", semiaxes1[0]);
            double_field("b1 ", 100.0f, id, "[km]", semiaxes1[1]);
            double_field("c1 ", 100.0f, id, "[km]", semiaxes1[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Body 2 semi - axes");
            double_field("a2 ", 100.0f, id, "[km]", semiaxes2[0]);
            double_field("b2 ", 100.0f, id, "[km]", semiaxes2[1]);
            double_field("c2 ", 100.0f, id, "[km]", semiaxes2[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "OK" button. This must be pressed, otherwise the semi-axes values will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                ell_clicked_ok = true;

            ImGui::End();
        }
    

        //.obj shape logic.
        if (ImGui::Checkbox(".obj file", &obj_checkbox) && obj_checkbox)
            obj_clicked_ok = false;
        if (obj_checkbox && !obj_clicked_ok)
        {
            ell_checkbox = false; //Untick the ellipsoids checkbox in case it is ticked.

            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 300.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj files", &obj_checkbox);

            //Radiobuttons logic : At least one will always be active and to this (the active one) the loaded obj file will correspond.
            if (ImGui::RadioButton("Body 1", obj_refers_to_body == 1))
                obj_refers_to_body = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Body 2", obj_refers_to_body == 2))
                obj_refers_to_body = 2;
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            
            
            //File list logic.
            if (ImGui::TreeNodeEx("Available .obj files in obj/ directory :"))
            {
                for (size_t i = 0; i < path_to_obj_dir.size(); ++i)
                {
                    //Which .obj path for Body 1.
                    if (obj_refers_to_body == 1)
                    {
                        if (ImGui::Selectable(path_to_obj_dir[i].string().c_str(), (obj1_clicked_index == (int)i)))
                        {
                            obj1_clicked = true;
                            obj1_clicked_index = i;
                            obj1_path = path_to_obj_dir[i].string();
                        }
                    }
                    else
                    {
                        //Which .obj path for Body 2.
                        if (ImGui::Selectable(path_to_obj_dir[i].string().c_str(), (obj2_clicked_index == (int)i)))
                        {
                            obj2_clicked = true;
                            obj2_clicked_index = i;
                            obj2_path = path_to_obj_dir[i].string();
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
        ImGui::Text("Mass");
        double_field("M1 ", 150.0f, id, "[kg]", M1);
        double_field("M2 ", 150.0f, id, "[kg]", M2);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        //Time parameters.
        ImGui::Text("Integration time");
        double_field("Epoch     ",   100.0f, id, "[days]", epoch);
        double_field("Duration ",    100.0f, id, "[days]", dur);
        double_field("Step        ", 100.0f, id, "[days]", step);
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));

        ImGui::Text("Initial state");
        ImGui::Indent();
        ImGui::Text("Relative position and velocity");

        //Initial position/velocity variables, either in the form of Cartesian coords, or Keplerian elements.
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                static const char *cart_kep_var[2] = {"Cartesian ", "Keplerian "}; //Nature of the relative position and velocity variables.
                ImGui::Combo("  ", &cart_kep_var_choice, cart_kep_var, IM_ARRAYSIZE(cart_kep_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (cart_kep_var_choice == 0)
        {
            double_field("x    ", 100.0f, id, "[km]",     cart[0]);
            double_field("y    ", 100.0f, id, "[km]",     cart[1]);
            double_field("z    ", 100.0f, id, "[km]",     cart[2]);
            double_field("υx  " , 100.0f, id, "[km/sec]", cart[3]);
            double_field("υy  " , 100.0f, id, "[km/sec]", cart[4]);
            double_field("υz  " , 100.0f, id, "[km/sec]", cart[5]);
        }
        else //cart_kep_var_choice is 1...
        {
            double_field("a     ",  100.0f, id, "[km]",  kep[0]);
            double_field("e     ",  100.0f, id, "[  ]",  kep[1]);
            double_field("i      ", 100.0f, id, "[deg]", kep[2]);
            double_field("Ω    ",   100.0f, id, "[deg]", kep[3]);
            double_field("ω   ",    100.0f, id, "[deg]", kep[4]);
            double_field("M   ",    100.0f, id, "[deg]", kep[5]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));






        ImGui::Text("Orientation");

        //orientation variables (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (orient_var_choice == 0)
        {
            double_field("roll 1    " , 100.0f, id, "[deg]", rpy1[0]);
            double_field("pitch 1 "   , 100.0f, id, "[deg]", rpy1[1]);
            double_field("yaw 1   "   , 100.0f, id, "[deg]", rpy1[2]);
            double_field("roll 2    " , 100.0f, id, "[deg]", rpy2[0]);
            double_field("pitch 2 "   , 100.0f, id, "[deg]", rpy2[1]);
            double_field("yaw 2  "    , 100.0f, id, "[deg]", rpy2[2]);
        }
        else if (orient_var_choice == 1)
        {
            double_field("q10   " , 100.0f, id, "[  ]", q1[0]);
            double_field("q11   " , 100.0f, id, "[  ]", q1[1]);
            double_field("q12   " , 100.0f, id, "[  ]", q1[2]);
            double_field("q13   " , 100.0f, id, "[  ]", q1[3]);
            double_field("q20   " , 100.0f, id, "[  ]", q2[0]);
            double_field("q21   " , 100.0f, id, "[  ]", q2[1]);
            double_field("q22   " , 100.0f, id, "[  ]", q2[2]);
            double_field("q23   " , 100.0f, id, "[  ]", q2[3]);
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Angular velocity");

        //angular velocities reference frames (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
                ImGui::Combo("  ", &frame_type_choice, frame_type, IM_ARRAYSIZE(frame_type));
            ImGui::PopID();
        ImGui::PopItemWidth();
        if (frame_type_choice == 0)
        {
            double_field("ω1x   " , 100.0f, id, "[rad/sec]", w1i[0]);
            double_field("ω1y   " , 100.0f, id, "[rad/sec]", w1i[1]);
            double_field("ω1z   " , 100.0f, id, "[rad/sec]", w1i[2]);
            double_field("ω2x   " , 100.0f, id, "[rad/sec]", w2i[0]);
            double_field("ω2y   " , 100.0f, id, "[rad/sec]", w2i[1]);
            double_field("ω2z   " , 100.0f, id, "[rad/sec]", w2i[2]);
        }
        else if (frame_type_choice == 1)
        {
            double_field("ω1x   " , 100.0f, id, "[rad/sec]", w1b[0]);
            double_field("ω1y   " , 100.0f, id, "[rad/sec]", w1b[1]);
            double_field("ω1z   " , 100.0f, id, "[rad/sec]", w1b[2]);
            double_field("ω2x   " , 100.0f, id, "[rad/sec]", w2b[0]);
            double_field("ω2y   " , 100.0f, id, "[rad/sec]", w2b[1]);
            double_field("ω2z   " , 100.0f, id, "[rad/sec]", w2b[2]);
        }
        ImGui::Unindent();
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,7.5f));


        //Kinetic impactor logic.
        ImGui::Text("Kinetic impactor");
        if (ImGui::Checkbox("Assume impactor", &impactor_checkbox) && impactor_checkbox)
            impactor_clicked_ok = false;
        if (impactor_checkbox && !impactor_clicked_ok)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 300.0f), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 300.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Impactor parameters", &impactor_checkbox);

            //Impactor menu.
            ImGui::Text("Velocity");
            double_field("υx ", 100.0f, id, "[km/sec]", v_impact[0]);
            double_field("υy ", 100.0f, id, "[km/sec]", v_impact[1]);
            double_field("υz ", 100.0f, id, "[km/sec]", v_impact[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Mass");
            double_field("M ", 100.0f, id, "[kg]", M_impact);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            ImGui::Text("Momentum enhancement factor");
            double_field("β ", 100.0f, id, "[  ]", beta);
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //Final "OK" button. This must be pressed, otherwise the impactor values will not be taken into account.
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                impactor_clicked_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f,7.5f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f,700.0f)); //Extra y-space in order to be able to scroll freely.

        ImGui::End();
    }

    //If all user inputs are valid, this member function returns an entirely empty vector of strings.
    //Otherwise the returned vector contains string messages, each corresponding to an invalid input. In this case,
    //the vector of strings will be displayed on the console and the simulation will not run.
    strvec validate()
    {
        strvec errors;
        
        //sim_name[] errors (empty, pure tabs, begin with tab).
        str sim_name_copy = sim_name;
        char first_char = sim_name_copy[0];
        char one_space = ' ';
        if ( (sim_name_copy.empty()) || (sim_name_copy.find_first_not_of(' ') == str::npos) || (first_char == one_space) )
            errors.push_back("[Error] :  'Simulation name' is invalid.");

        //Theory model checkboxes error (at least one must be checked).
        if (!ord2_checkbox && !ord3_checkbox && !ord4_checkbox)
            errors.push_back("[Error] :  Neither 'Order 2', nor 'Order 3', nor 'Order 4' was selected as theory.");

        //Shape model checkboxes error (at least one must be checked).
        if (!ell_checkbox && !obj_checkbox)
            errors.push_back("[Error] :  Neither 'Ellipsoids', nor '.obj files' was selected  for determining the shapes.");

        //Ellipsoids semiaxes error (all semiaxes must be > 0.0).
        if (ell_checkbox && (semiaxes1[0] <= 0.0 || semiaxes1[1] <= 0.0 || semiaxes1[2] <= 0.0))
            errors.push_back("[Error] :  'a1', 'b1', 'c1' must be positive numbers.");
        if (ell_checkbox && (semiaxes2[0] <= 0.0 || semiaxes2[1] <= 0.0 || semiaxes2[2] <= 0.0))
            errors.push_back("[Error] :  'a2', 'b2', 'c2' must be positive numbers.");

        //.obj files error (at least one .obj file per body must be selected by the user).
        if (obj_checkbox && obj1_clicked_index == -1)
             errors.push_back("[Error] :  No .obj file is selected for 'Body 1'.");
        if (obj_checkbox && obj2_clicked_index == -1)
             errors.push_back("[Error] :  No .obj file is selected for 'Body 2'.");

        //.obj files error of polyhedron category (the .obj file must at least contain lines with the
        //format 'v x y z' AND 'f i j k' to be assumed as a valid polyhedron.
        if (obj_checkbox && obj1_clicked)
        {
            /*
            vf1 = Obj::vf_status(obj1_path.c_str());
            if ( !(vf1[0] && vf1[1]) )
                errors.push_back("[Error] :  In 'Body 1' .obj file, vertices and faces lines must exist.");
            */
        }
        if (obj_checkbox && obj2_clicked)
        {
            /*
            vf2 = Obj::vf_status(obj2_path.c_str());
            if ( !(vf2[0] && vf2[1]) )
                errors.push_back("[Error] :  In 'Body 2' .obj file, vertices and faces lines must exist.");
            */
        }

        //Masses error (both M1 and M2 must be > 0.0).
        if (M1 <= 0.0)
            errors.push_back("[Error] :  'M1' must be positive.");
        if (M2 <= 0.0)
            errors.push_back("[Error] :  'M2' must be positive.");

        //Impactor's parameters errors (both Mass and β be non negative).
        if (M_impact < 0.0)
            errors.push_back("[Error] : Impactor's 'Mass' must be non negative.");
        if (beta < 0.0)
            errors.push_back("[Error] : 'β value' must be non negative.");

        //Time parameters errors (must : epoch >= 0.0, dur >= 0.0, print_step <= dur)
        if (!(epoch >= 0.0 && dur >= 0.0 && step <= dur))
            errors.push_back("[Error] :  Invalid set of 'Epoch', 'Duration', 'Step'.");

        //Relative position/velocity errors. Here, we assume that only the Keplerian elements 'a','e' might be invalid.
        if (cart_kep_var_choice == 1 && kep[0] <= 0.0)
            errors.push_back("[Error] :  Invalid semi-major axis 'a'.");
        if (cart_kep_var_choice == 1 && kep[1] >= 1.0)
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

        return errors;
    }
};

#endif
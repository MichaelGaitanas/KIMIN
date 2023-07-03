#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

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

class Properties
{

public:

    //'Simulation name' field. 30 characters available (plus the '\0' character).
    char simname[31];

    //'Ellipsoids' checkbox state.
    bool ell_checkbox;
    //Ellipsoids 'OK' button state (from the submenu).
    bool clicked_ell_ok;

    //Ellipsoids {'a1', 'b1', 'c1'}, {'a2', 'b2', 'c2'} fields.
    dvec3 semiaxes1, semiaxes2;

    //'.obj files' checkbox state.
    bool obj_checkbox;
    //.obj files 'Body 1' or 'Body 2' radiobuttons reference (1 or 2 only).
    int obj_refer_to_body;

    //Index of the clicked .obj path. -1 means no path is clicked. Only one path per body can be clicked.
    int clicked_poly1_index, clicked_poly2_index;
    
    //Decide whether body 1 and body 2 will be loaded.
    bool clicked_poly1, clicked_poly2;
    
    //Relative path to obj/ directory.
    std::vector<std::filesystem::path> path_to_poly_obj;

    //Relative path to the 2 .obj models.
    str obj_path1, obj_path2;

    //fundamental contents of the 2 .obj files (vertices, faces).
    bvec vf1, vf2;

    //Cartesian grid resolutions (per axis) for filling the polyhedra with mascons.
    ivec3 grid_reso1, grid_reso2, grid_reso_inactive;

    //.obj 'OK' button state.
    bool clicked_obj_ok;

    //'Theory' checkbox state
    bool ord2_checkbox, ord3_checkbox, ord4_checkbox;

    //'M1', 'M2' fields.
    double M1,M2;

    //Impactor parameters : 'υx', 'υy', 'υz', 'Mass', 'β value'
    dvec3 v_impact;
    double M_impact;
    double beta;

    //'Epoch', 'Duration', 'Step' fields.
    double epoch, dur, step;

    //Nature of the relative position and velocity variables.
    const char *cart_kep_var[2];
    //Initial choice. 0 -> Cartesian, 1 -> Keplerian.
    int cart_kep_var_choice;

    //'x', 'y', 'z', 'υx', 'υy', 'υz' fields.
    dvec6 cart;
    //'a', 'e', 'i', 'Ω', 'ω', 'M' fields.
    dvec6 kep;

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

    Properties() : simname("test_sim"),
                   ell_checkbox(true),
                   clicked_ell_ok(false),
                   semiaxes1({0.416194, 0.418765, 0.39309}),
                   semiaxes2({0.104, 0.080, 0.066}),
                   obj_checkbox(false),
                   obj_refer_to_body(1),
                   clicked_poly1_index(-1),
                   clicked_poly2_index(-1),
                   clicked_poly1(false),
                   clicked_poly2(false),
                   path_to_poly_obj(list_files("../obj/")),
                   obj_path1(""),
                   obj_path2(""),
                   vf1({false, false}),
                   vf2({false, false}),
                   grid_reso1({10,10,10}),
                   grid_reso2({10,10,10}),
                   grid_reso_inactive({0,0,0}),
                   clicked_obj_ok(false),
                   ord2_checkbox(true),
                   ord3_checkbox(false),
                   ord4_checkbox(false),
                   M1(5.320591856403073e11),
                   M2(4.940814359692687e9),
                   v_impact({0.0,0.0,0.0}),
                   M_impact(0.0),
                   beta(0.0),
                   epoch(0.0),
                   dur(30.0),
                   step(0.001388888888888889),
                   cart_kep_var{"Cartesian ", "Keplerian "},
                   cart_kep_var_choice(1),
                   cart({0.0,0.0,0.0, 0.0,0.0,0.0}),
                   kep({1.19,0.0,0.0,0.0,0.0,0.0,}),
                   orient_var{"Euler angles", "Quaternions"},
                   orient_var_choice(0),
                   rpy1({0.0,0.0,0.0}),
                   rpy2({0.0,0.0,0.0}),
                   q1({1.0,0.0,0.0,0.0}),
                   q2({1.0,0.0,0.0,0.0}),
                   frame_type{"Inertial frame", "Body frames"},
                   frame_type_choice(0),
                   w1i({0.0,0.0,0.000772269580528465}),
                   w2i({0.0,0.0,0.000146399360157891}),
                   w1b({0.0,0.0,0.0}),
                   w2b({0.0,0.0,0.0})
                   
    { }

    //This function receives as input a path to a directory and as a result it returns a vector of paths, corresponding
    //to all the files (even child directories) found inside 'path/'
    std::vector<std::filesystem::path> list_files(const char *path)
    {
        std::vector<std::filesystem::path> paths; 
        for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
            paths.push_back(entry.path());
        return paths;
    }

    void double_field(const char *label, const float iwidth, int &id, const char *unit, double &variable)
    {      
        ImGui::Text(label);
        ImGui::SameLine();
        ImGui::PushItemWidth(iwidth);
            ImGui::PushID(id++);
                ImGui::InputDouble(unit, &variable, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
        return;
    }

    void int_field(const char *label, const float iwidth, int &id, const char *unit, int &variable)
    {
        ImGui::Text(label);
        ImGui::SameLine();
        ImGui::PushItemWidth(iwidth);
            ImGui::PushID(id++);
                ImGui::InputInt(unit, &variable);
            ImGui::PopID();
        ImGui::PopItemWidth();
        return;
    }

    void render()
    {
        int id = 0;

        ImGui::SetNextWindowPos( ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties ", nullptr);

        ImGui::Text("Simulation name");
        ImGui::PushItemWidth(200.0f);
            ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Shape model");
        if (ImGui::Checkbox("Ellipsoids", &ell_checkbox) && ell_checkbox)
            clicked_ell_ok = false;
            
        if (ell_checkbox && !clicked_ell_ok)
        {
            obj_checkbox = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 300.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters");
            
            //ellipsoids semiaxes submenu
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
            
            //ellipsoids submenu "OK" button
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                clicked_ell_ok = true;

            ImGui::End();
        }

        if (ImGui::Checkbox(".obj file", &obj_checkbox) && obj_checkbox)
            clicked_obj_ok = false;

        if (obj_checkbox && !clicked_obj_ok)
        {
            ell_checkbox = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); //display position of the obj files menu 
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x/7.0f, 300.0f), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj parameters");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            if (ImGui::RadioButton("Body 1", obj_refer_to_body == 1))
                obj_refer_to_body = 1;
            if (ImGui::RadioButton("Body 2", obj_refer_to_body == 2))
                obj_refer_to_body = 2;
            
            if (ImGui::TreeNodeEx("Available .obj models"))
            {
                for (int i = 0; i < path_to_poly_obj.size(); ++i)
                {
                    //which polyhedral .obj model path for body1
                    if (obj_refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_to_poly_obj[i].string().c_str(), (clicked_poly1_index == i)))
                        {
                            clicked_poly1 = true;
                            clicked_poly1_index = i;
                            obj_path1 = path_to_poly_obj[i].string();
                        }
                    }
                    else
                    {
                        //which polyhedral .obj model path for body2
                        if (ImGui::Selectable(path_to_poly_obj[i].string().c_str(), (clicked_poly2_index == i)))
                        {
                            clicked_poly2 = true;
                            clicked_poly2_index = i;
                            obj_path2 = path_to_poly_obj[i].string();
                        }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,5.0f));

            ImGui::Indent();
            ImGui::Text("Raycasting grid resolution ");
            if ((obj_refer_to_body == 1 && !clicked_poly1) ||
                (obj_refer_to_body == 2 && !clicked_poly2) ||
                (!clicked_poly1 && !clicked_poly2))
            {
                ImGui::BeginDisabled();
                int_field("x axis", 100.0f, id, " [ > 1 ]", grid_reso_inactive[0]);
                int_field("y axis", 100.0f, id, " [ > 1 ]", grid_reso_inactive[1]);
                int_field("z axis", 100.0f, id, " [ > 1 ]", grid_reso_inactive[2]);
                ImGui::EndDisabled();
            }
            else if (obj_refer_to_body == 1 && clicked_poly1)
            {
                int_field("x axis", 100.0f, id, " [ > 1 ]", grid_reso1[0]);
                int_field("y axis", 100.0f, id, " [ > 1 ]", grid_reso1[1]);
                int_field("z axis", 100.0f, id, " [ > 1 ]", grid_reso1[2]);
            }
            else if (obj_refer_to_body == 2 && clicked_poly2)
            {
                int_field("x axis", 100.0f, id, " [ > 1 ]", grid_reso2[0]);
                int_field("y axis", 100.0f, id, " [ > 1 ]", grid_reso2[1]);
                int_field("z axis", 100.0f, id, " [ > 1 ]", grid_reso2[2]);
            }
            ImGui::Unindent();
            ImGui::Dummy(ImVec2(0.0f, 15.0f));

            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))   
                clicked_obj_ok = true;

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        //physics theory
        ImGui::Text("Theory expansion");
        if (ImGui::Checkbox("Order 2", &ord2_checkbox))
            ord3_checkbox = ord4_checkbox = false;
        if (ImGui::Checkbox("Order 3", &ord3_checkbox))
            ord2_checkbox = ord4_checkbox = false;
        if (ImGui::Checkbox("Order 4", &ord4_checkbox))
            ord2_checkbox = ord3_checkbox = false;
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Mass");
        double_field("M1 ", 150.0f, id, "[kg]", M1);
        double_field("M2 ", 150.0f, id, "[kg]", M2);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Impactor parameters");
        double_field("υx          ", 100.0f, id, "[km/sec]", v_impact[0]);
        double_field("υy          ", 100.0f, id, "[km/sec]", v_impact[1]);
        double_field("υz          ", 100.0f, id, "[km/sec]", v_impact[2]);
        double_field("Mass    ", 100.0f, id, "[kg]", M_impact);
        double_field("β value ", 100.0f, id, "[  ]", beta);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Integration time");
        double_field("Epoch     ",   100.0f, id, "[days]", epoch);
        double_field("Duration ",   100.0f, id, "[days]", dur);
        double_field("Step        ", 100.0f, id, "[days]", step);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Initial state");
        ImGui::Indent();
        ImGui::Text("Relative position and velocity");

        //position/velocity variables (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(id++);
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
        else
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
        ImGui::Dummy(ImVec2(0.0f, 700.0f)); //extra y-space in order to be able to scroll even when we change the glfw window y size
        ImGui::Unindent();
        

        ImGui::End();
    }

    //If all user inputs are valid, this member function returns an entirely empty vector of strings.
    //Otherwise the returned vector contains string messages, each corresponding to an invalid input. In this case,
    //the vector of strings will be displayed on the console and the simulation will not run.
    strvec validate()
    {
        strvec errors;
        
        //simname[] errors (empty, pure tabs, begin with tab).
        str simnname_copy = simname;
        char first_char = simnname_copy[0];
        char one_space = ' ';
        if ( (simnname_copy.empty()) || (simnname_copy.find_first_not_of(' ') == str::npos) || (first_char == one_space) )
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
        if (obj_checkbox && clicked_poly1_index == -1)
             errors.push_back("[Error] :  No .obj file is selected for 'Body 1'.");
        if (obj_checkbox && clicked_poly2_index == -1)
             errors.push_back("[Error] :  No .obj file is selected for 'Body 2'.");

        //.obj files error of polyhedron category (the .obj file must at least contain lines with the
        //format 'v x y z' AND 'f i j k' to be assumed as a valid polyhedron.
        if (obj_checkbox && clicked_poly1)
        {
            vf1 = Obj::vf_status(obj_path1.c_str());
            if ( !(vf1[0] && vf1[1]) )
                errors.push_back("[Error] :  In 'Body 1' .obj file, vertices and faces lines must exist.");
        }
        if (obj_checkbox && clicked_poly2)
        {
            vf2 = Obj::vf_status(obj_path2.c_str());
            if ( !(vf2[0] && vf2[1]) )
                errors.push_back("[Error] :  In 'Body 2' .obj file, vertices and faces lines must exist.");
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
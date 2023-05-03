#ifndef GUI_HPP
#define GUI_HPP

#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdarg>
#include<filesystem>

#include"typedef.hpp"
#include"constant.hpp"
#include"linalg.hpp"
#include"obj.hpp"
#include"conversion.hpp"

class Console
{

private:

    ImGuiTextBuffer buffer;
    bool scroll_to_bottom;

public:

    //Clear the console.
    void cls()
    {
        buffer.clear();
    }

    //Add formatted text to the console.
    void add_text(const char *format, ...) IM_FMTARGS(2)
    {
        va_list args;
        va_start(args, format);
            buffer.appendfv(format, args);
        va_end(args);
        scroll_to_bottom = true;
    }

    //This function identifies the operating system in which the program is running.
    str os_name()
    {
        #if defined(__APPLE__) || defined(__MACH__)
            return "Mac OSX";
        #elif defined(__linux__) || defined(__unix) || defined(__unix__)
            return "Linux||Unix";
        #elif defined(__FreeBSD__)
            return "FreeBSD";
        #elif defined(_WIN32) || defined(_WIN64)
            return "Windows";
        #else
            return "Other";
        #endif
    }

    //Render the console imgui window.
    void render()
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(win_width/7.0f, win_height - win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width - 2*win_width/7.0f, win_height/7.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Console", NULL);

        //mouse input : clear the console
        if (ImGui::Button("Clear "))
            cls();
        
        ImGui::SameLine();
        ImGui::Text("FPS [ %.1f ], ", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::Text("OS [ %s ]", os_name().c_str());
        ImGui::Separator();

        ImGui::BeginChild("Scroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(buffer.begin());
        if (scroll_to_bottom)
            ImGui::SetScrollHereY(1.0f);
        scroll_to_bottom = false;    
        ImGui::EndChild();

        ImGui::End();
    }
};

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
    int clicked_masc1_index, clicked_poly1_index;
    int clicked_masc2_index, clicked_poly2_index;
    
    //Decide whether body 1 and body 2 will be loaded as polyhedra .obj
    bool clicked_poly1, clicked_poly2;
    
    //Relative paths to mascons/ and polyhedra/ directories.
    std::vector<std::filesystem::path> path_to_masc_obj, path_to_poly_obj;

    //Relative path to the 2 .obj models.
    str obj_path1, obj_path2;

    //fundamental contents of the 2 .obj files (vertices, faces, normals, textures).
    bvec vfnt1, vfnt2;

    //Cartesian grid resolutions (per axis) for filling the polyhedra with mascons.
    ivec3 grid_reso1, grid_reso2, grid_reso_inactive;

    //.obj 'OK' button state.
    bool clicked_obj_ok;

    //'Theory' checkbox state
    bool ord2_checkbox, ord3_checkbox, ord4_checkbox, mascons_checkbox;

    //'M1', 'M2' fields.
    double M1,M2;

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

    //'tolerance' field of the intgration method.
    double tol;

    //'Run' button state.
    bool clicked_run;

    //'Kill' button state.
    bool clicked_kill;

    Properties() : simname("test_sim"),
                   ell_checkbox(false),
                   clicked_ell_ok(false),
                   semiaxes1({0.416194, 0.418765, 0.39309}),
                   semiaxes2({0.104, 0.080, 0.066}),
                   obj_checkbox(false),
                   obj_refer_to_body(1),
                   clicked_masc1_index(-1),
                   clicked_poly1_index(-1),
                   clicked_masc2_index(-1),
                   clicked_poly2_index(-1),
                   clicked_poly1(false),
                   clicked_poly2(false),
                   path_to_masc_obj(list_files("../obj/mascons/")),
                   path_to_poly_obj(list_files("../obj/polyhedra/")),
                   obj_path1(""),
                   obj_path2(""),
                   vfnt1({false, false, false, false}),
                   vfnt2({false, false, false, false}),
                   grid_reso1({10,10,10}),
                   grid_reso2({10,10,10}),
                   grid_reso_inactive({0,0,0}),
                   clicked_obj_ok(false),
                   ord2_checkbox(false),
                   ord3_checkbox(false),
                   ord4_checkbox(false),
                   mascons_checkbox(false),
                   M1(5.320591856403073e11),
                   M2(4.940814359692687e9),
                   epoch(0.0),
                   dur(30.0),
                   step(0.001388888888888889),
                   cart_kep_var{"Cartesian ", "Keplerian "},
                   cart_kep_var_choice(0),
                   cart({1.19,0.0,0.0, 0.0,0.00017421523858789,0.0}),
                   kep({0.0,0.0,0.0,0.0,0.0,0.0,}),
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
                   w2b({0.0,0.0,0.0}),
                   tol(1e-10),
                   clicked_run(false),
                   clicked_kill(false)
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

    void import(const char *path)
    {
        //1) Read a user file that contains the desired 'properties'.
        //2) Update the class member variables, so that the next frame renders the imported ones.
        return;
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

    void render(GLFWwindow *pointer)
    {
        int id = 0;

        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width/7.0f, win_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties ", NULL, ImGuiWindowFlags_MenuBar);

        //classical menu bar section
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Import"))
            {
                if (ImGui::MenuItem("Properties"))
                {
                    //Import ONE file (json or txt) which contains a standard (example-like) set of all the member variables of the 'inputs' class.
                }
                if (ImGui::MenuItem("State vector"))
                {
                    //Import a bunch of files (json or txt) that enhold state vector of one simulation run.
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Theme"))
            {
                if (ImGui::MenuItem("Dark"))
                {
                }
                if (ImGui::MenuItem("Light"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("Simulation name");
        ImGui::PushItemWidth(200.0f);
            ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Shape models");
        if (ImGui::Checkbox("Ellipsoids", &ell_checkbox) && ell_checkbox)
            clicked_ell_ok = false;
            
        if (ell_checkbox && !clicked_ell_ok)
        {
            obj_checkbox = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(300,300), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters");
            
            //ellipsoids semiaxes submenu
            ImGui::Text("Semiaxes");
            double_field("a1 ", 100.0f, id, "[km]", semiaxes1[0]);
            double_field("b1 ", 100.0f, id, "[km]", semiaxes1[1]);
            double_field("c1 ", 100.0f, id, "[km]", semiaxes1[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            double_field("a2 ", 100.0f, id, "[km]", semiaxes2[0]);
            double_field("b2 ", 100.0f, id, "[km]", semiaxes2[1]);
            double_field("c2 ", 100.0f, id, "[km]", semiaxes2[2]);
            ImGui::Dummy(ImVec2(0.0f,15.0f));
            
            //ellipsoids submenu "OK" button
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                clicked_ell_ok = true;

            ImGui::End();
        }

        if (ImGui::Checkbox(".obj files", &obj_checkbox) && obj_checkbox)
            clicked_obj_ok = false;

        if (obj_checkbox && !clicked_obj_ok)
        {
            ell_checkbox = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); //display position of the obj files menu 
            ImGui::SetNextWindowSize(ImVec2(300,400), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj parameters");
            ImGui::Text("KIMIN's .obj database\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            if (ImGui::RadioButton("Body 1", obj_refer_to_body == 1))
                obj_refer_to_body = 1;
            if (ImGui::RadioButton("Body 2", obj_refer_to_body == 2))
                obj_refer_to_body = 2;
            
            if (ImGui::TreeNodeEx("Mascon models"))
            {
                for (int i = 0; i < path_to_masc_obj.size(); ++i)
                {
                    if (obj_refer_to_body == 1)
                    {
                        //which mascons .obj model path for body1
                        if (ImGui::Selectable(path_to_masc_obj[i].string().c_str(), (clicked_masc1_index == i)))
                        {
                            clicked_poly1 = false;
                            clicked_masc1_index = i;
                            clicked_poly1_index = -1;
                            obj_path1 = path_to_masc_obj[i].string();
                        }
                    }
                    else
                    {
                        //which mascons .obj model path for body2
                        if (ImGui::Selectable(path_to_masc_obj[i].string().c_str(), (clicked_masc2_index == i)))
                        {
                            clicked_poly2 = false;
                            clicked_masc2_index = i;
                            clicked_poly2_index = -1;
                            obj_path2 = path_to_masc_obj[i].string();
                        }
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Polyhedral models"))
            {
                for (int i = 0; i < path_to_poly_obj.size(); ++i)
                {
                    //which polyhedral .obj model path for body1
                    if (obj_refer_to_body == 1)
                    {
                        if (ImGui::Selectable(path_to_poly_obj[i].string().c_str(), (clicked_poly1_index == i)))
                        {
                            clicked_poly1 = true;
                            clicked_masc1_index = -1;
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
                            clicked_masc2_index = -1;
                            clicked_poly2_index = i;
                            obj_path2 = path_to_poly_obj[i].string();
                        }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Dummy(ImVec2(0.0f,5.0f));

            ImGui::Indent();
            ImGui::Text("Grid resolution ");
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
        ImGui::Text("Theory");
        if (ImGui::Checkbox("Order 2", &ord2_checkbox))
            ord3_checkbox = ord4_checkbox = mascons_checkbox = false;
        if (ImGui::Checkbox("Order 3", &ord3_checkbox))
            ord2_checkbox = ord4_checkbox = mascons_checkbox = false;
        if (ImGui::Checkbox("Order 4", &ord4_checkbox))
            ord2_checkbox = ord3_checkbox = mascons_checkbox = false;
        if (ImGui::Checkbox("Mascons", &mascons_checkbox))
            ord2_checkbox = ord3_checkbox = ord4_checkbox = false;
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Mass");
        double_field("M1 ", 150.0f, id, "[kg]", M1);
        double_field("M2 ", 150.0f, id, "[kg]", M2);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Integration time");
        double_field("Epoch     ",   100.0f, id, "[days]", epoch);
        double_field("Duration  ",   100.0f, id, "[days]", dur);
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
            double_field("a       " , 100.0f, id, "[km]",  kep[0]);
            double_field("e       " , 100.0f, id, "[  ]",  kep[1]);
            double_field("i        ", 100.0f, id, "[deg]", kep[2]);
            double_field("Ω   "     , 100.0f, id, "[deg]", kep[3]);
            double_field("ω       " , 100.0f, id, "[deg]", kep[4]);
            double_field("M      "  , 100.0f, id, "[deg]", kep[5]);
        }

        ImGui::Text("Orientations");

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
            double_field("yaw 1  "    , 100.0f, id, "[deg]", rpy1[2]);
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
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Unindent();

        //integration tolerance
        ImGui::Text("Integrator");
        double_field("Tolerance " , 150.0f, id, "", tol);
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        //run button
        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            clicked_run = true;

        ImGui::SameLine();

        //kill button
        if (clicked_run)
        {
            if (ImGui::Button("Kill", ImVec2(50.0f,30.0f)))
                clicked_kill = true;
        }
        else
        {
            ImGui::BeginDisabled();
                ImGui::Button("Kill", ImVec2(50.0f,30.0f));
            ImGui::EndDisabled();
        }

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

        //Tolerance error (must be > 0.0).
        if (tol <= 0.0)
            errors.push_back("[Error] :  Invalid 'Tolerance'.");

        return errors;
    }
};

#include"integrator.hpp"

class Graphics
{
    
public:

    //x(t), y(t), z(t), r(t) magnitude (distance), υx(t), υy(t),υz(t), υ(t) magnitude
    bvec plot_cart;

    //a(t), e(t), i(t), Ω(t), ω(t), M(t)
    bvec plot_kep;

    //roll1(t), pitch1(t), yaw1(t), roll2(t), pitch2(t), yaw2(t)
    bvec plot_rpy1, plot_rpy2;
    //q10(t), q11(t), q12(t), q13(t), q20(t), q21(t), q22(t), q23(t)
    bvec plot_q1, plot_q2;

    //ω1ix(t), ω1iy(t), ω1iz(t),  ω1bx(t), ω1by(t), ω1bz(t)
    bvec plot_w1i, plot_w1b;
    //ω2ix(t), ω2ix(t), ω2ix(t),  ω2bx(t), ω2by(t), ω2bz(t)
    bvec plot_w2i, plot_w2b;

    //energy and momentum (magnitude) relative errors
    bvec plot_ener_mom_rel_err;

    //orbit data
    Solution solution;

    Graphics() : plot_cart({false,false,false,false, false,false,false,false}),
                 plot_kep({false,false,false,false,false,false}),
                 plot_rpy1({false,false,false}),
                 plot_rpy2({false,false,false}),
                 plot_q1({false,false,false,false}),
                 plot_q2({false,false,false,false}),
                 plot_w1i({false,false,false}),
                 plot_w1b({false,false,false}),
                 plot_w2i({false,false,false}),
                 plot_w2b({false,false,false}),
                 plot_ener_mom_rel_err({false,false})
    { }

    void yield_solution(const Solution &solution)
    {
        this->solution = solution;
        return;
    }
    
    void plot_buttons()
    {
        ImGui::Text("Relative position");
        if (ImGui::Button("    x    ")) plot_cart[0] = !plot_cart[0]; ImGui::SameLine();
        if (ImGui::Button("    y    ")) plot_cart[1] = !plot_cart[1]; ImGui::SameLine();
        if (ImGui::Button("    z    ")) plot_cart[2] = !plot_cart[2]; ImGui::SameLine();
        if (ImGui::Button("distance"))  plot_cart[3] = !plot_cart[3];
        ImGui::Separator();

        ImGui::Text("Relative velocity");
        if (ImGui::Button("   υx   ")) plot_cart[4] = !plot_cart[4]; ImGui::SameLine();
        if (ImGui::Button("   υy   ")) plot_cart[5] = !plot_cart[5]; ImGui::SameLine();
        if (ImGui::Button("   υz   ")) plot_cart[6] = !plot_cart[6]; ImGui::SameLine();
        if (ImGui::Button("υ mag"))    plot_cart[7] = !plot_cart[7];
        ImGui::Separator();

        ImGui::Text("Relative Keplerian elements");
        if (ImGui::Button("   a   ")) plot_kep[0] = !plot_kep[0]; ImGui::SameLine();
        if (ImGui::Button("   e   ")) plot_kep[1] = !plot_kep[1]; ImGui::SameLine();
        if (ImGui::Button("   i   ")) plot_kep[2] = !plot_kep[2]; ImGui::SameLine();
        if (ImGui::Button("   Ω   ")) plot_kep[3] = !plot_kep[3]; ImGui::SameLine();
        if (ImGui::Button("   ω   ")) plot_kep[4] = !plot_kep[4]; ImGui::SameLine();
        if (ImGui::Button("   M   ")) plot_kep[5] = !plot_kep[5];
        ImGui::Separator();

        ImGui::Text("Euler angles (Body 1)");
        if (ImGui::Button("roll 1"))  plot_rpy1[0] = !plot_rpy1[0]; ImGui::SameLine();
        if (ImGui::Button("pitch 1")) plot_rpy1[1] = !plot_rpy1[1]; ImGui::SameLine();
        if (ImGui::Button("yaw 1"))   plot_rpy1[2] = !plot_rpy1[2];
        ImGui::Separator();

        ImGui::Text("Euler angles (Body 2)");
        if (ImGui::Button("roll 2"))  plot_rpy2[0] = !plot_rpy2[0]; ImGui::SameLine();
        if (ImGui::Button("pitch 2")) plot_rpy2[1] = !plot_rpy2[1]; ImGui::SameLine();
        if (ImGui::Button("yaw 2"))   plot_rpy2[2] = !plot_rpy2[2];
        ImGui::Separator();
        
        ImGui::Text("Quaternion (Body 1)");
        if (ImGui::Button(" q10 ")) plot_q1[0] = !plot_q1[0]; ImGui::SameLine();
        if (ImGui::Button(" q11 ")) plot_q1[1] = !plot_q1[1]; ImGui::SameLine();
        if (ImGui::Button(" q12 ")) plot_q1[2] = !plot_q1[2]; ImGui::SameLine();
        if (ImGui::Button(" q13 ")) plot_q1[3] = !plot_q1[3];
        ImGui::Separator();
        
        ImGui::Text("Quaternion (Body 2)");
        if (ImGui::Button(" q20 ")) plot_q2[0] = !plot_q2[0]; ImGui::SameLine();
        if (ImGui::Button(" q21 ")) plot_q2[1] = !plot_q2[1]; ImGui::SameLine();
        if (ImGui::Button(" q22 ")) plot_q2[2] = !plot_q2[2]; ImGui::SameLine();
        if (ImGui::Button(" q23 ")) plot_q2[3] = !plot_q2[3];
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 1, Inertial)");
        if (ImGui::Button(" ω1ix ")) plot_w1i[0] = !plot_w1i[0]; ImGui::SameLine();
        if (ImGui::Button(" ω1iy ")) plot_w1i[1] = !plot_w1i[1]; ImGui::SameLine();
        if (ImGui::Button(" ω1iz ")) plot_w1i[2] = !plot_w1i[2];
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 1, Body)");
        if (ImGui::Button(" ω1bx ")) plot_w1b[0] = !plot_w1b[0]; ImGui::SameLine();
        if (ImGui::Button(" ω1by ")) plot_w1b[1] = !plot_w1b[1]; ImGui::SameLine();
        if (ImGui::Button(" ω1bz ")) plot_w1b[2] = !plot_w1b[2];
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 2, Inertial)");
        if (ImGui::Button(" ω2ix ")) plot_w2i[0] = !plot_w2i[0]; ImGui::SameLine();
        if (ImGui::Button(" ω2iy ")) plot_w2i[1] = !plot_w2i[1]; ImGui::SameLine();
        if (ImGui::Button(" ω2iz ")) plot_w2i[2] = !plot_w2i[2];
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 2, Body)");
        if (ImGui::Button(" ω2bx ")) plot_w2b[0] = !plot_w2b[0]; ImGui::SameLine();
        if (ImGui::Button(" ω2by ")) plot_w2b[1] = !plot_w2b[1]; ImGui::SameLine();
        if (ImGui::Button(" ω2bz ")) plot_w2b[2] = !plot_w2b[2];
        ImGui::Separator();

        ImGui::Text("Constants of motion");
        if (ImGui::Button("Energy"))       plot_ener_mom_rel_err[0] = !plot_ener_mom_rel_err[0]; ImGui::SameLine();
        if (ImGui::Button("Momentum mag")) plot_ener_mom_rel_err[1] = !plot_ener_mom_rel_err[1];
        ImGui::Separator();

        return;
    }

    bool common_plot(const char *begin_id, const char *begin_plot_id, const char *yaxis_str, const bool bool_plot_func, dvec &plot_func)
    {
        bool temp_bool_plot_func = bool_plot_func;
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x - ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
        ImGui::Begin(begin_id, &temp_bool_plot_func);
        ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        if (ImPlot::BeginPlot(begin_plot_id, plot_win_size))
        {
            ImPlot::SetupAxes("time [days]", yaxis_str);
            ImPlot::PlotLine("", &(solution.t[0]), &plot_func[0], solution.t.size());
            ImPlot::EndPlot();
        }
        ImGui::End();
        return temp_bool_plot_func;
    }

    void render()
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(win_width - win_width/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width/7.0f, win_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Graphics", NULL);

        if (ImGui::CollapsingHeader("Plots"))
        {
            if (!solution.t.size())
            {
                ImGui::BeginDisabled();
                    plot_buttons();
                ImGui::EndDisabled();
            }
            else
            {
                plot_buttons();

                if (plot_cart[0]) plot_cart[0] = common_plot("##1",  "##2",  "x [km]",               plot_cart[0], solution.x);
                if (plot_cart[1]) plot_cart[1] = common_plot("##3",  "##4",  "y [km]",               plot_cart[1], solution.y);
                if (plot_cart[2]) plot_cart[2] = common_plot("##5",  "##6",  "z [km]",               plot_cart[2], solution.z);
                if (plot_cart[3]) plot_cart[3] = common_plot("##7",  "##8",  "distance [km]",        plot_cart[3], solution.dist);
                if (plot_cart[4]) plot_cart[4] = common_plot("##9",  "##10", "υx [km/sec]",          plot_cart[4], solution.vx);
                if (plot_cart[5]) plot_cart[5] = common_plot("##11", "##12", "υy [km/sec]",          plot_cart[5], solution.vy);
                if (plot_cart[6]) plot_cart[6] = common_plot("##13", "##14", "υz [km/sec]",          plot_cart[6], solution.vz);
                if (plot_cart[7]) plot_cart[7] = common_plot("##15", "##16", "υ magnitude [km/sec]", plot_cart[7], solution.vmag);

                if (plot_kep[0]) plot_kep[0] = common_plot("##17", "##18", "a [km]",  plot_kep[0], solution.a);
                if (plot_kep[1]) plot_kep[1] = common_plot("##19", "##20", "e [  ]",  plot_kep[1], solution.e);
                if (plot_kep[2]) plot_kep[2] = common_plot("##21", "##22", "i [deg]", plot_kep[2], solution.inc);
                if (plot_kep[3]) plot_kep[3] = common_plot("##23", "##24", "Ω [deg]", plot_kep[3], solution.Om);
                if (plot_kep[4]) plot_kep[4] = common_plot("##25", "##26", "ω [deg]", plot_kep[4], solution.w);
                if (plot_kep[5]) plot_kep[5] = common_plot("##27", "##28", "M [deg]", plot_kep[5], solution.M);

                if (plot_rpy1[0]) plot_rpy1[0] = common_plot("##29", "##30", "roll 1 [deg]",  plot_rpy1[0], solution.roll1);
                if (plot_rpy1[1]) plot_rpy1[1] = common_plot("##31", "##32", "pitch 1 [deg]",  plot_rpy1[1], solution.pitch1);
                if (plot_rpy1[2]) plot_rpy1[2] = common_plot("##33", "##34", "yaw 1 [deg]", plot_rpy1[2], solution.yaw1);

                if (plot_rpy2[0]) plot_rpy2[0] = common_plot("##35", "##36", "roll 2 [deg]", plot_rpy2[0], solution.roll2);
                if (plot_rpy2[1]) plot_rpy2[1] = common_plot("##37", "##38", "pitch 2 [deg]", plot_rpy2[1], solution.pitch2);
                if (plot_rpy2[2]) plot_rpy2[2] = common_plot("##38", "##40", "yaw 2 [deg]", plot_rpy2[2], solution.yaw2);
            }
        }
        ImGui::End();
    }
};

class GUI
{
public:
    Properties properties;
    Graphics graphics;
    Console console;

    //Constructor and correct initialization of &io.
    GUI(GLFWwindow *pointer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.Fonts->AddFontFromFileTTF("../font/Roboto-Regular.ttf", 15.0f, NULL, io.Fonts->GetGlyphRangesGreek());
        //(void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGuiStyle &imstyle = ImGui::GetStyle();
        imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
        imstyle.FrameRounding = 5.0f;
        imstyle.WindowRounding = 5.0f;
        imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    }

    //Destructor
    ~GUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    //Create an new imgui frame.
    void begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    //Render the gui stuff.
    void render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void on_click_run()
    {
        if (properties.clicked_run)
        {
            strvec errors = properties.validate();
            if (!errors.size())
            {
                Integrator integrator(properties);
                Solution solution = integrator.run();
                graphics.yield_solution(solution);
                //std::thread propagator_thread(std::bind(&Propagator::run, propagator));
                //propagator_thread.detach();
            }
            else
            {
                for (int i = 0; i < errors.size(); ++i)
                {
                    console.add_text(errors[i].c_str());
                    console.add_text("\n");
                }
                console.add_text("\n");
            }
            properties.clicked_run = false;
        }

        return;
    }
};

#endif
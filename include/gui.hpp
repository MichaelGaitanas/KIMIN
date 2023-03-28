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
            return "Other OS";
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

    //Enable the close button on the root imgui window (not the glfw one). Once it is clicked, the game loop terminates.
    bool xclose; 

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
    std::vector<std::filesystem::path> path_to_masc_obj;
    std::vector<std::filesystem::path> path_to_poly_obj;

    //Relative path to the 2 .obj models.
    str obj_path1, obj_path2;

    //fundamental contents of the 2 .obj files (vertices, faces, normals, textures).
    bvec vfnt1, vfnt2;

    //Cartesian grid resolutions (per axis) for filling the polyhedra with mascons.
    ivec3 grid_reso1;
    ivec3 grid_reso2;
    ivec3 grid_reso_inactive;

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
    dvec6 cart;
    //'a', 'e', 'i', 'Om', 'w', 'M' fields.
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

    //'w1x', 'w1y', 'w1z', 'w2x', 'w2y, 'w2z' fields.
    dvec3 w1i, w2i;
    dvec3 w1b, w2b;

    //Integration methods.
    const char *integ_method[4];
    //Initial choice. 0 -> Runge-Kutta-Fehlberg 78, 1 -> Bulirsch-Stoer, etc..
    int integ_method_choice;
    //'tolerance' field.
    double tol;

    //'Run' button state.
    bool clicked_run;

    //'Kill' button state.
    bool clicked_kill;

    Properties() : xclose(true),
                   simname("test_sim"),
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
                   integ_method{"Runge-Kutta-Fehlberg 78 ", "Bulirsch-Stoer", "Dormand-Prince 5 ", "Runge-Kutta 4 (explicit)"},
                   integ_method_choice(0),
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

    void render(GLFWwindow *pointer)
    {
        const float win_width  = ImGui::GetIO().DisplaySize.x;
        const float win_height = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(win_width/7.0f, win_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Properties ",&xclose, ImGuiWindowFlags_MenuBar);
        if (!xclose)
            glfwSetWindowShouldClose(pointer, true);

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
        ImGui::PushItemWidth(200);
            ImGui::InputText(" ", simname, IM_ARRAYSIZE(simname));
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Shape models");
        if (ImGui::Checkbox("Ellipsoids", &ell_checkbox) && ell_checkbox)
        {
            clicked_ell_ok = false;
        }
        if (ell_checkbox && !clicked_ell_ok)
        {
            obj_checkbox = false; //untick the obj checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); 
            ImGui::SetNextWindowSize(ImVec2(300,300), ImGuiCond_FirstUseEver); 
            ImGui::Begin("Ellipsoid parameters");
                
            //ellipsoid semiaxes submenu
            ImGui::Text("Semiaxes");
            //keyboard input : a1 semiaxis
            ImGui::Text("a1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(0);
                    ImGui::InputDouble("[km]", &semiaxes1[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : b1 semiaxis
            ImGui::Text("b1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(1);
                    ImGui::InputDouble("[km]", &semiaxes1[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : c1 semiaxis
            ImGui::Text("c1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(2);
                    ImGui::InputDouble("[km]", &semiaxes1[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            //keyboard input : a2 semiaxis
            ImGui::Text("a2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(3);
                    ImGui::InputDouble("[km]", &semiaxes2[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : b2 semiaxis
            ImGui::Text("b2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(4);
                    ImGui::InputDouble("[km]", &semiaxes2[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            //keyboard input : c2 semiaxis
            ImGui::Text("c2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(5);
                    ImGui::InputDouble("[km]", &semiaxes2[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f,15.0f));

            //mouse input : ellipsoid submenu "OK" button
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))
                clicked_ell_ok = true;

            ImGui::End();
        }

        //mouse input : .obj shape model (either mascons or polyhedra)
        if (ImGui::Checkbox(".obj files", &obj_checkbox) && obj_checkbox)
        {
            clicked_obj_ok = false;
        }
        if (obj_checkbox && !clicked_obj_ok)
        {
            ell_checkbox = false; //untick the ellipsoids checkbox in case it is ticked
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver); //display position of the obj files menu 
            ImGui::SetNextWindowSize(ImVec2(300,400), ImGuiCond_FirstUseEver); 
            ImGui::Begin(".obj parameters");
            ImGui::Text("KIMIN's .obj database\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
            ImGui::Dummy(ImVec2(0.0f,10.0f));

            //mouse input : body1 or body2 via radio buttons
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
                        //mouse input : which .obj model path for body1
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
                        //mouse input : which .obj model path for body2
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
                    //mouse input : which .obj model path for body1
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
                        //mouse input : which .obj model path for body2
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
                //pseudo mouse or keyboard inputs (basically you cannot input anything if the following commands are executed)

                ImGui::BeginDisabled();
                    ImGui::Text("x axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(6);
                            ImGui::InputInt(" [ > 1 ]", &grid_reso_inactive[0]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::Text("y axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(7);
                            ImGui::InputInt(" [ > 1 ]", &grid_reso_inactive[1]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::Text("z axis ");
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                        ImGui::PushID(8);
                            ImGui::InputInt(" [ > 1 ]", &grid_reso_inactive[2]);
                        ImGui::PopID();
                    ImGui::PopItemWidth();
                ImGui::EndDisabled();
            }
            else if (obj_refer_to_body == 1 && clicked_poly1)
            {
                //mouse or keyboard input : body1 x grid resolution for the raycast
                ImGui::Text("x axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(9);
                        ImGui::InputInt(" [ > 1 ]", &grid_reso1[0]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body1 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(10);
                        ImGui::InputInt(" [ > 1 ]", &grid_reso1[1]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body1 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(11);
                        ImGui::InputInt(" [ > 1 ]", &grid_reso1[2]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
            else if (obj_refer_to_body == 2 && clicked_poly2)
            {
                //mouse or keyboard input : body2 x grid resolution for the raycast
                ImGui::Text("x axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(12);
                        ImGui::InputInt(" [ > 1 ]", &grid_reso2[0]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body2 y grid resolution for the raycast
                ImGui::Text("y axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(13);
                        ImGui::InputInt(" [ > 1 ]", &grid_reso2[1]);
                    ImGui::PopID();
                ImGui::PopItemWidth();

                //mouse or keyboard input : body2 z grid resolution for the raycast
                ImGui::Text("z axis ");
                ImGui::SameLine();
                ImGui::PushItemWidth(100.0f);
                    ImGui::PushID(14);
                        ImGui::InputInt(" [ > 1 ]", &grid_reso2[2]);
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
            ImGui::Unindent();

            ImGui::Dummy(ImVec2(0.0f, 15.0f));

            //mouse input : .obj submenu "OK" button
            if (ImGui::Button("OK", ImVec2(50.0f,30.0f)))   
            {
                clicked_obj_ok = true;
            }

            ImGui::End();
        }
        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        //mouse input : physics theory
        ImGui::Text("Theory");
        if (ImGui::Checkbox("Order 2", &ord2_checkbox))
        {
            ord3_checkbox = false;
            ord4_checkbox = false;
            mascons_checkbox = false;
        }
        if (ImGui::Checkbox("Order 3", &ord3_checkbox))
        {
            ord2_checkbox = false;
            ord4_checkbox = false;
            mascons_checkbox = false;
        }
        if (ImGui::Checkbox("Order 4", &ord4_checkbox))
        {
            ord2_checkbox = false;
            ord3_checkbox = false;
            mascons_checkbox = false;
        }
        if (ImGui::Checkbox("Mascons", &mascons_checkbox))
        {
            ord2_checkbox = false;
            ord3_checkbox = false;
            ord4_checkbox = false;
        }
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Mass");

        //keyboard input : M1
        ImGui::Text("M1 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(150.0f);
            ImGui::PushID(15);
                ImGui::InputDouble("[kg]", &M1, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
        
        //keyboard input : M2
        ImGui::Text("M2 ");
        ImGui::SameLine();
        ImGui::PushItemWidth(150.0f);
            ImGui::PushID(16);
                ImGui::InputDouble("[kg]", &M2, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Integration time");

        //keyboard input : Epoch
        ImGui::Text("Epoch     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(17);
                ImGui::InputDouble(" [days]", &epoch, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();

        //keyboard input : Duration
        ImGui::Text("Duration  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(18);
                ImGui::InputDouble("[days]", &dur, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();

        //keyboard input : Step
        ImGui::Text("Step        ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
            ImGui::PushID(19);
                ImGui::InputDouble("[days]", &step, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("Initial state");
        ImGui::Indent();
        ImGui::Text("Relative position and velocity");

        //mouse input : position/velocity variables (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(20);
                ImGui::Combo("  ", &cart_kep_var_choice, cart_kep_var, IM_ARRAYSIZE(cart_kep_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (cart_kep_var_choice == 0)
        {
            //keyboard input : relative position x
            ImGui::Text("x    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(21);
                    ImGui::InputDouble("[km]", &cart[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative position y
            ImGui::Text("y    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(22);
                    ImGui::InputDouble("[km]", &cart[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative position z
            ImGui::Text("z    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(23);
                    ImGui::InputDouble("[km]", &cart[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative velocity vx
            ImGui::Text("υx  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(24);
                    ImGui::InputDouble("[km/sec]", &cart[3], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : relative velocity vy
            ImGui::Text("υy  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(25);
                    ImGui::InputDouble("[km/sec]", &cart[4], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative velocity vz
            ImGui::Text("υz  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(26);
                    ImGui::InputDouble("[km/sec]", &cart[5], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

        }
        else if (cart_kep_var_choice == 1)
        {
            //keyboard input : relative semi-major axis a
            ImGui::Text("a       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(27);
                    ImGui::InputDouble("[km]", &kep[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
                        
            //keyboard input : relative eccentricity e
            ImGui::Text("e       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(28);
                    ImGui::InputDouble("[  ]", &kep[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative inclination i
            ImGui::Text("i        ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(29);
                    ImGui::InputDouble("[deg]", &kep[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative longitude of ascending node Om
            ImGui::Text("Ω   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(30);
                    ImGui::InputDouble("[deg]", &kep[3], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative argument of periapsis w
            ImGui::Text("ω       ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(31);
                    ImGui::InputDouble("[deg]", &kep[4], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : relative mean anomaly M
            ImGui::Text("M      ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(32);
                    ImGui::InputDouble("[deg]", &kep[5], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }

        ImGui::Text("Orientations");

        //mouse input : orientation variables (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(33);
                ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (orient_var_choice == 0)
        {
            //keyboard input : roll 1 angle
            ImGui::Text("roll 1    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(34);
                    ImGui::InputDouble("[deg]", &rpy1[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : pitch 1 angle
            ImGui::Text("pitch 1 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0);
                ImGui::PushID(35);
                    ImGui::InputDouble("[deg]", &rpy1[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : yaw 1 angle
            ImGui::Text("yaw 1  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(36);
                    ImGui::InputDouble("[deg]", &rpy1[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : roll 2 angle
            ImGui::Text("roll 2    ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(37);
                    ImGui::InputDouble("[deg]", &rpy2[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : pitch 2 angle
            ImGui::Text("pitch 2 ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(38);
                    ImGui::InputDouble("[deg]", &rpy2[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : yaw 2 angle
            ImGui::Text("yaw 2  ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(39);
                    ImGui::InputDouble("[deg]", &rpy2[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }
        else if (orient_var_choice == 1)
        {
            //keyboard input : q10 quaternion component
            ImGui::Text("q10   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(40);
                    ImGui::InputDouble("[  ]", &q1[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q11 quaternion component
            ImGui::Text("q11   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(41);
                    ImGui::InputDouble("[  ]", &q1[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q12 quaternion component
            ImGui::Text("q12   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(42);
                    ImGui::InputDouble("[  ]", &q1[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : q13 quaternion component
            ImGui::Text("q13   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(43);
                    ImGui::InputDouble("[  ]", &q1[3], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q20 quaternion component
            ImGui::Text("q20   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(44);
                    ImGui::InputDouble("[  ]", &q2[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q21 quaternion component
            ImGui::Text("q21   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(45);
                    ImGui::InputDouble("[  ]", &q2[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : q22 quaternion component
            ImGui::Text("q22   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(46);
                    ImGui::InputDouble("[  ]", &q2[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //field : q23 quaternion component
            ImGui::Text("q23   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(47);
                    ImGui::InputDouble("[  ]", &q2[3], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }

        ImGui::Text("Angular velocity");

        //mouse input : frame choice (combo)
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(48);
                ImGui::Combo("  ", &frame_type_choice, frame_type, IM_ARRAYSIZE(frame_type));
            ImGui::PopID();
        ImGui::PopItemWidth();

        if (frame_type_choice == 0)
        {
            //keyboard input : w1x (inertial) angular velocity
            ImGui::Text("ω1x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(49);
                    ImGui::InputDouble("[rad/sec]", &w1i[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w1y (inertial) angular velocity
            ImGui::Text("ω1y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(50);
                    ImGui::InputDouble("[rad/sec]", &w1i[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : w1z (inertial) angular velocity
            ImGui::Text("ω1z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(51);
                    ImGui::InputDouble("[rad/sec]", &w1i[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w2x (inertial) angular velocity
            ImGui::Text("ω2x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(52);
                    ImGui::InputDouble("[rad/sec]", &w2i[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w2y (inertial) angular velocity
            ImGui::Text("ω2y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(53);
                    ImGui::InputDouble("[rad/sec]", &w2i[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
            
            //keyboard input : w2z (inertial) angular velocity
            ImGui::Text("ω2z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(54);
                    ImGui::InputDouble("[rad/sec]", &w2i[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

        }
        else if (frame_type_choice == 1)
        {
            //keyboard input : w11 (body) angular velocity
            ImGui::Text("ω1x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(55);
                    ImGui::InputDouble("[rad/sec]", &w1b[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w12 (body) angular velocity
            ImGui::Text("ω1y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(56);
                    ImGui::InputDouble("[rad/sec]", &w1b[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w13 (body) angular velocity
            ImGui::Text("ω1z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(57);
                    ImGui::InputDouble("[rad/sec]", &w1b[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w21 (body) angular velocity
            ImGui::Text("ω2x   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(58);
                    ImGui::InputDouble("[rad/sec]", &w2b[0], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w22 (body) angular velocity
            ImGui::Text("ω2y   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(59);
                    ImGui::InputDouble("[rad/sec]", &w2b[1], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();

            //keyboard input : w23 (body) angular velocity
            ImGui::Text("ω2z   ");
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
                ImGui::PushID(60);
                    ImGui::InputDouble("[rad/sec]", &w2b[2], 0.0, 0.0,"%g");
                ImGui::PopID();
            ImGui::PopItemWidth();
        }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Unindent();

        ImGui::Text("Integrator");

        //mouse input : integration method
        ImGui::PushItemWidth(200.0f);
            ImGui::PushID(61);
                ImGui::Combo("  ", &integ_method_choice, integ_method, IM_ARRAYSIZE(integ_method));
            ImGui::PopID();
        ImGui::PopItemWidth();

        //keyboard input : integration method tolerance
        ImGui::Text("Tolerance ");
        ImGui::SameLine();
        ImGui::PushItemWidth(150.0);
            ImGui::PushID(62);
                ImGui::InputDouble("", &tol, 0.0, 0.0,"%g");
            ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f,15.0f));

        //mouse input : run button
        if (ImGui::Button("Run", ImVec2(50.0f,30.0f)))
            clicked_run = true;

        ImGui::SameLine();

        //mouse input : kill button
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
            errors.push_back("[Error] :  Invalid tolerance 'tol'.");

        return errors;
    }
};

#include"integrator.hpp"

class Graphics
{
    
public:

    Solution solution;

    bool plot_x, plot_y, plot_z, plot_dist;
    bool plot_roll1, plot_pitch1, plot_yaw1;
    bool plot_roll2, plot_pitch2, plot_yaw2;
    bool plot_q10, plot_q11, plot_q12, plot_q13;
    bool plot_q20, plot_q21, plot_q22, plot_q23;

    bool plot_vx, plot_vy, plot_vz, plot_vdist;
    bool plot_w1ix, plot_w1iy, plot_w1iz;
    bool plot_w1bx, plot_w1by, plot_w1bz;
    bool plot_w2ix, plot_w2iy, plot_w2iz;
    bool plot_w2bx, plot_w2by, plot_w2bz;

    bool plot_a, plot_e, plot_i, plot_Om, plot_w, plot_M;

    bool plot_ener_rel_err, plot_mom_rel_err;

    Graphics() : plot_x(false),
                 plot_y(false),
                 plot_z(false),
                 plot_dist(false),

                 plot_roll1(false),
                 plot_pitch1(false),
                 plot_yaw1(false),

                 plot_roll2(false),
                 plot_pitch2(false),
                 plot_yaw2(false),

                 plot_q10(false),
                 plot_q11(false),
                 plot_q12(false),
                 plot_q13(false),

                 plot_q20(false),
                 plot_q21(false),
                 plot_q22(false),
                 plot_q23(false),

                 plot_vx(false),
                 plot_vy(false),
                 plot_vz(false),
                 plot_vdist(false),
                 
                 plot_w1ix(false),
                 plot_w1iy(false),
                 plot_w1iz(false),
                 
                 plot_w1bx(false),
                 plot_w1by(false),
                 plot_w1bz(false),

                 plot_w2ix(false),
                 plot_w2iy(false),
                 plot_w2iz(false),
                 
                 plot_w2bx(false),
                 plot_w2by(false),
                 plot_w2bz(false),

                 plot_a(false),
                 plot_e(false),
                 plot_i(false),
                 plot_Om(false),
                 plot_w(false),
                 plot_M(false),

                 plot_ener_rel_err(false),
                 plot_mom_rel_err(false)
    { }

    void yield_solution(const Solution &solution)
    {
        this->solution = solution;
        return;
    }
    
    void plot_buttons()
    {
        ImGui::Text("Relative position");
        if (ImGui::Button("    x    ")) plot_x = !plot_x; ImGui::SameLine();
        if (ImGui::Button("    y    ")) plot_y = !plot_y; ImGui::SameLine();
        if (ImGui::Button("    z    ")) plot_z = !plot_z; ImGui::SameLine();
        if (ImGui::Button("distance")) plot_dist = !plot_dist;
        ImGui::Separator();

        ImGui::Text("Relative velocity");
        if (ImGui::Button("   υx   ")) plot_vx = !plot_vx; ImGui::SameLine();
        if (ImGui::Button("   υy   ")) plot_vy = !plot_vy; ImGui::SameLine();
        if (ImGui::Button("   υz   ")) plot_vz = !plot_vz;
        ImGui::Separator();

        ImGui::Text("Relative Keplerian elements");
        if (ImGui::Button("   a   ")) plot_a =  !plot_a;  ImGui::SameLine();
        if (ImGui::Button("   e   ")) plot_e =  !plot_e;  ImGui::SameLine();
        if (ImGui::Button("   i   ")) plot_i =  !plot_i;  ImGui::SameLine();
        if (ImGui::Button("   Ω   ")) plot_Om = !plot_Om; ImGui::SameLine();
        if (ImGui::Button("   ω   ")) plot_w =  !plot_w;  ImGui::SameLine();
        if (ImGui::Button("   M   ")) plot_M =  !plot_M;
        ImGui::Separator();

        ImGui::Text("Euler angles (Body 1)");
        if (ImGui::Button("roll 1"))  plot_roll1 =  !plot_roll1;  ImGui::SameLine();
        if (ImGui::Button("pitch 1")) plot_pitch1 = !plot_pitch1; ImGui::SameLine();
        if (ImGui::Button("yaw 1"))   plot_yaw1 =   !plot_yaw1;
        ImGui::Separator();

        ImGui::Text("Euler angles (Body 2)");
        if (ImGui::Button("roll 2"))  plot_roll2 =  !plot_roll2;  ImGui::SameLine();
        if (ImGui::Button("pitch 2")) plot_pitch2 = !plot_pitch2; ImGui::SameLine();
        if (ImGui::Button("yaw 2"))   plot_yaw2 =   !plot_yaw2;
        ImGui::Separator();
        
        ImGui::Text("Quaternion (Body 1)");
        if (ImGui::Button(" q10 ")) plot_q10 = !plot_q10; ImGui::SameLine();
        if (ImGui::Button(" q11 ")) plot_q11 = !plot_q11; ImGui::SameLine();
        if (ImGui::Button(" q12 ")) plot_q12 = !plot_q12; ImGui::SameLine();
        if (ImGui::Button(" q13 ")) plot_q13 = !plot_q13;
        ImGui::Separator();
        
        ImGui::Text("Quaternion (Body 2)");
        if (ImGui::Button(" q20 ")) plot_q20 = !plot_q20; ImGui::SameLine();
        if (ImGui::Button(" q21 ")) plot_q21 = !plot_q21; ImGui::SameLine();
        if (ImGui::Button(" q22 ")) plot_q22 = !plot_q22; ImGui::SameLine();
        if (ImGui::Button(" q23 ")) plot_q23 = !plot_q23;
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 1, inertial frame)");
        if (ImGui::Button(" ω1ix ")) plot_w1ix = !plot_w1ix; ImGui::SameLine();
        if (ImGui::Button(" ω1iy ")) plot_w1iy = !plot_w1iy; ImGui::SameLine();
        if (ImGui::Button(" ω1iz ")) plot_w1iz = !plot_w1iz;
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 2, inertial frame)");
        if (ImGui::Button(" ω2ix ")) plot_w2ix = !plot_w2ix; ImGui::SameLine();
        if (ImGui::Button(" ω2iy ")) plot_w2iy = !plot_w2iy; ImGui::SameLine();
        if (ImGui::Button(" ω2iz ")) plot_w2iz = !plot_w2iz;
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 1, body frame)");
        if (ImGui::Button(" ω1bx ")) plot_w1bx = !plot_w1bx; ImGui::SameLine();
        if (ImGui::Button(" ω1by ")) plot_w1by = !plot_w1by; ImGui::SameLine();
        if (ImGui::Button(" ω1bz ")) plot_w1bz = !plot_w1bz;
        ImGui::Separator();

        ImGui::Text("Angular velocity (Body 2, body frame)");
        if (ImGui::Button(" ω2bx ")) plot_w2bx = !plot_w2bx; ImGui::SameLine();
        if (ImGui::Button(" ω2by ")) plot_w2by = !plot_w2by; ImGui::SameLine();
        if (ImGui::Button(" ω2bz ")) plot_w2bz = !plot_w2bz;
        ImGui::Separator();

        ImGui::Text("Constants of motion");
        if (ImGui::Button("Energy"))   plot_ener_rel_err = !plot_ener_rel_err; ImGui::SameLine();
        if (ImGui::Button("Momentum")) plot_mom_rel_err =  !plot_mom_rel_err;
        ImGui::Separator();

        return;
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
                    
                if (plot_x)
                {
                    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x - ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
                    ImGui::Begin("Position x", &plot_x);
                    ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
                    if (ImPlot::BeginPlot("x(t)", plot_win_size))
                    {
                        ImPlot::SetupAxes("time [days]","x [km]");
                        ImPlot::PlotLine("", &(solution.t[0]), &(solution.x[0]), solution.t.size());
                        ImPlot::EndPlot();  
                    }
                    ImGui::End();
                }
                if (plot_ener_rel_err)
                {
                    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x - ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
                    ImGui::Begin("Energy relative error", &plot_ener_rel_err);
                    ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
                    if (ImPlot::BeginPlot("ener_rel_err(t)", plot_win_size))
                    {
                        ImPlot::SetupAxes("time [days]","error [ ]");
                        ImPlot::PlotLine("", &(solution.t[0]), &(solution.ener_rel_err[0]), solution.t.size());
                        ImPlot::EndPlot();  
                    }
                    ImGui::End();
                }
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

    //while (!glfwWindowShouldClose(window))
    //{

        //...

        /*

        //ellipsoid mass
        ImGui::Text("Mass");
        static double mass = 0.0f;
        ImGui::PushID(3);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("M ");
        ImGui::SameLine();
        ImGui::InputDouble("", &mass, 0.0f, 0.0f,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[kg]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        //integration time parameters
        ImGui::Text("Integration time parameters");
        static double t0 = 0.0f;
        ImGui::PushID(4);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("t0             ");
        ImGui::SameLine();
        ImGui::InputDouble("", &t0, 0.0f, 0.0f,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();

        static double tmax = 0.0f;
        ImGui::PushID(5);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("tmax        ");
        ImGui::SameLine();
        ImGui::InputDouble("", &tmax, 0.0f, 0.0f,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        
        static double print_step = 0.0f;
        ImGui::PushID(6);
        ImGui::PushItemWidth(100.0f);
        ImGui::Text("print step ");
        ImGui::SameLine();
        ImGui::InputDouble("", &print_step, 0.0f, 0.0f,"%.5lf");
        ImGui::SameLine();
        ImGui::Text("[days]");
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        //initial orientation
        ImGui::Text("Initial orientation");
        static const char *orient_var[] = {"Euler angles ", "Quaternion "};
        static int orient_var_choice = 0;
        ImGui::PushID(7);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("  ", &orient_var_choice, orient_var, IM_ARRAYSIZE(orient_var));
        ImGui::PopID();
        ImGui::PopItemWidth();
        if (orient_var_choice == 0)
        {
            static double roll = 0.0f;
            ImGui::PushID(8);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("roll    ");
            ImGui::SameLine();
            ImGui::InputDouble("", &roll, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[deg]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double pitch = 0.0f;
            ImGui::PushID(9);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("pitch ");
            ImGui::SameLine();
            ImGui::InputDouble("", &pitch, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[deg]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            
            static double yaw = 0.0f;
            ImGui::PushID(10);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("yaw  ");
            ImGui::SameLine();
            ImGui::InputDouble("", &yaw, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[deg]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }
        else if (orient_var_choice == 1)
        {
            static double q0 = 1.0f;
            ImGui::PushID(11);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q0 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q0, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double q1 = 0.0f;
            ImGui::PushID(12);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q1 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q1, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double q2 = 0.0f;
            ImGui::PushID(13);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q2 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q2, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double q3 = 0.0f;
            ImGui::PushID(14);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("q3 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &q3, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[  ]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }

        ImGui::Text("Initial angular velocity");
        static const char *frame_type[] = {"Inertial frame", "Body frame"};
        static int frame_type_choice = 0;
        ImGui::PushID(15);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("  ", &frame_type_choice, frame_type, IM_ARRAYSIZE(frame_type));
        ImGui::PopID();
        ImGui::PopItemWidth();
        if (frame_type_choice == 0)
        {
            static double wx = 0.0f;
            ImGui::PushID(16);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("wx ");
            ImGui::SameLine();
            ImGui::InputDouble("", &wx, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double wy = 0.0f;
            ImGui::PushID(17);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("wy ");
            ImGui::SameLine();
            ImGui::InputDouble("", &wy, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            
            static double wz = 0.0f;
            ImGui::PushID(18);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("wz ");
            ImGui::SameLine();
            ImGui::InputDouble("", &wz, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }
        else if (frame_type_choice == 1)
        {
            static double w1 = 0.0f;
            ImGui::PushID(19);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("w1 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &w1, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double w2 = 0.0f;
            ImGui::PushID(20);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("w2 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &w2, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();

            static double w3 = 0.0f;
            ImGui::PushID(21);
            ImGui::PushItemWidth(100.0f);
            ImGui::Text("w3 ");
            ImGui::SameLine();
            ImGui::InputDouble("", &w3, 0.0f, 0.0f,"%.5lf");
            ImGui::SameLine();
            ImGui::Text("[rad/sec]");
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
        }

        ImGui::Text("Files to print");
        static bool print_time = true;
        ImGui::Checkbox("Time", &print_time);
        static bool print_quat = true;
        ImGui::Checkbox("Quaternion", &print_quat);
        static bool print_ang = true;
        ImGui::Checkbox("Roll, Pitch, Yaw", &print_ang);
        static bool print_wb = true;
        ImGui::Checkbox("Body angular velocity", &print_wb);
        static bool print_wi = true;
        ImGui::Checkbox("Inertial angular velocity", &print_wi);
        static bool print_ener_mom = true;
        ImGui::Checkbox("Energy & momentum", &print_ener_mom);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        if (ImGui::Button("Run", ImVec2(60,30)))
        {
            //run_physics();
        }
        */

        /*
        ImGui::NextColumn();

        if (ImGui::CollapsingHeader("Interactive graphics"))
        {
            static bool render_ellipsoid = true;
            ImGui::Checkbox("Render ellipsoid", &render_ellipsoid);

            static bool render_axes = true;
            ImGui::Checkbox("Render axes", &render_axes);
        }
        */
 
       
    //} end while()



























    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}
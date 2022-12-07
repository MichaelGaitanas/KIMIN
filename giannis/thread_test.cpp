#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>

#include<thread>

//Μια υποτιθέμενη "βαριά" συνάρτηση η οποία παίρνει χρόνο να τρέξει και εξαιτίας της κολλάει το gui.
void run_physics()
{
    for (double z = 0.0; z < 20000.0; z += 0.001)
    {
        double y = exp(sin(sqrt(z*fabs(z)+ cos(z))));
    }
    return;
}

int main()
{
    //Αρχικοποίησε την glfw.
	glfwInit();

    //Πες της ότι θα χρησιμοποιήσει την έκδοση 3.3 της OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Φτιάξε ένα παράθυρο για γραφικά 800x600 (σε pixels).
    GLFWwindow *window = glfwCreateWindow(800,600, "Proper threading", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }

    //Οποιαδήποτε εντολή που έχει glfwBlaBla από δω και πέρα αφορά μόνο το παράθυρο 'window' που έφτιαξες.
    glfwMakeContextCurrent(window);

    //Ξέχνα το αυτό.
    //////////////////////////////////////////////////////////
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    //////////////////////////////////////////////////////////

    //Αρχικοποίησε το imgui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //Background χρώμα του 'window' κάθε φορά που ανανεώνεται ένα frame.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                 //r    g     b     a

    //Game loop
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT); //Καθάρισε το frame με το χρώμα που όρισες στην glClearColor()

        //Φτιάξε ένα imgui παράθυρο 'πάνω από' το (μαύρο) window της OpenGL.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Controls");
        ImGui::Button("Test button"); //φτιάξε ένα κουμπί 'Test button'
        if (ImGui::Button("Physics button")) //φτιάξε ένα κουμπί 'Physics button' ΚΑΙ αν πατηθεί τρέξε αυτό που λέει το if { }
        {
            //Εδώ είναι το πρόβλημα. Θέλουμε η run_physics() να τρέξει σε ξεχωριστό thread,
            //έτσι ώστε η while() που ανανεώνει τα frames να μην "κολλήσει" εδώ.
            run_physics();
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //Τέλος του imgui frame

        //Ξέχνα το αυτό
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //free imgui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //free glfw resources
    glfwTerminate();

	return 0;
}
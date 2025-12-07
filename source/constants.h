#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr double PI = 3.1415926535897932384626433832795;
constexpr double G = 6.67430e-20; //[km^3/(kg*sec^2)]
constexpr double AU2KM = 149597870.7; //[km]
constexpr double PSRP = 4.56e-6; //[N/m^2]
constexpr size_t N_SOL2D = 5000;
constexpr float CAM_MIN_FOV = 1.0f; //[deg]
constexpr float CAM_MAX_FOV = 179.0f; //[deg]
constexpr float CAM_INIT_FOV = 60.0f; //[deg]
constexpr int DEPTH_RESO_MIN = 1024; //[pix]
constexpr int DEPTH_RESO_MAX = 8192; //[pix]
constexpr int DEPTH_RESO_INIT = 4096; //[pix]
constexpr double MIN_SUN_DIST_SRP = 0.005; //[AU]
constexpr int NR_MAX_ITERATIONS = 20;
constexpr size_t N_ODES = 26;
constexpr double ODES_INIT_GUESS_TIME_STEP = 1.0; //[sec]
constexpr int GLFW_MIN_WIDTH = 400; //[pix]
constexpr int GLFW_MIN_HEIGHT = 400; //[pix]
constexpr int GLFW_SSAS_SAMPLES = 4;
constexpr int GLFW_REQUESTED_DEPTH_BITS = 32;
constexpr float CLEAR_COLOR_R = 0.06f;
constexpr float CLEAR_COLOR_G = 0.06f;
constexpr float CLEAR_COLOR_B = 0.06f;

constexpr char SIM_ROOT_DIR[] = "../simulations/";

//These are scale factors and are defined as variables because their values can only be computed as soon as the app launches. They remain however unchanged throughout the app run.
float SCX, SCY;

#endif
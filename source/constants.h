#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr double PI = 3.1415926535897932384626433832795;
constexpr double G = 6.67430e-20; //[km^3/(kg*sec^2)]
constexpr double AU2KM = 149597870.7; //[km]
constexpr double MSUN = 1.989e30; //[kg]
constexpr double PSRP = 4.56e-6; //[N/m^2]
constexpr double MIN_SUN_BODY_DIST = 0.005; //[AU]
constexpr double ODES_INIT_GUESS_TIME_STEP = 1.0; //[sec]

constexpr size_t N_SOL2D = 5000;
constexpr size_t N_ODES = 26;

constexpr float CAM_MIN_FOV = 1.0f; //[deg]
constexpr float CAM_MAX_FOV = 179.0f; //[deg]
constexpr float CAM_INIT_FOV = 60.0f; //[deg]
constexpr float CLEAR_COLOR_R = 0.06f;
constexpr float CLEAR_COLOR_G = 0.06f;
constexpr float CLEAR_COLOR_B = 0.06f;
constexpr float OBJ_AXES_LENGTH = 2.98179f; //[km]
//These are defined as variables because their values are hardware dependent, hence computed
//as soon as the KIMIN launches (in window.h). They remain constant however throughout the runtime.
float SCX, SCY;

constexpr int NR_MAX_ITERATIONS = 20;
constexpr int DEPTH_RESO_MIN = 1024; //[pix]
constexpr int DEPTH_RESO_MAX = 8192; //[pix]
constexpr int DEPTH_RESO_INIT = 4096; //[pix]
constexpr int GLFW_MIN_WIDTH = 400; //[pix]
constexpr int GLFW_MIN_HEIGHT = 400; //[pix]
constexpr int GLFW_SSAS_SAMPLES = 4;
constexpr int GLFW_REQUESTED_DEPTH_BITS = 32;

constexpr char SIM_ROOT_DIR[] = "../simulations/";
constexpr char PATH_TO_FONTS[] = "../fonts/RobotoRegular.ttf";
constexpr char PATH_TO_ICONS[] = "../fonts/Icons.otf";
constexpr char PATH_TO_LOGO_IMAGE[] = "../logo/logo.jpg";

#endif
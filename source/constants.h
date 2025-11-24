#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr double PI = 3.1415926535897932384626433832795;
constexpr double G = 6.67430e-20; //[km^3/(kg*sec^2)]
constexpr size_t SOL2D_SIZE = 5000;
constexpr float CAM_MIN_FOV = 1.0f; //[deg]
constexpr float CAM_MAX_FOV = 179.0f; //[deg]
constexpr int NR_MAX_ITERATIONS = 20;
constexpr double ODES_INIT_GUESS_TIME_STEP = 1.0; //[sec]
constexpr int GLFW_MIN_WIDTH = 400; //[pix]
constexpr int GLFW_MIN_HEIGHT = 400; //[pix]
constexpr int GLFW_SSAS_SAMPLES = 4;
constexpr int GLFW_REQUESTED_DEPTH_BITS = 32;
constexpr float CLEAR_COLOR_R = 0.06f;
constexpr float CLEAR_COLOR_G = 0.06f;
constexpr float CLEAR_COLOR_B = 0.06f;

#endif
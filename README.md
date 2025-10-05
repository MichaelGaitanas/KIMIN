# KIMIN

![image_alt](https://github.com/MichalisGaitanas/KIMIN/blob/main/logo/logo.jpg)

(K)inetic (I)mpact (MI)ssion to (N)EO is a binary asteroid simulation app. The code is written in C++ and uses OpenGL as a graphics API renderer. It is designed to model the Full 2 Rigid Body Problem (F2RBP), primarily applied to binary asteroid systems. It evaluates the gravitational interactions of arbitrarily shaped rigid bodies, and solves their differential equations of motion (translational and rotational kinematics-dynamics), providing detailed insight of their behavior, over a time range.

Build for Windows (for now). To compile and run, the following must be installed :
1) C++ compiler, e.g. GNU's gcc/g++.
2) OpenGL kernel (usually preinstalled in all common systems).
3) GLFW.
4) GLEW.
5) GLM.
6) ImGui.
7) ImPlot.
8) Boost.
9) Eigen.

Installation of MSYS2 provides a quick way to setup the aforementioned.

In case you do use GNU's gcc/g++ compiler, here's how you can compile and run the app :

-------------------------------------------------------------------------------------------------------------------------------------------------

Windows : 

'Add to path' the compiler, so you can execute commands in the powershell. Then, in powershell navigate to the KIMIN\source\ folder and type :

g++ main.cpp -o main.exe (PATH_TO_KIMIN)\imgui\*.cpp -I (PATH_TO_KIMIN)\imgui -lopengl32 -lglew32 -lglfw3 -O3 -Wall -Wpedantic -Wextra -Werror

It is a good idea to (pre)compile ImGui as a static library, so that you don't have to compile every time the same code.

To compile ImGui as a static library : Navigate to the imgui folder and then, in the powershell type :
a) g++ -c *.cpp
b) ar rcs libimgui.a *o
Library is created.

Then : g++ main.cpp -o main.exe -L (PATH_TO_KIMIN)\imgui -limgui -lopengl32 -lglew32 -lglfw3 -O3 -Wall -Wpedantic -Wextra -Werror

-------------------------------------------------------------------------------------------------------------------------------------------------

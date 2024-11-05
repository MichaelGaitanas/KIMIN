# KIMIN

(K)inetic (I)mpact (MI)ssion to (N)EO is a physics simulator developed in C++ with OpenGL graphics rendering. Designed to model the Full 2 Body Problem (F2BP) applied to binary asteroid systems. It accurately evaluates the gravitational interactions of arbitrarily shaped rigid bodies, such as asteroids and solves their differential equations governing both translational and rotational dynamics, providing detailed insight of their behavior, over a time range.

In order to compile and run, the following must be installed :
1) C++ compiler. We use GNU's gcc/g++.
2) OpenGL kernel (usually preinstalled).
3) GLFW.
4) GLEW.
5) GLM.
6) ImGui.
7) ImPlot.
8) Boost.

MSYS2 provides a quick way to setup the aforementioned.

In case you do use GNU's gcc/g++ compiler, here's how you can compile and run the app :

-------------------------------------------------------------------------------------------------------------------------------------------------

Windows : 

'Add to path' the compiler, so you can execute commands in the cmd. Then, in cmd navigate to the KIMIN/kimin/ folder type :

g++ main.cpp -o main.exe (PATH_TO_KIMIN)\imgui\*.cpp -I (PATH_TO_KIMIN)\imgui -lopengl32 -lglfw3 -lglew32

It is a good idea to (pre)compile ImGui as a static library, so that you don't have to compile every time the same code.

Compile as a static library : Navigate to the imgui folder and then, in the cmd write :
a) g++ -c *.cpp
b) ar rcs libimgui.a *o
Library is created.

Then : g++ main.cpp -o main.exe -L (PATH_TO_KIMIN)\imgui -limgui -lopengl32 -lglfw3 -lglew32

-------------------------------------------------------------------------------------------------------------------------------------------------
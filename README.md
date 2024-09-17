# KIMIN
/------------------------------------------------------/
Windows (using MSVC):

Install the x64 Windows Build Tools and add the following libraries to the path include:

boost
glfw3
glew32
glm

Then navigate to kimin/ folder and compile the main program as follows:

cl /std:c++17 /EHsc main.cpp (PATH_TO_KIMIN)\imgui\imgui*.cpp (PATH_TO_KIMIN)\imgui\implot*.cpp /link glew32.lib glfw3.lib opengl32.lib
/------------------------------------------------------/
Linux:

To compile KIMIN you need the following libraries to be installed and added to the include path:

boost
glfw3
glew32
glm

Then navigate to kimin/ folder and compile the main program as follows:

g++ main.cpp -o main.exe (PATH_TO_KIMIN)\imgui\imgui*.cpp (PATH_TO_KIMIN)\imgui\implot*.cpp -I  (PATH_TO_KIMIN)\imgui -lopengl32 -lglfw3 -lglew32
/------------------------------------------------------/
Windows (using MSYS2):

Install MSYS2 and add g++ and the following libraries:

boost
glfw3
glew32
glm

Then navigate to kimin/ folder and compile the main program as follows:

g++ main.cpp -o main.exe (PATH_TO_KIMIN)\imgui\imgui*.cpp (PATH_TO_KIMIN)\imgui\implot*.cpp -I  (PATH_TO_KIMIN)\imgui -lopengl32 -lglfw3 -lglew32


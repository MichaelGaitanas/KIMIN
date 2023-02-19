# KIMIN

Windows, cmd (assuming the installations : g++ (add to path), glfw, glew)
g++ main.cpp -o main.exe C:\Users\micha\github\KIMIN\imgui\imgui*.cpp -I C:\Users\micha\github\KIMIN\imgui -lopengl32 -lglfw3 -lglew32 -ljsoncpp

Linux, terminal (assuming the installations : glfw, glew)
g++ main.cpp -o main C:\Users\micha\github\KIMIN\imgui\imgui*.cpp -I C:\Users\micha\github\KIMIN\imgui -lGL -lglfw -lGLEW -ljsoncpp
import os
import sys

dir_path = os.path.dirname(os.path.realpath(__file__))
include_path = os.path.join(dir_path,"BuildFiles")
imgui = os.path.join(dir_path,"BuildFiles\imgui\imgui*.cpp")
implot = os.path.join(dir_path,"BuildFiles\imgui\implot*.cpp")
print(implot)
print(imgui)
os.system("cl /I "+include_path+" /std:c++17 /EHsc main.cpp "+imgui+" "+implot + " /link /LIBPATH:BuildFiles glew32.lib glfw3.lib opengl32.lib")
#os.system("del *.obj")
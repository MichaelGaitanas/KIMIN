# KIMIN

g++ main.cpp -o main.exe C:\Users\micha\github\KIMIN\imgui\imgui*.cpp C:\Users\micha\github\KIMIN\imgui\implot*.cpp -I C:\Users\micha\github\KIMIN\imgui -lopengl32 -lglfw3 -lglew32

ToDoList :
1) Run detached // Kill // Status-Run
2) Skybox
3) Check why error fails

() Να τεσταριστεί ότι δουλεύει το πεδίο ".obj files" στο GUI executable και (κατά συνέπεια) η συνάρτηση correct_masc_inertia( ) στο mascon.hpp
() Δηλαδή να φορτωθούν 5-6 σχήματα από .obj και να γίνει propagation.
() Να τεσταρίστουν οι εξισώσεις του β. 
() Ο intgrator ΔΕΝ υπακούει στο adaptive step. Θεωρεί ότι το dt είναι το εσωτερικό step.



==================================================
ΜΕΤΑΤΡΟΠΕΣ ΜΟΝΑΔΩΝ >>> ΟΛΑ CONSISTENT ΝΑ ΕΙΝΑΙ.
# KIMIN

g++ main.cpp -o main.exe C:\Users\Michalis\github\KIMIN\imgui\imgui*.cpp C:\Users\Michalis\github\KIMIN\imgui\implot*.cpp -I C:\Users\Michalis\github\KIMIN\imgui -lopengl32 -lglfw3 -lglew32

ToDoList :
1) Run detached // Kill // Status-Run
2) Skybox
3) Check why error fails

() Να τεσταριστεί ότι δουλεύει το πεδίο ".obj files" στο GUI executable και (κατά συνέπεια) η συνάρτηση correct_masc_inertia( ) στο mascon.hpp
() Δηλαδή να φορτωθούν 5-6 σχήματα από .obj και να γίνει propagation.
() Να τεσταρίστουν οι εξισώσεις του β. 
() Ο intgrator ΔΕΝ υπακούει στο adaptive step. Θεωρεί ότι το dt είναι το εσωτερικό step.
() Το θέμα με τα threads. Πρέπει να επικοινωνεί το thread που κάνει την ολοκλήρωση με το thread που τρέχει τα γραφικά.
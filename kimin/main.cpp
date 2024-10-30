#include"../include/logo.hpp"
#include"../include/window.hpp"

int main()
{
    draw_logo_for_seconds("../logo/logo.jpg", 3.0f);

    Window window;
    window.game_loop();
    
    return 0;
}
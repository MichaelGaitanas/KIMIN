
/* This is the entry point of the KIMIN-GUI code. */

//#include"logo.h"
#include"window.h"

int main()
{
    //draw_logo_for_seconds("../logo/logo.jpg", 3.0f);

    window win;
    win.game_loop();
    
    return 0;
}
/* Entry point of KIMIN. */

#include"logo.h"
#include"window.h"

int main()
{
    render_logo("../logo/logo.jpg", 3.0f);

    window win;
    win.game_loop();
    
    return 0;
}
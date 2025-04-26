/* Entry point of KIMIN. */

#include"logo.h"
#include"window.h"

int main()
{
    render_logo_for_seconds("../logo/logo.jpg", 2.0f);

    window win;
    win.game_loop();
    
    return 0;
}
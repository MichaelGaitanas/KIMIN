/* Entry point of KIMIN. */

#include"logo.h"
#include"window.h"

int main()
{
    render_logo(PATH_TO_LOGO_IMAGE, 3.0f);

    window win;
    win.game_loop();
    
    return 0;
}
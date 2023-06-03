#include"../include/logo.hpp"
#include"../include/window.hpp"

int main()
{
    render_logo_for_a_few_seconds(3.0f);

    Window window;
    window.game_loop();

    return 0;
}
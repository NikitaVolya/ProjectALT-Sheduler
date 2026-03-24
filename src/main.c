
#include <stdlib.h>
#include <stdio.h>
#include "ui/app_window.h"

int main() {
    AppWindow *app_window;
    
    printf("MAIN.C\n");

    start_app_interface();

    app_window = create_app_window("test", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480, SDL_WINDOW_SHOWN, NULL);

    set_app_window_background_color(app_window, create_colorRGBA(255, 0, 0, 125));
    show_app_window(app_window);
    
    SDL_Delay(1000);

    close_app_interface();
    
    exit(EXIT_SUCCESS);
}

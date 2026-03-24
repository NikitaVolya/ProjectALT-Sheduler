
#include <stdlib.h>
#include <stdio.h>
#include "ui/app_window.h"

int main() {
    AppWindow *app_window;
    
    printf("MAIN.C\n");

    start_app_interface();

    app_window = create_app_window("test", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480, SDL_WINDOW_SHOWN);

    set_app_window_title(app_window, "Hello, world!"); 
    
    SDL_Delay(3000);

    quit_app_interface();
    
    exit(EXIT_SUCCESS);
}

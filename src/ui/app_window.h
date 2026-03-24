
#ifndef _APP_WINDOW_H_
#define _APP_WINDOW_H_

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include"../database/queue.h"


struct AppWindow {

    struct AppWindow *parent;
    SDL_Window *window;
    SDL_Renderer *renderer;

};

typedef struct AppWindow AppWindow;

/* globa functions */
void start_app_interface();

void quit_app_interface();

void raise_app_interface_error(const char *message, int code);

/* AppWindow functions */
AppWindow* create_app_window(const char* title,
                             int x, int y, int w, int h,
                             Uint32 flags);

const char* get_app_window_title(AppWindow *app_window);

void set_app_window_title(AppWindow *app_window, const char *title);

SDL_Window* get_app_windom_sdl_window(AppWindow *window);

SDL_Renderer* get_app_window_sdl_render(AppWindow *window);

AppWindow* destory_app_window(AppWindow *window);

#endif /* _APP_WINDOW_H_ */
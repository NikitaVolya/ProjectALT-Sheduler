#include"app_window.h"

static Queue *windows = NULL;

void free_app_window(void *value) {
    AppWindow *app_window = (AppWindow*) value;

    if (app_window != NULL) {
        if(app_window->renderer != NULL)
            SDL_DestroyRenderer(app_window->renderer);
        if(app_window->window != NULL)
            SDL_DestroyWindow(app_window->window);
        free(app_window);
    }
}

void start_app_interface() {

    if (windows != NULL) {
        raise_app_interface_error("AppInterface already stated\n", EXIT_FAILURE);
    }

    windows = create_queue();
    set_queue_element_free_function(windows, &free_app_window);

    SDL_Init(SDL_INIT_VIDEO);
}

void close_app_interface() {

    if (windows == NULL) {

        fprintf(stderr, "Before quit_app_interface, you must use start_app_interface\n");
        exit(EXIT_FAILURE);
    }

    free_queue(windows);
    SDL_Quit();
}

void raise_app_interface_error(const char *message, int code) {

    if (windows == NULL) {
        fprintf(stderr, "Before raise_app_interface_error, you must use start_app_interface\n");
        exit(EXIT_FAILURE);
    } 

    close_app_interface();
    fprintf(stderr, "%s", message);
    exit(code);
}

AppWindow* create_app_window(const char* title,
                             int x, int y, int w, int h,
                             Uint32 flags, AppWindow *parent) {
    AppWindow *res;

    if (windows == NULL) {
        fprintf(stderr, "Before create_app_window, you must use start_app_interface\n");
        exit(EXIT_FAILURE);
    } 
 
    if ((res = (AppWindow*) malloc(sizeof(AppWindow))) == NULL) {
        fprintf(stderr, "Error while memory allocation\n");
        return NULL;
    }

    res->window = SDL_CreateWindow(title, x, y, w, h, flags);
    if(NULL == res->window)
    {
        free(res);
        fprintf(stderr, "Error SDL_CreateWindow : %s", SDL_GetError());
        raise_app_interface_error("", EXIT_FAILURE);
    }

    SDL_HideWindow(res->window);

    res->renderer = SDL_CreateRenderer(res->window, -1, SDL_RENDERER_ACCELERATED);
    if(NULL == res->renderer)
    {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s", SDL_GetError());
        free(res->window);
        free(res);
        raise_app_interface_error("", EXIT_FAILURE);
    }

    res->parent = parent;

    push_queue_element(windows, res);

    return res;
}

void draw_app_window(AppWindow *app_window) {

    /* clearing window */
    if(SDL_SetRenderDrawColor(app_window->renderer, 
                              app_window->background_color.r, 
                              app_window->background_color.g, 
                              app_window->background_color.b, 
                              app_window->background_color.a) < 0)
        raise_app_interface_error("Error SDL_SetRenderDrawColor", EXIT_FAILURE);

    if(SDL_RenderClear(app_window->renderer) < 0)
        raise_app_interface_error("Error SDL_RenderClear", EXIT_FAILURE);
    
    /* dsiplay changes */
    SDL_RenderPresent(app_window->renderer);
}

void show_app_window(AppWindow *app_window) {
    SDL_ShowWindow(app_window->window);
    draw_app_window(app_window);
}

void hide_app_window(AppWindow *app_window) {
    SDL_HideWindow(app_window->window);
}

void show_dialog_app_window(AppWindow *app_window, int hide_parent) {

    if (hide_parent && app_window->parent != NULL)
        hide_app_window(app_window->parent);

    show_app_window(app_window);
}

void set_app_window_background_color(AppWindow *app_window, ColorRGBA color) {

    app_window->background_color = color;
    draw_app_window(app_window);
}

ColorRGBA get_app_window_background_color(AppWindow *app_window) {
    return app_window->background_color;
}

const char* get_app_window_title(AppWindow *app_window) {
    return SDL_GetWindowTitle(app_window->window);
}

void set_app_window_title(AppWindow *app_window, const char *title) {
    SDL_SetWindowTitle(app_window->window, title);
}

SDL_Window* get_app_windom_sdl_window(AppWindow *app_window) {
    return app_window->window;
}

SDL_Renderer* get_app_window_sdl_render(AppWindow *app_window) {
    return app_window->renderer;
}

AppWindow* destory_app_window(AppWindow *window) {
    AppWindow *res;

    res = window->parent;

    remove_queue_element(windows, window);
    free_app_window(window);

    return res;
}
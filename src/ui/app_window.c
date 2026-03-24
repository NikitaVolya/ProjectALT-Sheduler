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

void quit_app_interface() {

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

    quit_app_interface();
    fprintf(stderr, "%s", message);
    exit(code);
}

AppWindow* create_app_window(const char* title,
                             int x, int y, int w, int h,
                             Uint32 flags) {
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

    res->renderer = SDL_CreateRenderer(res->window, -1, SDL_RENDERER_ACCELERATED);
    if(NULL == res->renderer)
    {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s", SDL_GetError());
        free(res->window);
        free(res);
        raise_app_interface_error("", EXIT_FAILURE);
    }

    push_queue_element(windows, res);

    return res;
}

const char* get_window_titl(AppWindow *app_window) {
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
#include <SDL2/SDL.h>
#include <stdio.h>

#define WINDOW_TITLE "Test APP"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int exit;
};

int sdl_initialize(struct App *app) {
    int rep = SDL_Init(SDL_INIT_EVERYTHING);
    
    if (rep) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    } else {
        app->window = SDL_CreateWindow(WINDOW_TITLE,
                         SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT,
                         0);

    }
    if (!rep && !app->window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        rep = 1;
    } else {
        app->renderer = SDL_CreateRenderer(app->window, -1, 0);
    }

    if (!rep && !app->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        rep = 1;
    }
    
    return rep;
}

void app_cleanup(struct App *app) {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}

int main() {
    SDL_Event event;
    
    struct App app;
    app.window = NULL;
    app.renderer = NULL;
    app.exit = 0;

    if (sdl_initialize(&app)) {
        app_cleanup(&app);
        exit(EXIT_FAILURE);
    }

    while (!app.exit) {

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                app.exit = 1;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    app.exit = 1;
                    break;
                case SDL_SCANCODE_SPACE:
                    SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
        

        SDL_RenderClear(app.renderer);
        SDL_RenderPresent(app.renderer);

        SDL_Delay(10);

    }
    
    app_cleanup(&app);

    printf("App end\n");
    
    exit(EXIT_SUCCESS);
}

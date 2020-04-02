#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#define WIDTH 800
#define HEIGHT 500

/**
 * 

pthreads-w32-2.10-mingw32-pre-20160821-1-doc.tar.xz
pthreads-w32-2.10-mingw32-pre-20160821-1-lic.tar.xz
libpthreadgc-2.10-mingw32-pre-20160821-1-dll-3.tar.xz
libpthreadgc-2.10-mingw32-pre-20160821-1-dev.tar.xz
pthreads-w32-2.10-mingw32-pre-20160821-1-dev.tar.xz

**/

typedef struct RectangleElement {
    SDL_Rect* rect;
    struct RectangleElement* next;

} RectangleElement;

RectangleElement* createRectangleElement(SDL_Rect* rect) {
    RectangleElement* tmp = (RectangleElement*)malloc(sizeof(RectangleElement));
    tmp->rect = rect;
    tmp->next = NULL;
    return tmp;
}

void insertRectBegenning(RectangleElement** rects, SDL_Rect* rect) {
    RectangleElement* tmp = createRectangleElement(rect);
    tmp->next = *rects;
    *rects = tmp;
}

void displayRects(RectangleElement* rects, SDL_Renderer* renderer) {
    while (rects) {
        SDL_RenderFillRect(renderer, rects->rect);
        rects = rects->next;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = NULL;
    window = SDL_CreateWindow("First Windows", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);

    SDL_Rect rectangle;
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.h = 100;
    rectangle.w = 100;

    if (window == NULL) {
        printf("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    RectangleElement* bigRects = NULL;
    RectangleElement* smallRects = NULL;

    SDL_Rect* sr;
    SDL_Rect* br;

    SDL_Event windowEvent;
    int x, y;
    while (1) {
        if (SDL_PollEvent(&windowEvent)) {
            if (SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                sr = (SDL_Rect*)malloc(sizeof(SDL_Rect));
                sr->x = x;
                sr->y = y;
                sr->h = 5;
                sr->w = 5;
                insertRectBegenning(&smallRects, sr);
            }
            if (windowEvent.type == SDL_QUIT) {
                break;
            }
            if (windowEvent.type == SDL_KEYDOWN)
                switch (windowEvent.key.keysym.scancode) {
                    case SDL_SCANCODE_D:
                        if (rectangle.x < WIDTH - rectangle.w)
                            rectangle.x += 10;
                        break;
                    case SDL_SCANCODE_A:
                        if (rectangle.x > 0)
                            rectangle.x -= 10;
                        break;
                    case SDL_SCANCODE_W:
                        if (rectangle.y > 0)
                            rectangle.y -= 10;
                        break;
                    case SDL_SCANCODE_S:
                        if (rectangle.y < HEIGHT - rectangle.h)
                            rectangle.y += 10;
                        break;
                    case SDL_SCANCODE_SPACE:
                        br = (SDL_Rect*)malloc(sizeof(SDL_Rect));
                        br->x = rectangle.x;
                        br->y = rectangle.y;
                        br->h = 100;
                        br->w = 100;
                        insertRectBegenning(&bigRects, br);
                        break;

                    default:
                        break;
                }
        }

        SDL_SetRenderDrawColor(renderer, 171, 178, 185, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
        displayRects(bigRects, renderer);
        SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
        displayRects(smallRects, renderer);
        SDL_SetRenderDrawColor(renderer, 249, 101, 96, 255);
        SDL_RenderFillRect(renderer, &rectangle);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

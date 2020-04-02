#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH_WINDOW 600
#define HEIGHT_WINDOW WIDTH_WINDOW

#define NBR_RECT_H 20
#define NBR_RECT_V NBR_RECT_H

#define WIDTH_RECT WIDTH_WINDOW / NBR_RECT_H
#define HEIGHT_RECT WIDTH_RECT

#define COLOR_BG 234, 237, 237
#define COLOR_OBS 0, 237, 0
#define COLOR_RECT_1 255, 87, 51
#define COLOR_RECT_2 249, 191, 96
#define COLOR_RECT_3 199, 0, 57
#define COLOR_RECT_4 255, 195, 0

#define M_1 WIDTH_RECT * 5 / 100
#define M_2 WIDTH_RECT * 10 / 100
#define M_3 WIDTH_RECT * 15 / 100
#define M_4 WIDTH_RECT * 20 / 100

typedef struct Position {
    float x;
    float y;
} Position;

typedef struct ListPosition {
    Position* p;
    struct ListPosition* next;
} ListPosition;

typedef struct Rect {
    Position* p;
    char type;  // can't use them
    float F;
    float G;
    struct Rect* from;
    struct ListRect* friends;
    int nbrFriends;
} Rect;

typedef struct ListRect {
    Rect* rect;
    struct ListRect* next;
} ListRect;

float dist(Rect*, Rect*);
Rect* exist(ListRect*, Rect*);
Rect* createRect(Position*, char*, float, float);
void pushRect(ListRect**, Rect*);
void removeRect(ListRect**, Rect*);
float dist(Rect*, Rect*);

void initUI(Rect****);
void addFriends(Rect***, int, int);

void pushPosition(ListPosition**, Position*);

Rect* getMin(ListRect*);

ListPosition* A_Start(Rect**, Rect**);

// -----------------SDL----------------------//
void drawRects(Rect***, SDL_Renderer*, SDL_Texture*, SDL_Texture*, SDL_Texture*);
void drawPath(ListPosition*, SDL_Renderer*);
void drawStartEnd(Rect*, Rect*, SDL_Renderer*);
SDL_Texture* loadTexture(const char* path, SDL_Renderer* gRenderer) {
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    } else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    printf("image loaded\n");
    return newTexture;
}
// -----------------SDL----------------------//

int main(int argc, char* argv[]) {
    Rect* start;
    Rect* end;

    Rect*** rectsUI;
    initUI(&rectsUI);

    int x, y;
    x = y = 0;

    start = rectsUI[x][y];
    start->type = '2';
    start->from = NULL;
    end = *(*(rectsUI + NBR_RECT_H - 1) + NBR_RECT_V - 1);
    end->type = '3';
    ListPosition* path = A_Start(&start, &end);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    //Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = NULL;
    window = SDL_CreateWindow("ENNBOU", 100, 100, WIDTH_WINDOW, HEIGHT_WINDOW, SDL_WINDOW_OPENGL);

    // SDL_Surface* screen = NULL;
    // screen = SDL_GetWindowSurface(window);

    if (window == NULL) {
        printf("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event windowEvent;

    // ------ sound ----------//
    Mix_Music* music = NULL;
    music = Mix_LoadMUS("SDL_App/beat.wav");
    if (music == NULL) {
        return 1;
    }

    // end------ sound ----------//

    // ------ img ------------ //

    SDL_Texture *imgObstacle, *imgPlayer, *imgEnd;
    imgObstacle = loadTexture("SDL_App/obstacle.png", renderer);
    SDL_Texture** imgPlayers = (SDL_Texture**)malloc(sizeof(SDL_Texture*) * 4);
    *(imgPlayers + 0) = loadTexture("SDL_App/left.png", renderer);
    *(imgPlayers + 1) = loadTexture("SDL_App/right.png", renderer);
    *(imgPlayers + 2) = loadTexture("SDL_App/up.png", renderer);
    *(imgPlayers + 3) = loadTexture("SDL_App/down.png", renderer);

    imgPlayer  = *(imgPlayers + 3);
    imgEnd  = loadTexture("SDL_App/end.png", renderer);;

    //END ------ img ------------ //

    while (1) {
        if (SDL_PollEvent(&windowEvent)) {
            SDL_SetRenderDrawColor(renderer, COLOR_BG, 255);
            SDL_RenderClear(renderer);
            if (windowEvent.type == SDL_MOUSEBUTTONDOWN) {
                int a = windowEvent.motion.x;
                int b = windowEvent.motion.y;
                a = a / (WIDTH_RECT);
                b = b / (HEIGHT_RECT);
                (rectsUI[a][b])->type = '1';
                path = A_Start(&start, &end);
            }

            if (windowEvent.type == SDL_KEYDOWN) {
                switch (windowEvent.key.keysym.sym) {
                    case SDLK_1:
                        if (Mix_PlayingMusic() == 0) {
                            //Play the music
                            if (Mix_PlayMusic(music, 1) == -1) {
                                return 1;
                            }
                        }
                        break;
                    case SDLK_LEFT:
                        if (x == 0 || (rectsUI[x - 1][y])->type == '1' || rectsUI[x - 1][y] == end) {
                            Mix_PlayMusic(music, 1);
                            break;
                        }
                        (rectsUI[x--][y])->type = '\0';
                        imgPlayer = *(imgPlayers + 0);
                        (rectsUI[x][y])->type = '2';
                        break;
                    case SDLK_RIGHT:
                        if (x == NBR_RECT_H - 1 || (rectsUI[x + 1][y])->type == '1' || rectsUI[x + 1][y] == end) {
                            if (Mix_PlayingMusic() == 0) {
                                if (Mix_PlayMusic(music, 1) == -1) {
                                    return 1;
                                }
                            }
                            break;
                        }
                        (rectsUI[x++][y])->type = '\0';
                        imgPlayer = *(imgPlayers + 1);
                        (rectsUI[x][y])->type = '2';
                        break;
                    case SDLK_UP:
                        if (y == 0 || (rectsUI[x][y - 1])->type == '1' || rectsUI[x][y - 1] == end) {
                            if (Mix_PlayingMusic() == 0) {
                                if (Mix_PlayMusic(music, 1) == -1) {
                                    return 1;
                                }
                            }
                            break;
                        }
                        (rectsUI[x][y--])->type = '\0';
                        imgPlayer = *(imgPlayers + 2);
                        (rectsUI[x][y])->type = '2';
                        break;
                    case SDLK_DOWN:
                        if (y == NBR_RECT_H - 1 || (rectsUI[x][y + 1])->type == '1' || rectsUI[x][y + 1] == end) {
                            if (Mix_PlayingMusic() == 0) {
                                if (Mix_PlayMusic(music, 1) == -1) {
                                    return 1;
                                }
                            }
                            break;
                        }
                        (rectsUI[x][y++])->type = '\0';
                        imgPlayer = *(imgPlayers + 3);
                        (rectsUI[x][y])->type = '2';
                        break;
                }
                start = rectsUI[x][y];
            }

            if (windowEvent.type == SDL_QUIT) {
                break;
            }
            drawRects(rectsUI, renderer, imgObstacle, imgPlayer, imgEnd);
            path = A_Start(&start, &end);
            drawPath(path, renderer);
            // drawStartEnd(start, end, renderer);
            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

ListPosition* A_Start(Rect** start, Rect** end) {
    if (*start == *end) return NULL;
    ListPosition* path = NULL;
    ListRect* openRects = NULL;
    ListRect* closedRects = NULL;

    (*start)->from = NULL;

    Rect* current;
    int g;
    pushRect(&openRects, *start);
    while (openRects) {
        current = getMin(openRects);
        if (current == *end) {
            break;
        }

        pushRect(&closedRects, current);
        removeRect(&openRects, current);
        ListRect* friend = current->friends;
        while (friend) {
            if (exist(closedRects, friend->rect) || friend->rect->type == '1') {
                friend = friend->next;
                continue;
            }
            g = current->G + dist(current, friend->rect);
            if (!exist(openRects, friend->rect)) {
                pushRect(&openRects, friend->rect);
            } else if (g > friend->rect->G) {
                friend = friend->next;
                continue;
            }
            friend->rect->from = current;
            friend->rect->G = g;
            friend->rect->F = g + dist(friend->rect, *end);

            friend = friend->next;
        }
    }
    if (current != *end) return NULL;
    Rect* rect = closedRects->rect;
    while (rect->from) {
        pushPosition(&path, rect->p);
        rect = rect->from;
    }
    return path;
}

Rect* getMin(ListRect* rects) {
    Rect* rect = rects->rect;
    while (rects) {
        if (rects->rect->F < rect->F)
            rect = rects->rect;
        rects = rects->next;
    }
    return rect;
}

void pushRect(ListRect** rects, Rect* rect) {
    ListRect* e = (ListRect*)malloc(sizeof(ListRect));
    e->rect = rect;
    e->next = *rects;
    *rects = e;
}

void removeRect(ListRect** rects, Rect* rect) {
    ListRect* tmp = *rects;
    ListRect* prec = tmp;
    while (tmp) {
        if (tmp->rect == rect) {
            if (tmp->rect == (*rects)->rect) {
                *rects = tmp->next;
                return;
            }
            prec->next = tmp->next;
            return;
        }
        prec = tmp;
        tmp = tmp->next;
    }
}

Rect* exist(ListRect* rects, Rect* rect) {
    while (rects) {
        if (rects->rect == rect) return rect;
        rects = rects->next;
    }
    return NULL;
}

float dist(Rect* rect1, Rect* rect2) {
    return sqrt(pow(rect1->p->x - rect2->p->x, 2) + pow(rect1->p->y - rect2->p->y, 2));
}

void initUI(Rect**** rects) {
    *rects = (Rect***)malloc(sizeof(Rect**) * NBR_RECT_H);
    int i = 0;
    int j;
    while (i < NBR_RECT_H) {
        *(*rects + i) = (Rect**)malloc(sizeof(Rect*) * NBR_RECT_V);
        j = 0;
        while (j < NBR_RECT_H) {
            *(*(*rects + i) + j) = (Rect*)malloc(sizeof(Rect));
            (*(*(*rects + i) + j))->p = (Position*)malloc(sizeof(Position));
            (*(*(*rects + i) + j))->p->x = i * WIDTH_RECT;
            (*(*(*rects + i) + j))->p->y = j * HEIGHT_RECT;
            (*(*(*rects + i) + j))->friends = NULL;
            (*(*(*rects + i) + j))->type = '\0';
            j++;
        }
        i++;
    }
    i = 0;
    while (i < NBR_RECT_H) {
        j = 0;
        while (j < NBR_RECT_H) {
            addFriends(*rects, i, j);
            j++;
        }
        i++;
    }
}

void addFriends(Rect*** rects, int i, int j) {
    ListRect* rect = NULL;

    if (i > 0)  // left
        pushRect(&rect, *((*(rects + i - 1)) + j));
    if (i < NBR_RECT_H - 1)  // right
        pushRect(&rect, *((*(rects + i + 1)) + j));
    if (j > 0)  // top
        pushRect(&rect, *((*(rects + i)) + j - 1));
    if (j < NBR_RECT_V - 1)  // bottom
        pushRect(&rect, *((*(rects + i)) + j + 1));
    if (i < NBR_RECT_H - 1 && j < NBR_RECT_V - 1)  // right bottom
        pushRect(&rect, *((*(rects + i + 1)) + j + 1));
    if (i < NBR_RECT_H - 1 && j > 0)  // right top
        pushRect(&rect, *((*(rects + i + 1)) + j - 1));
    if (i > 0 && j < NBR_RECT_V - 1)  // left bottom
        pushRect(&rect, *((*(rects + i - 1)) + j + 1));
    if (i > 0 && j > 0)  // left top
           pushRect(&rect, *((*(rects + i - 1)) + j - 1));

    (*(*(rects + i) + j))->friends = rect;
}

void pushPosition(ListPosition** path, Position* p) {
    ListPosition* l = (ListPosition*)malloc(sizeof(ListPosition));
    l->p = p;
    l->next = *path;
    *path = l;
}

//-------------------------------------
void drawRects(Rect*** rects, SDL_Renderer* renderer, SDL_Texture* imgObstacle, SDL_Texture* imgPlayer, SDL_Texture* imgEnd) {
    int i, j;
    i = j = 0;
    while (i < NBR_RECT_H) {
        j = 0;
        while (j < NBR_RECT_V) {
            SDL_Rect* rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
            rect->x = (*(*(rects + i) + j))->p->x + M_1;
            rect->y = (*(*(rects + i) + j))->p->y + M_1;
            rect->w = WIDTH_RECT - M_1 * 2;
            rect->h = HEIGHT_RECT - M_1 * 2;
            if ((*(*(rects + i) + j))->type == '1') {
                SDL_RenderCopy(renderer, imgObstacle, NULL, rect);
            } else if ((*(*(rects + i) + j))->type == '2') {
                SDL_RenderCopy(renderer, imgPlayer, NULL, rect);
            } else if ((*(*(rects + i) + j))->type == '3') {
                SDL_RenderCopy(renderer, imgEnd, NULL, rect);
            }
            // SDL_SetRenderDrawColor(renderer, COLOR_OBS, 255);
            // SDL_RenderFillRect(renderer, rect);
            j++;
        }
        i++;
    }
}

void drawPath(ListPosition* path, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, COLOR_RECT_2, 10);
    while (path) {
        SDL_Rect* rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
        rect->x = path->p->x + M_4;
        rect->y = path->p->y + M_4;
        rect->w = WIDTH_RECT - M_4 * 2;
        rect->h = HEIGHT_RECT - M_4 * 2;
        SDL_RenderFillRect(renderer, rect);
        path = path->next;
    }
}

void drawStartEnd(Rect* start, Rect* end, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, COLOR_RECT_3, 100);
    SDL_Rect* rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    rect->x = start->p->x;
    rect->y = start->p->y;
    rect->w = WIDTH_RECT;
    rect->h = HEIGHT_RECT;
    SDL_RenderFillRect(renderer, rect);
    rect->x = end->p->x;
    rect->y = end->p->y;
    SDL_RenderFillRect(renderer, rect);
}
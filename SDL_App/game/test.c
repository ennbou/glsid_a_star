#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define WIDTH_WINDOW 600
#define HEIGHT_WINDOW WIDTH_WINDOW

#define NBR_RECT_H 10
#define NBR_RECT_V NBR_RECT_H

#define WIDTH_RECT WIDTH_WINDOW / NBR_RECT_H
#define HEIGHT_RECT WIDTH_RECT

#define COLOR_BG 234, 237, 237
#define COLOR_OBS 0, 237, 0
#define COLOR_RECT_1 255, 87, 51
#define COLOR_RECT_2 218, 247, 166
#define COLOR_RECT_3 199, 0, 57
#define COLOR_RECT_4 255, 195, 0

#define M_1 WIDTH_RECT * 5 / 100
#define M_2 WIDTH_RECT * 10 / 100
#define M_3 WIDTH_RECT * 15 / 100
#define M_4 WIDTH_RECT * 20 / 100
#define M_5 WIDTH_RECT * 40 / 100
#define M_6 WIDTH_RECT * 60 / 100

typedef struct Position
{
    float x;
    float y;
} Position;

typedef struct ListPosition
{
    Position *p;
    struct ListPosition *next;
} ListPosition;

typedef struct Rect
{
    Position *p;
    char obstacles; // can't use them
    float F;
    float G;
    struct Rect *from;
    struct ListRect *friends;
    int nbrFriends;
} Rect;

typedef struct ListRect
{
    Rect *rect;
    struct ListRect *next;
} ListRect;

float dist(Rect *, Rect *);
Rect *exist(ListRect *, Rect *);
Rect *createRect(Position *, char *, float, float);
void pushRect(ListRect **, Rect *);
void removeRect(ListRect **, Rect *);
float dist(Rect *, Rect *);

void initUI(Rect ****);
void addFriends(Rect ***, int, int);

void pushPosition(ListPosition **, Position *);

Rect *getMin(ListRect *);

ListPosition *A_Start(Rect **, Rect **);

// -----------------SDL----------------------//
void drawRects(Rect ***, SDL_Renderer *);
void drawPath(ListPosition *, SDL_Renderer *);
void drawStartEnd(Rect *, Rect *, SDL_Renderer *);
// -----------------SDL----------------------//

int main(int argc, char *argv[])
{
    Rect *start;
    Rect *end;

    Rect ***rectsUI;
    initUI(&rectsUI);

    start = *(*rectsUI);
    start->from = NULL;
    end = *(*(rectsUI + NBR_RECT_H - 1) + NBR_RECT_V - 1);

    ListPosition *path = A_Start(&start, &end);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = NULL;
    window = SDL_CreateWindow("ENNBOU", 100, 100, WIDTH_WINDOW, HEIGHT_WINDOW, SDL_WINDOW_OPENGL);

    if (window == NULL)
    {
        printf("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event windowEvent;

    //END ------ img ------------ //

    while (1)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = windowEvent.motion.x;
                int y = windowEvent.motion.y;
                x = x / (WIDTH_RECT);
                y = y / (HEIGHT_RECT);
                if ((rectsUI[x][y])->obstacles == '1')
                {
                    (rectsUI[x][y])->obstacles = '\0';
                }
                else
                {
                    (rectsUI[x][y])->obstacles = '1';
                }
                path = A_Start(&start, &end);
            }

            if (windowEvent.type == SDL_QUIT)
            {
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, COLOR_BG, 255);
        SDL_RenderClear(renderer);
        drawRects(rectsUI, renderer);
        SDL_SetRenderDrawColor(renderer, COLOR_RECT_2, 100);
        drawPath(path, renderer);
        SDL_SetRenderDrawColor(renderer, COLOR_RECT_3, 100);
        drawStartEnd(start, end, renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

ListPosition *A_Start(Rect **start, Rect **end)
{

    // struct timeval stop, startt;
    // gettimeofday(&startt, NULL);

    ListPosition *path = NULL;
    ListRect *openRects = NULL;
    ListRect *closedRects = NULL;

    Rect *current;
    int g;
    pushRect(&openRects, *start);
    while (openRects)
    {
        current = getMin(openRects);
        if (current == *end)
        {
            break;
        }

        pushRect(&closedRects, current);
        removeRect(&openRects, current);
        ListRect *friend = current->friends;
        while (friend)
        {
            if (exist(closedRects, friend->rect) || friend->rect->obstacles)
            {
                friend = friend->next;
                continue;
            }
            g = current->G + dist(current, friend->rect);
            if (!exist(openRects, friend->rect))
            {
                pushRect(&openRects, friend->rect);
            }
            else if (g > friend->rect->G)
            {
                friend = friend->next;
                continue;
            }
            friend->rect->from = current;
            friend->rect->G = g;
            friend->rect->F = g + dist(friend->rect, *end);

            friend = friend->next;
        }
    }
    if (current != *end)
        return NULL;
    Rect *rect = closedRects->rect;
    while (rect->from)
    {
        pushPosition(&path, rect->p);
        rect = rect->from;
    }

    /**
     * une fonction pour ajouter a la fine d'un fichier le nombre des nodes cree et 
     * les node teste et aussi le temps depuis le debut de l'algorithne jusqu'au la fine
     * void writeInFile(int, int, int, int); 
    */

    // int numberOfNodesInOpenList = getNumberOfNodes(openRects);
    // int numberOfNodesInCloseList = getNumberOfNodes(closedRects);

    // gettimeofday(&stop, NULL);

    // writeInFile(numberOfNodesInOpenList, numberOfNodesInCloseList, startt, stop);

    return path;
}

Rect *getMin(ListRect *rects)
{
    Rect *rect = rects->rect;
    while (rects)
    {
        if (rects->rect->F < rect->F) // g + h
            rect = rects->rect;
        rects = rects->next;
    }
    return rect;
}

void pushRect(ListRect **rects, Rect *rect)
{
    ListRect *e = (ListRect *)malloc(sizeof(ListRect));
    e->rect = rect;
    e->next = *rects;
    *rects = e;
}

void removeRect(ListRect **rects, Rect *rect)
{
    ListRect *tmp = *rects;
    ListRect *prec = tmp;
    while (tmp)
    {
        if (tmp->rect == rect)
        {
            if (tmp->rect == (*rects)->rect)
            {
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

Rect *exist(ListRect *rects, Rect *rect)
{
    while (rects)
    {
        if (rects->rect == rect)
            return rect;
        rects = rects->next;
    }
    return NULL;
}

float dist(Rect *rect1, Rect *rect2)
{
    // return abs(rect1->p->x - rect2->p->x) + abs(rect1->p->y - rect2->p->y);
    float X = abs(rect1->p->x - rect2->p->x);
    float Y = abs(rect1->p->y - rect2->p->y);
    return sqrt(X * X + Y * Y);
}

// float heuristic(Rect *start, Rect *end){
//     float dx = abs(start->x - end->x);
//     float dy = abs(start->y - end->y);
//     return D * (dx + dy) + (D2 - 2 * D) * min(dx, dy);
//     // D * max(dx, dy) + (D2-D) * min(dx, dy).
// }

void initUI(Rect ****rects)
{
    *rects = (Rect ***)malloc(sizeof(Rect **) * NBR_RECT_H);
    int i = 0;
    int j;
    while (i < NBR_RECT_H)
    {
        *(*rects + i) = (Rect **)malloc(sizeof(Rect *) * NBR_RECT_V);
        j = 0;
        while (j < NBR_RECT_H)
        {
            *(*(*rects + i) + j) = (Rect *)malloc(sizeof(Rect));
            (*(*(*rects + i) + j))->p = (Position *)malloc(sizeof(Position));
            (*(*(*rects + i) + j))->p->x = i * WIDTH_RECT;
            (*(*(*rects + i) + j))->p->y = j * HEIGHT_RECT;
            (*(*(*rects + i) + j))->friends = NULL;
            (*(*(*rects + i) + j))->obstacles = '\0';
            j++;
        }
        i++;
    }
    i = 0;
    while (i < NBR_RECT_H)
    {
        j = 0;
        while (j < NBR_RECT_H)
        {
            addFriends(*rects, i, j);
            j++;
        }
        i++;
    }
}

void addFriends(Rect ***rects, int i, int j)
{
    ListRect *rect = NULL;

    if (i > 0) // left
        pushRect(&rect, *((*(rects + i - 1)) + j));
    if (i < NBR_RECT_H - 1) // right
        pushRect(&rect, *((*(rects + i + 1)) + j));
    if (j > 0) // top
        pushRect(&rect, *((*(rects + i)) + j - 1));
    if (j < NBR_RECT_V - 1) // bottom
        pushRect(&rect, *((*(rects + i)) + j + 1));
    if (i < NBR_RECT_H - 1 && j < NBR_RECT_V - 1) // right bottom
        pushRect(&rect, *((*(rects + i + 1)) + j + 1));
    if (i < NBR_RECT_H - 1 && j > 0) // right top
        pushRect(&rect, *((*(rects + i + 1)) + j - 1));
    if (i > 0 && j < NBR_RECT_V - 1) // left bottom
        pushRect(&rect, *((*(rects + i - 1)) + j + 1));
    if (i > 0 && j > 0) // left top
        pushRect(&rect, *((*(rects + i - 1)) + j - 1));

    (*(*(rects + i) + j))->friends = rect;
}

void pushPosition(ListPosition **path, Position *p)
{
    ListPosition *l = (ListPosition *)malloc(sizeof(ListPosition));
    l->p = p;
    l->next = *path;
    *path = l;
}

//-------------------------------------
void drawRects(Rect ***rects, SDL_Renderer *renderer)
{
    int i, j;
    i = j = 0;
    while (i < NBR_RECT_H)
    {
        j = 0;
        while (j < NBR_RECT_V)
        {
            SDL_Rect *rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
            rect->x = (*(*(rects + i) + j))->p->x + M_1;
            rect->y = (*(*(rects + i) + j))->p->y + M_1;
            rect->w = WIDTH_RECT - M_1 * 2;
            rect->h = HEIGHT_RECT - M_1 * 2;
            if ((*(*(rects + i) + j))->obstacles)
                SDL_SetRenderDrawColor(renderer, COLOR_RECT_1, 255);
            else
                SDL_SetRenderDrawColor(renderer, COLOR_OBS, 255);
            SDL_RenderFillRect(renderer, rect);
            j++;
        }
        i++;
    }
}

void drawPath(ListPosition *path, SDL_Renderer *renderer)
{
    while (path)
    {
        SDL_Rect *rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
        rect->x = path->p->x + M_2;
        rect->y = path->p->y + M_2;
        rect->w = WIDTH_RECT - M_2 * 2;
        rect->h = HEIGHT_RECT - M_2 * 2;
        SDL_RenderFillRect(renderer, rect);
        path = path->next;
    }
}

void drawStartEnd(Rect *start, Rect *end, SDL_Renderer *renderer)
{
    SDL_Rect *rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    rect->x = start->p->x + M_2;
    rect->y = start->p->y + M_2;
    rect->w = WIDTH_RECT - M_2 * 2;
    rect->h = HEIGHT_RECT - M_2 * 2;
    SDL_RenderFillRect(renderer, rect);
    rect->x = end->p->x + M_2;
    rect->y = end->p->y + M_2;
    SDL_RenderFillRect(renderer, rect);
}

int getNumberOfNodes(ListPosition *list)
{
    int i = 0;

    while (list)
    {
        i++;
        list = list->next;
    }
    return i;
}

// opened nodes
void writeInFile(int nodesOpen, int nodesClose, struct timeval startTime, struct timeval endTime)
{
    FILE *file = NULL;
    long time = (endTime.tv_sec - startTime.tv_sec) * 1000000 + endTime.tv_usec - startTime.tv_usec;

    file = fopen("test.txt", "a+");
    fprintf(file, "--------------Distance Euclidienne------------------------------\n");
    fprintf(file, " ###### WIDTH  : %d node\n", NBR_RECT_V);
    fprintf(file, " ###### HIEGHT : %d node\n", NBR_RECT_H);
    fprintf(file, " ########### node size (H/W) : %d/%d node\n", WIDTH_RECT, HEIGHT_RECT);
    fprintf(file, "time find path : %ld microsecond\n", time);
    fprintf(file, "number of nodes open : %d \n", nodesOpen);
    fprintf(file, "number of nodes close : %d \n", nodesClose);
    fprintf(file, "----------------------------------------------------------------\n");
    fclose(file);
}

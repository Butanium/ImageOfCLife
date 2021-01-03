#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image/stb_image.h"
#include <SDL.h>


typedef struct nbInfo neighboursInfo;

//todo changer la fonc pixel en traçant des rectangles à la place des pixels et faire fonction
// actualiser screenp
SDL_Color initSDL_Color(unsigned char *rgb) {
    SDL_Color p = {rgb[0], rgb[1], rgb[2], 255};
    return p;
}

struct nbInfo {
    SDL_Color averageDeathColor;
    SDL_Color averageLiveColor;
    char nbAliveNeighbours;
};

const int ITERATION_COUNT = 500;
const int MILLIS_DELAY = 1000/60;
unsigned char deathColor[3] = {0, 0, 0};
unsigned char lifeColor[3];
double tolerance = 30;
unsigned char deathRules[] = {1, 4, 5, 6, 7};
unsigned char birthRules[] = {3};
unsigned char dRulesLen, bRulesLen;
int rectangleSize, xOffset, yOffset;

void printSDL_Color(SDL_Color p);

char isCellAlive(int c, int p, SDL_Color **readMatrix);

void refresh(int row, int col, SDL_Color **stateMatrix, SDL_Color **writeMatrix);

void renderMatrix(SDL_Color **stateMatrix, int row, int col, SDL_Renderer *renderer);

char neighbourList[8][2] = {{0,  1},
                            {0,  -1},
                            {1,  0},
                            {1,  1},
                            {1,  -1},
                            {-1, 0},
                            {-1, 1},
                            {-1, -1}};

neighboursInfo getNeighboursInfo(int c, int p, char is_a, int row, int col, SDL_Color **readMatrix);

void printColorMatrix(SDL_Color **colorMatrix, int height, int width);

int contains(unsigned char element, unsigned char list[], int listSize) {
    for (int i = 0; i < listSize; i++) {
        if (element == list[i]) {
            return 1;
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    FILE *debugFile = fopen("C:\\Users\\Clement\\Documents\\coding\\ImageOfCLife\\debug.txt", "w+");
    int imgWidth, imgHeight, channels;
    unsigned char *img = stbi_load("C:\\Users\\Clement\\Documents\\coding\\ImageOfCLife\\fractal1.jpg", &imgWidth,
                                   &imgHeight, &channels, 0);
    fprintf(debugFile, "Loaded image with a width of %dpx, a imgHeight of %dpx and %d channels\n", imgWidth, imgHeight,
            channels);
    dRulesLen = sizeof(deathRules);
    bRulesLen = sizeof(birthRules);
    if (img == NULL) {
        fprintf(debugFile, "Error in loading the image\n");
        exit(3);
    }

    int ch, pix;
    SDL_Color **stateMatrix1 = malloc(imgHeight * sizeof(SDL_Color *));
    if (stateMatrix1 == NULL) {
        fprintf(debugFile, "Unable to allocate memory\n");
        exit(1);
    }
    for (int i = 0; i < imgHeight; ++i) {
        stateMatrix1[i] = malloc(imgWidth * sizeof(SDL_Color));
    }

    for (ch = 0; ch < imgHeight; ch++) {
        printf("{");
        for (pix = 0; pix < imgWidth; pix++) {
            unsigned bytePerSDL_Color = channels;
            unsigned char *SDL_ColorOffset = img + (pix + imgHeight * ch) * bytePerSDL_Color;
            SDL_Color p = initSDL_Color(SDL_ColorOffset);
            stateMatrix1[ch][pix] = p;
            printSDL_Color(p);
            printf(", ");
        }
        printf("}\n");
    }
    SDL_Color **stateMatrix2 = malloc(imgHeight * sizeof(SDL_Color *));
    if (stateMatrix2 == NULL) {
        fprintf(debugFile, "Unable to allocate memory\n");
        exit(1);
    }
    for (int i = 0; i < imgHeight; ++i) {
        stateMatrix2[i] = malloc(imgWidth * sizeof(SDL_Color));
    }

    for (int i = 0; i < imgHeight; ++i) {
        memcpy(stateMatrix2[i], stateMatrix1[i], imgWidth * sizeof(SDL_Color));
    }
    for (char i = 0; i < 3; ++i) {
        lifeColor[i] = 255 - deathColor[i];
    }
    //refresh(imgHeight, width, stateMatrix1, (SDL_Color **) stateMatrix2);
    //The window we'll be rendering to
    SDL_Window *window = NULL;

    //The surface contained by the window
    //4SDL_Surface *screenSurface = NULL;
    SDL_DisplayMode DM;
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(debugFile, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        //Create window
        SDL_GetCurrentDisplayMode(0, &DM);
        int windowHeight, windowWidth;
        float heightRelation = (float) imgHeight / (.9f * (float) DM.h - 4.f);
        float widthRelation = (float) imgWidth / (.9f * (float) DM.w - 4.f);
        if (max(widthRelation, heightRelation) > 1) {
            fprintf(debugFile, "image is to big, resize not implemented yet\n");
            exit(5);
        }
        yOffset = 2;
        xOffset = 2;

        if (heightRelation < widthRelation) {
            windowWidth = (int) (.9f * (float) DM.w);
            windowHeight = (int) ceilf((float) imgHeight / widthRelation);
            rectangleSize = (int) (1.f / widthRelation);
            windowHeight += max(0, 4 - (windowHeight % rectangleSize));
        } else {
            windowHeight = (int) (.9f * (float) DM.h);
            windowWidth = (int) ceilf((float) imgWidth / heightRelation);
            rectangleSize = (int) (1.f / heightRelation);
            windowWidth += max(0, 4 - (windowWidth % rectangleSize));

        }

        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth,
                                  windowHeight, SDL_WINDOW_SHOWN);

        if (window == NULL) {
            fprintf(debugFile, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        } /*else {
            //Get window surface
            screenSurface = SDL_GetWindowSurface(window);

            //Fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

            //Update the surface
            SDL_UpdateWindowSurface(window);

            //Wait two seconds
            SDL_Delay(2000);
        }*/
    }
    SDL_Renderer *renderer = NULL;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Set render color to black ( background will be rendered in this color )
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // Clear window
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

//    for(int i = 0; i<500; ++i){
//        for (int j; j < 500; ++j) {
//            addPixelToRenderer(i, j, initSDL_Color(lifeColor), renderer);
//        }
//    }
//    SDL_RenderPresent(renderer);
//    SDL_Delay(10000);
    //   SDL_RenderClear(renderer);
    /*for(int i = 0; i<imgWidth; i+=1) {
        for (int j = 0 ; j < imgHeight; j+=1) {
            SDL_SetRenderDrawColor(renderer, (j%2)*255,(i%2)*255,0,255);
            SDL_Rect r;
            r.x = i*rectangleSize;
            r.y = j*rectangleSize;
            r.w = rectangleSize;
            r.h = rectangleSize;
           SDL_RenderFillRect(renderer, &r);

        }
    }

    SDL_RenderPresent(renderer);*/
    renderMatrix(stateMatrix1, imgHeight, imgWidth, renderer);
    SDL_PumpEvents();
    SDL_Delay(8000);
    char title[30];
    for(int i = 0; i < ITERATION_COUNT; ++i){
        if(i%2){
            refresh(imgHeight, imgWidth, stateMatrix2,stateMatrix1 );
            renderMatrix(stateMatrix1, imgHeight, imgWidth, renderer);
        } else{
            refresh(imgHeight, imgWidth, stateMatrix1,stateMatrix2 );
            renderMatrix(stateMatrix1, imgHeight, imgWidth, renderer);
        }
        SDL_Delay(MILLIS_DELAY);
        SDL_PumpEvents();
        sprintf(title, "frame : %i", i+1);
        SDL_SetWindowTitle(window, title);

    }

    SDL_Delay(3000);
    SDL_DestroyWindow(window);
    SDL_Quit();

    for (int i = 0; i < imgHeight; ++i) {
        free(stateMatrix1[i]);
    }
    free(stateMatrix1);
    for (int i = 0; i < imgHeight; ++i) {
        free(stateMatrix2[i]);
    }
    free(stateMatrix2);

    return EXIT_SUCCESS;
}


void printSDL_Color(SDL_Color p) {
    printf("[r:%i, g:%i, b:%i]", p.r, p.g, p.b);
}

char isCellAlive(int c, int p, SDL_Color **readMatrix) {
    double s = 0;
    SDL_Color pix = readMatrix[c][p];
    s += abs(pix.r - deathColor[0]);
    s += abs(pix.g - deathColor[1]);
    s += abs(pix.b - deathColor[2]);
    return s > tolerance;
}


neighboursInfo getNeighboursInfo(int c, int p, char is_a, int row, int col, SDL_Color **readMatrix) {
    char nbDeathN = 0;
    char nbAliveN = 0;
    double sumAColor[3] = {0, 0, 0};
    double sumDColor[3] = {0, 0, 0};
    for (int i = 0; i < 8; ++i) {
        int x = neighbourList[i][0] + c;
        int y = neighbourList[i][1] + p;
        if (0 <= x && x < row && 0 <= y && y < col) {
            SDL_Color neighbour = readMatrix[x][y];
            if (isCellAlive(x, y, readMatrix)) {
                nbAliveN++;
                sumAColor[0] += neighbour.r;
                sumAColor[1] += neighbour.g;
                sumAColor[2] += neighbour.b;
            } else {
                nbDeathN++;
                sumDColor[0] += neighbour.r;
                sumDColor[1] += neighbour.g;
                sumDColor[2] += neighbour.b;
            }
        }
    }
    unsigned char avAColor[3];
    unsigned char avDColor[3];
    SDL_Color cell = readMatrix[c][p];
    if (is_a) {
        avAColor[0] = cell.r;
        avAColor[1] = cell.g;
        avAColor[2] = cell.b;
        avDColor[0] = deathColor[0];
        avDColor[1] = deathColor[1];
        avDColor[2] = deathColor[2];

    } else {
        avDColor[0] = cell.r;
        avDColor[1] = cell.g;
        avDColor[2] = cell.b;
        avAColor[0] = 0;
        avAColor[1] = 0;
        avAColor[2] = 0;
    }
    for (char i = 0; i < 3; ++i) {
        avAColor[i] = is_a != 0 || nbAliveN != 0 ? (unsigned char) ((avAColor[i] + sumAColor[i]) / (is_a + nbAliveN))
                                                 : lifeColor[i];
        avDColor[i] = (unsigned char) ((avDColor[i] + sumDColor[i]) / (nbDeathN + 1));
    }
    neighboursInfo result = {initSDL_Color(avDColor), initSDL_Color(avAColor), nbAliveN};
    return result;
}

void refresh(int row, int col, SDL_Color **readMatrix, SDL_Color **writeMatrix) {
    for (int c = 0; c < row; ++c) {
        for (int p = 0; p < col; ++p) {
            char is_a = isCellAlive(c, p, readMatrix);
            neighboursInfo nbInfo = getNeighboursInfo(c, p, is_a, row, col, readMatrix);
            if (is_a) {
                if (contains(nbInfo.nbAliveNeighbours, deathRules, dRulesLen)) {
                    writeMatrix[c][p] = nbInfo.averageDeathColor;
                } else {
                    writeMatrix[c][p] = nbInfo.averageLiveColor;
                }
            } else {
                if (contains(nbInfo.nbAliveNeighbours, birthRules, bRulesLen)) {
                    writeMatrix[c][p] = nbInfo.averageLiveColor;
                } else {
                    writeMatrix[c][p] = nbInfo.averageDeathColor;
                }
            }

        }
    }
}

void renderMatrix(SDL_Color **stateMatrix, int row, int col, SDL_Renderer *renderer) {
    SDL_RenderClear(renderer);
    for (int x = 0; x < col; ++x) {
        for (int y = 0; y < row; ++y) {
            SDL_Color color = stateMatrix[y][x];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_Rect r;
            r.w = r.h = rectangleSize;
            r.x = xOffset + x * rectangleSize;
            r.y = yOffset + y * rectangleSize;
            SDL_RenderFillRect(renderer, &r);

        }
    }
    SDL_RenderPresent(renderer);
}

void printColorMatrix(SDL_Color **colorMatrix, int height, int width) {
    for (int ch = 0; ch < height; ch++) {
        printf("{");
        for (int pix = 0; pix < width; pix++) {
            printSDL_Color(colorMatrix[ch][pix]);
            printf(", ");
        }
        printf("}\n");
    }

}

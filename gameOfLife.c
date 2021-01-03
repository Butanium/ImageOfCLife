#include <stdio.h>
#include <SDL2/SDL.h>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image/stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//  #include "stb_image/stb_image_write.h"


typedef struct nbInfo neighboursInfo;


SDL_Color initSDL_Color(unsigned char *rgb) {
    SDL_Color p = {rgb[0], rgb[1], rgb[2]};
    return p;
}

struct nbInfo {
    SDL_Color averageDeathColor;
    SDL_Color averageLiveColor;
    char nbAliveNeighbours;
};


unsigned char deathColor[3] = {0, 0, 0};
unsigned char lifeColor[3];
double tolerance = 30;
unsigned char deathRules[] = {1, 4, 5, 6, 7};
unsigned char birthRules[] = {3};
unsigned char dRulesLen;
unsigned char bRulesLen;

void printSDL_Color(SDL_Color p);

char isCellAlive(int c, int p, SDL_Color **readMatrix);

void refresh(int row, int col, SDL_Color **stateMatrix, SDL_Color **writeMatrix);

char neighbourList[8][2] = {{0,  1},
                            {0,  -1},
                            {1,  0},
                            {1,  1},
                            {1,  -1},
                            {-1, 0},
                            {-1, 1},
                            {-1, -1}};

neighboursInfo getNeighboursInfo(int c, int p, char is_a, int row, int col, SDL_Color **readMatrix);

int contains(unsigned char element, unsigned char list[], int listSize) {
    for (int i = 0; i < listSize; i++) {
        if (element == list[i]) {
            return 1;
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    int width, height, channels;
    unsigned char *img = stbi_load("black_white_test.jpg", &width, &height, &channels, 0);
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
    dRulesLen = sizeof(deathRules);
    bRulesLen = sizeof(birthRules);
    if (img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    int ch, pix;
    SDL_Color **stateMatrix1 = malloc(height * sizeof(SDL_Color *));
    if (stateMatrix1 == NULL){
        printf("Unable to allocate memory");
        exit(1);
    }
    for (int i = 0; i < height; ++i) {
        stateMatrix1[i] = malloc(width * sizeof(SDL_Color));
    }
    for (ch = 0; ch < height; ch++) {
        //printf("{");
        for (pix = 0; pix < width; pix++) {
            unsigned bytePerSDL_Color = channels;
            unsigned char *SDL_ColorOffset = img + (pix + height * ch) * bytePerSDL_Color;
            SDL_Color p = initSDL_Color(SDL_ColorOffset);
            stateMatrix1[ch][pix] = p;
            //printSDL_Color(p);
            //printf(", ");

        }
        //printf("}\n");
    }
    SDL_Color stateMatrix2[height][width];
    memcpy(stateMatrix2, stateMatrix1, sizeof(stateMatrix2));
    for (char i = 0; i < 3; ++i) {
        lifeColor[i] = 255 - deathColor[i];
    }
    refresh(height, width, stateMatrix1, (SDL_Color **) stateMatrix2);
    Sleep(1000);
    SDL_Quit();
    return 0;
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
    return s < tolerance;
}


neighboursInfo getNeighboursInfo(int c, int p, char is_a, int row, int col, SDL_Color **readMatrix) {
    char nbDeathN;
    char nbAliveN;
    double sumAColor[3] = {0, 0, 0};
    double sumDColor[3] = {0, 0, 0};
    for (int i = 0; i < 8; ++i) {
        int x = neighbourList[i][0] + c;
        int y = neighbourList[i][1] + p;
        if (0 < x && x < row && 0 < y && y < col) {
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
        avAColor[i] = is_a != 0 || nbAliveN != 0 ? (unsigned char) (avAColor[i] + sumAColor[i]) / (is_a + nbAliveN)
                                                 : lifeColor[i];
        avDColor[i] = (unsigned char) (avDColor[i] + sumDColor[i]) / (1 - is_a + nbDeathN + 1);
    }
    neighboursInfo result = {initSDL_Color(avAColor), initSDL_Color(avDColor), nbAliveN};
    return result;
}

void refresh(int row, int col, SDL_Color **readMatrix, SDL_Color **writeMatrix) {
    for (int c; c < row; ++c) {
        for (int p; p < col; ++p) {
            char is_a = isCellAlive(c, p, readMatrix);
            neighboursInfo nbInfo = getNeighboursInfo(c, p, is_a, row, col, readMatrix);
            if (is_a) {
                if (contains(nbInfo.nbAliveNeighbours, deathRules, dRulesLen)) {
                    writeMatrix[c][p] = nbInfo.averageDeathColor;
                } else {
                    writeMatrix[c][p] = nbInfo.averageLiveColor;
                }
            } else {
                if(contains(nbInfo.nbAliveNeighbours, birthRules, bRulesLen)){
                    writeMatrix[c][p] = nbInfo.averageLiveColor;
                } else {
                    writeMatrix[c][p] = nbInfo.averageDeathColor;
                }
            }

        }
    }
}


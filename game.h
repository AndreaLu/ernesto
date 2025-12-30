#ifndef GAME_H
#define GAME_H

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 170
struct Arc {
    float radius;
    float angle;
    float angularSpeed;
    float width;
    int offset;
    bool enabled;
};

const int MAX_ARCS = 5;


void GameInit();
void GameUpdate();
// Call this only once when the button is clicked
void GamePressButton();
void GameLongPressButton();
// Call this only once when the encoder is rotated
void GameRotateEncoder(bool clockwise);
Arc* GetArcs();
int GetSelection();

unsigned long GetMillis();
int GetBarX();
#endif
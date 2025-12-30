#ifndef GAME_H
#define GAME_H


struct Arc {
    float radius;
    float angle;
    float angularSpeed;
    float width;
    int offset;
    bool enabled;
    uint32_t color;
    float posX, posY;
    float velX, velY;
};
enum GameState {
    GS_PLAYING,
    GS_LEVEL_TRANSITION,
    GS_CHECKING_WRONG,
    GS_CHECKING_RIGHT,
    GS_GAME_OVER,
    GS_MAX,
};





void GameInit();
void GameUpdate();
// Call this only once when the button is clicked
void GamePressButton();
void GameLongPressButton();
// Call this only once when the encoder is rotated
void GameRotateEncoder(bool clockwise);
Arc* GetArcs();
GameState GetState();
int GetSelection();
int GetNumCorrect();
unsigned long GetMillis();
int GetBarX();
#endif
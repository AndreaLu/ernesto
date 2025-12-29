#ifndef GAME_H
#define GAME_H

uint8_t currLevel = 0;

typedef enum GameState {
    GS_PLAYING,
    GS_LEVEL_TRANSITION,
    GS_MAX
} state;


#endif
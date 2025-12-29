#include "game.h"
#include <TFT_eSPI.h> // just for millis() ...

enum GameState {
    GS_PLAYING,
    GS_LEVEL_TRANSITION,
    GS_GAME_OVER,
    GS_MAX,
};

GameState state;
uint8_t currLevel = 0;
unsigned long time0;


float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

struct Arc {
    float radius;
    float angle;
    float angularSpeed;
    float width;
    float apparentOffset;
    float apparentAngle;
    float realAngle;
    float realOffset;
};

const int MAX_ARCS = 5;
Arc arcs[MAX_ARCS]; // Max 7 arcs
const int MAX_LEVEL = 4;
const int numArcs[MAX_LEVEL] = {2, 3, 4, 5};
bool buttonPressed = false;
uint8_t encoderRotation = 0; // +1 for clockwise, -1 for counterclockwise, 0 for no rotation

void GameInit() {
    state = GS_PLAYING;
    time0 = millis();

    // Basic arc initialization
    for( int i=0; i<MAX_ARCS; i++ ) {
        arcs[i].radius = 27 + i * (12 + 3*2);
        arcs[i].angle = 0;
        arcs[i].angularSpeed = 0;
        arcs[i].width = 3;
        arcs[i].apparentOffset = 0;
        arcs[i].apparentAngle = 0;
        arcs[i].realAngle = 0;
        arcs[i].realOffset = 0;
    }
}

void advanceLevel() {
    currLevel++;
    state = GS_LEVEL_TRANSITION;
    time0 = millis();
}


void GameUpdate() {
    unsigned long time_ms;
    float time;

    time_ms = millis() - time0;
    time = time_ms / 1000.0f;

    switch( state ) {
        case GS_PLAYING:
            break;

        case GS_LEVEL_TRANSITION:
            if (time > 3.0f) { // After 3 seconds, go to next level or game over
                state = (currLevel == MAX_LEVEL - 1) ? GS_GAME_OVER : GS_PLAYING;
                time0 = millis();
            }
            break;

        default:
            break;
    }

    if( currLevel < MAX_LEVEL-1 ) {
        for( int i=0; i < numArcs[currLevel]; i++ ) {
            arcs[i].realAngle += arcs[i].angularSpeed * time;
            arcs[i].apparentAngle = lerp(arcs[i].apparentAngle, arcs[i].realAngle, 0.1f);
        }
    }

    // Reset inputs
    encoderRotation = 0;
    buttonPressed = false;

}


void GamePressButton() {
    buttonPressed = true;
}
void GameRotateEncoder(bool clockwise) {
    encoderRotation = clockwise ? 1 : -1;
}
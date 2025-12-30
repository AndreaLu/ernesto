#ifdef DEBUG    
    #define TFT_WHITE 0xFFFFFF
    #define TFT_RED 0x0000FF
    #define TFT_GREEN 0x00FF00
    #define TFT_CYAN 0xf58742 //0x4287f5
    #include "debug.h"
    #include <cstdint>
#else
    #include <TFT_eSPI.h>
    #include <stdint.h>
#endif
#include "game.h"
#include <math.h>

#include "network.h"
#include "pin_config.h"


GameState state;
GameState GetState() {
    return state;
}
unsigned short currLevel = 0;
unsigned long time0;
float time_back = 0;
unsigned long prevTime0;
unsigned long pressStartTime;
unsigned long pressDuration;


Arc arcs[MAX_RINGS]; // Max 7 arcs

Arc* GetArcs() {
    return &arcs[0];
}
int barX = SCREEN_WIDTH / 2;
int barY = - SCREEN_HEIGHT/2;

int GetBarX() { return barX; }
int GetBarY() { return barY; }

#ifdef DEBUG
    #define PI 3.14159265f
#endif
#define PIPI 6.283185307f
#define DEGTORAD(x) (x/180.0f*PI)
#define RADTODEG(x) (x/PI*180.0f)

const float ANGLE_TOLERANCE = DEGTORAD(20.0f);

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

const float angleStep = DEGTORAD(30.0f);

const int MAX_LEVEL = 4;
const int numArcs[MAX_LEVEL] = {3, 3, 4, 5};
//bool buttonPressed = false;
//bool prevButtonPressed = false;
//bool buttonReleased = false;
bool buttonShortPressed = false;
bool buttonLongPressed = false;



int encoderRotation = 0; // +1 for clockwise, -1 for counterclockwise, 0 for no rotation
int selection = 0;
int GetSelection() { return selection; }

#ifndef DEBUG
float randomFloat(float minVal, float maxVal) {
  return minVal + (maxVal - minVal) * ((float)random(0, 1000000) / 1000000.0);
}
#else
uint32_t seed = 123456789;  // scegli un seed non nullo

uint32_t pseudoRandom() {
  seed = seed * 1664525UL + 1013904223UL;
  return seed;
}

int randomRange(int minVal, int maxVal) {
  uint32_t r = pseudoRandom();
  return minVal + (r % (maxVal - minVal + 1));
}

float randomFloat(float minVal, float maxVal) {
  return minVal + (maxVal - minVal) * ((float)randomRange(0, 1000000) / 1000000.0);
}
#endif

void GameInit() {
    state = GS_PREPARING_LEVEL;
    time0 = GetMillis();
    #ifndef DEBUG
    randomSeed(esp_random());
    #endif

    // Basic arc initialization
    for( int i=0; i<MAX_RINGS; i++ ) {
        arcs[i].radius = 27 + i * (RINGS_SEP + RINGS_THICKNESS*2);
        arcs[i].angle = 0;
        arcs[i].angularSpeed = 1;
        arcs[i].width = RINGS_THICKNESS;
        arcs[i].offset = 0;
        arcs[i].enabled = false;
        arcs[i].color = TFT_CYAN;
        arcs[i].posX = 0.0f;
        arcs[i].posY = -SCREEN_HEIGHT_F;
        #ifndef DEBUG
        arcs[i].offset = random(1,25);
        #else
        arcs[i].offset = randomRange(1,25);
        #endif
        arcs[i].velX = randomFloat(-3.0f,3.0f);
        arcs[i].velY = randomFloat(-3.0f,-0.3f);
    }
}

static float wronglerp(float x) {
    return sin(x*2.0f*PI*3.0f)/(5.0f*(x+0.2f));
}
static float cupola(float x ) {
    return 0.5f*(sin(2.0f*PI*x-PI/2.0f) + 1.0f);
}
static float sigmoide(float x) {
    return 0.5f*(sin(PI*x-PI/2.0f)+1.0f);
}
int numCorrectRings = 0;
int GetNumCorrect() { return numCorrectRings; }
// Called once per frame
void GameUpdate() {
    unsigned long time_ms;
    float time;
    

    time = ((float)(GetMillis() - time0)) / 1000.0f;
    float jj;
    switch( state ) {
        case GS_PREPARING_LEVEL:
            jj = sigmoide(time/1.3f);
            for( int i=0; i < numArcs[currLevel]; i++ ) {
                arcs[i].posY = lerp(-SCREEN_HEIGHT_F,0.0f,jj);
                arcs[i].angle = arcs[i].offset * angleStep;
                barY = int(lerp(-SCREEN_HEIGHT_F/2.0f,SCREEN_HEIGHT_F/2.0f,jj));
            }
            if( time >= 1.3f ) {
                for( int i=0; i < numArcs[currLevel]; i++ ) {
                    arcs[i].posY = 0.0f;
                }
                barY = SCREEN_HEIGHT/2;
                state = GS_PLAYING;
                time0 = GetMillis();
            }
            break;
        case GS_PLAYING:
            time += time_back;
            // Update arcs angles
            // TODO: rimetti i=0
            for( int i=0; i < numArcs[currLevel]; i++ ) {
                arcs[i].angle = arcs[i].angularSpeed * time + arcs[i].offset * angleStep;
                while( arcs[i].angle > PIPI ) arcs[i].angle -= PIPI;
                arcs[i].color = i == selection ? TFT_CYAN : TFT_WHITE;
            }
            // TODO: togli questo for
            //for( int i=0; i < numArcs[currLevel]; i++ ) {
            //    arcs[i].color = i == selection ? TFT_CYAN : TFT_WHITE;
            // }
            // Select the next arc to rotate with the encoder
            //if( buttonReleased && pressDuration < 1000 ) {
            if( buttonShortPressed ) {
                buttonShortPressed = false;
                selection = (selection + 1) % numArcs[currLevel];
            }
            if( encoderRotation != 0 ) {
                arcs[selection].offset += encoderRotation;
                encoderRotation = 0;
            }
            if( time > 10.0f ) {
                for( int i=0; i < numArcs[currLevel]; i++ ) {
                    arcs[i].angle = 0.0f;
                }
            }
            // Attempt a solution
            //if( buttonReleased && pressDuration >= 1000 ) {
            if( buttonLongPressed ) {
                buttonLongPressed = false;
                bool allAligned = true;
                numCorrectRings = 0;
                for( int i=0; i < numArcs[currLevel]; i++ ) {
                    // angle is always between 0 and 2PI, 
                    float alpha = arcs[i].angle;
                    if( alpha <= ANGLE_TOLERANCE || alpha >= PIPI-ANGLE_TOLERANCE ) {
                        numCorrectRings += 1;
                        arcs[i].color = TFT_GREEN;
                    } else {
                        arcs[i].color = TFT_RED;
                        allAligned = false;
                    }
                }
                if( allAligned ) {
                    state = GS_CHECKING_RIGHT;
                    time0 = GetMillis();
                    time_back = 0;
                } else {
                    state = GS_CHECKING_WRONG;
                    time_back = time;
                    time0 = GetMillis();
                }
                
            }
            break;

        case GS_CHECKING_WRONG:
            // This state lasts 2 seconds
            //sin( 3.0f * 2.0f * pi * time * time / 2.0f ) * 10 + SCREEN_WIDTH / 2;
            if( numCorrectRings > 0 ) {
                // First a small animation where the bar goes to the right of the given amount
                if( time < 1.0f ) {
                    barX = SCREEN_WIDTH/2 + cupola(time) * (arcs[numCorrectRings-1].radius + RINGS_SEP_F - BAR_WIDTH_F/2.0f);
                } else {
                    if( time - 1.0f < 1.3f ) {
                        barX = -wronglerp(time/1.3f)*10 + SCREEN_WIDTH/2;
                    } else {
                        state = GS_PLAYING;
                        for( int i=0; i<numArcs[currLevel]; i++) {
                            arcs[i].color = TFT_WHITE;
                            arcs[i].posX = 0.0f; arcs[i].posY = 0.0f;
                        }
                        time0 = GetMillis();
                        barX = SCREEN_WIDTH / 2;
                        numCorrectRings = 0;
                    }
                }
            } else {
                barX = wronglerp(time/1.3f)*10 + SCREEN_WIDTH/2;
                if( time > 1.3f ) {
                    state = GS_PLAYING;
                    for( int i=0; i<numArcs[currLevel]; i++){
                        arcs[i].color = TFT_WHITE;
                        arcs[i].posX = 0.0f; arcs[i].posY = 0.0f;
                    }
                    time0 = GetMillis();
                    barX = SCREEN_WIDTH / 2;
                    numCorrectRings = 0;
                }
            }
            
            break;
        case GS_CHECKING_RIGHT:
            // This state lasts 2 seconds
            if( time < 1.0f ) {
                barX = SCREEN_WIDTH_F/2.0f-BAR_WIDTH_F/2.0f + sigmoide(time)*(arcs[numArcs[currLevel]-1].radius + BAR_WIDTH_F/2.0f + RINGS_SEP_F);
            } else {
                if( time - 1.0f < 4.0f ) {
                    for( int i=0; i<numArcs[currLevel]; i++) {
                        arcs[i].velY += GRAVITY_Y;
                        arcs[i].posX += arcs[i].velX;
                        arcs[i].posY += arcs[i].velY;
                    }
                } else {
                    currLevel++;
                    if( currLevel >= MAX_LEVEL ) {
                        state = GS_GAME_OVER;
                        #if(WIFI_ENABLED == 1)
                        NETW::SignalCompletion();
                        #endif
                    } else {
                        // TODO: use GS_LEVEL_TRANSITION for some animations
                        state = GS_PLAYING;
                        for(int i=0; i<numArcs[currLevel]; i++ ) {
                            arcs[i].enabled = true;
                            arcs[i].color = TFT_WHITE;
                            arcs[i].posX = 0.0f; arcs[i].posY = 0.0f;
                        }
                    }
                    time0 = GetMillis();
                    selection = 0;
                    barX = SCREEN_WIDTH / 2;
                    numCorrectRings = 0;
                }
            }
            break;

        case GS_LEVEL_TRANSITION:
            if (time > 3.0f) { // After 3 seconds, go to next level or game over
                state = (currLevel == MAX_LEVEL - 1) ? GS_GAME_OVER : GS_PLAYING;
                for( int i=0; i<numArcs[currLevel]; i++) arcs[i].color = TFT_WHITE;
                time0 = GetMillis();
            }
            break;

        default:
            break;
    }
}


void GamePressButton() {
    buttonShortPressed = true;
}
void GameLongPressButton() {
    buttonLongPressed = true;
}
void GameRotateEncoder(bool clockwise) {
    encoderRotation = clockwise ? 1 : -1;
}


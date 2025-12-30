#include "game.h"
#include<math.h>
#ifdef DEBUG
#include "debug.h"
#endif
#include "network.h"

enum GameState {
    GS_PLAYING,
    GS_LEVEL_TRANSITION,
    GS_CHECKING_WRONG,
    GS_CHECKING_RIGHT,
    GS_GAME_OVER,
    GS_MAX,
};

GameState state;
unsigned short currLevel = 0;
unsigned long time0;
float time_back = 0;
unsigned long prevTime0;
unsigned long pressStartTime;
unsigned long pressDuration;

Arc arcs[MAX_ARCS]; // Max 7 arcs

Arc* GetArcs() {
    return &arcs[0];
}
int barX = SCREEN_WIDTH / 2;

int GetBarX() { return barX; }


const float pi = 3.14159265f;
const float ANGLE_TOLERANCE = pi*2.0f/360.0f*7;

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

const float angleStep = 2.0f * pi / 30.0f;

const int MAX_LEVEL = 4;
const int numArcs[MAX_LEVEL] = {2, 3, 4, 5};
//bool buttonPressed = false;
//bool prevButtonPressed = false;
//bool buttonReleased = false;
bool buttonShortPressed = false;
bool buttonLongPressed = false;



int encoderRotation = 0; // +1 for clockwise, -1 for counterclockwise, 0 for no rotation
int selection = 0;
int GetSelection() { return selection; }

void GameInit() {
    state = GS_PLAYING;
    time0 = GetMillis();

    // Basic arc initialization
    for( int i=0; i<MAX_ARCS; i++ ) {
        arcs[i].radius = 27 + i * (12 + 3*2);
        arcs[i].angle = 0;
        arcs[i].angularSpeed = 1;
        arcs[i].width = 3;
        arcs[i].offset = 0;
        arcs[i].enabled = false;
    }
    // TODO: randomize the offset in real time at the change of level
    arcs[0].offset = 3;
    arcs[1].offset = 25;
    arcs[2].offset = 2;
    arcs[3].offset = 16;
    arcs[4].offset = 20;
    arcs[5].offset = 8;
    arcs[6].offset = 12;


}

static float wronglerp(float x) {
    //return sin((1-x)*(1-x)*3.9633f*3.9633f);
    //return sin((1-x)*(1-x)*3.54491f*3.54491f);
    return sin(x*2.0f*pi*3.0f)/(5.0f*(x+0.2f));
}

// Called once per frame
void GameUpdate() {
    unsigned long time_ms;
    float time;
    int numCorrectRings = 0;

    time = ((float)(GetMillis() - time0)) / 1000.0f;

    // Manage Inputs
    // buttonPressed is externally set to 1 only once when the button is pressed
    /*
    if( buttonPressed && !prevButtonPressed) {
        pressStartTime = GetMillis();
        prevButtonPressed = true;
    }
    if( !buttonPressed && prevButtonPressed) {
        pressDuration = GetMillis() - pressStartTime;
        prevButtonPressed = false;
        buttonReleased = true;
    }
    */
    switch( state ) {
        case GS_PLAYING:
            time += time_back;
            // Update arcs angles
            for( int i=0; i < numArcs[currLevel]; i++ ) {
                arcs[i].angle = arcs[i].angularSpeed * time + arcs[i].offset * angleStep;
            }
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
            // Attempt a solution
            //if( buttonReleased && pressDuration >= 1000 ) {
            if( buttonLongPressed ) {
                buttonLongPressed = false;
                bool allAligned = true;
                numCorrectRings = 0;
                for( int i=0; i < numArcs[currLevel]; i++ ) {
                    if( arcs[i].angle > ANGLE_TOLERANCE || arcs[i].angle < -ANGLE_TOLERANCE)  {
                        allAligned = false;
                        break;
                    } else {
                        numCorrectRings += 1;
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
                selection = 0;
            }
            break;

        case GS_CHECKING_WRONG:
            // This state lasts 2 seconds
            barX = wronglerp(time/1.3f)*10 + SCREEN_WIDTH/2;//sin( 3.0f * 2.0f * pi * time * time / 2.0f ) * 10 + SCREEN_WIDTH / 2;
            if( time > 1.3f ) {
                state = GS_PLAYING;
                time0 = GetMillis();
                barX = SCREEN_WIDTH / 2;
            }
            break;
        case GS_CHECKING_RIGHT:
            // This state lasts 2 seconds
            barX = lerp( SCREEN_WIDTH / 2, SCREEN_WIDTH, (time / 2.0f)*(time / 2.0f) );
            if( time > 2.0f ) {
                currLevel++;
                if( currLevel >= MAX_LEVEL ) {
                    state = GS_GAME_OVER;
                    NETW::SignalCompletion();
                } else {
                    // TODO: use GS_LEVEL_TRANSITION for some animations
                    state = GS_PLAYING;
                    for(int i=0; i<numArcs[currLevel]; i++ ) {
                        arcs[i].enabled = true;
                    }
                }
                time0 = GetMillis();
                selection = 0;
                barX = SCREEN_WIDTH / 2;
            }
            break;

        case GS_LEVEL_TRANSITION:
            if (time > 3.0f) { // After 3 seconds, go to next level or game over
                state = (currLevel == MAX_LEVEL - 1) ? GS_GAME_OVER : GS_PLAYING;
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


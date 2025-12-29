#include<TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include<SPI.h>
#include"game.h"

TFT_eSPI tft = TFT_eSPI(); // Create TFT object

#define PIN_POWER_ON 46

#define W 320
#define H 170

#define HW 160
#define HH 85

unsigned long currTime = 0;
int level = 0;

int numArcs[] = {2, 3, 4, 5};
int W = 320;
int H = 170;
int HW = W/2;
int HH = H/2;


void setup() {
    
    // From TFT_Rainbow example - turns on the display?
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    tft.init();
    tft.setRotation(1);

    state = IN_LEVEL;

    // Arcs should span from abuot 1/3 - 1/10 of the screen height and shuold be equally spaced
    // Let's say there will be at most 7 arcs displayed at once
    tft.drawSmoothArc(HW, HY, radius, inner_radius, start_angle, end_angle, fg_color, bg_color, arc_end);

}

void loop() {
    tft.fillScreen(TFT_BLACK);


}
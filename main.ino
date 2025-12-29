#include<TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include<SPI.h>
#include"game.h"

/*
Tutto va su rete wifi

SSID:
Lo Spirito Della Nonna

PSW:
SoloLeiLaSa
*/
TFT_eSPI tft = TFT_eSPI(); // Create TFT object

#define PIN_POWER_ON 46


unsigned long currTime = 0;
int level = 0;

int numArcs[] = {2, 3, 4, 5};


void setup() {
    
    // From TFT_Rainbow example - turns on the display?
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    tft.init();
    tft.setRotation(1);


    // Arcs should span from abuot 1/3 - 1/10 of the screen height and shuold be equally spaced
    // Let's say there will be at most 7 arcs displayed at once
    GameInit();
}

void loop() {
    tft.fillScreen(TFT_BLACK);
    GameUpdate();

}